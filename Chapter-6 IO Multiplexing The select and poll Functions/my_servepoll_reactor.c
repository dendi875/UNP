/**
 * epoll 反应堆模式的简单实现。服务端
 */

#include "unp.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define MAX_EVENTS  1024
#define BUFLEN      100

void recvdata(int fd, int events, void *arg);
void senddata(int fd, int events, void *arg);

struct myevent_s {
    int     fd;         /* 要监听的文件描述符 */
    int     events;     /* 对应监听的事件 */
    void    *arg;       /* 指向自己结构体的指针 */
    void    (*call_back)(int fd, int events, void *arg);    /* 回调函数 */
    int     status;     /* 1：表示在监听事件中，0：表示不在 */
    char    buf[BUFLEN];
    int     len;
    long    last_active;    /* 记录最后一次响应时间，做超时处理 */
};

int                 g_efd;  /* epoll_create 返回的文件描述符 */
struct myevent_s    g_events[MAX_EVENTS + 1];  /* +1 最后一个用于 lfd */

/**
 * 初始化 myevent_s 结构体变量
 */
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg)
{
    ev->fd = fd;
    ev->events = 0;
    ev->arg = arg;
    ev->call_back = call_back;
    ev->status = 0;
    memset(ev->buf, 0, sizeof(ev->buf));
    ev->len = 0;
    ev->last_active = time(NULL);

    return;
}

/**
 * 向 epoll 监听的红黑树上添加一个文件描述符
 */
void eventadd(int efd, int events, struct myevent_s *ev)
{
    struct  epoll_event epv = {0, {0}};
    int op;
    epv.data.ptr = ev;
    epv.events = ev->events = events;  /* EPOLLIN 或 EPOLLOUT */

    if (ev->status == 1) {  /* 已经在红黑树 g_efd上 */
        op = EPOLL_CTL_MOD;
    } else {
        op = EPOLL_CTL_ADD; /* 将其加入到 g_efd 上并将 status 置为1 */
        ev->status = 1;
    }

    if (epoll_ctl(efd, op, ev->fd, &epv) < 0) {
        printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
    } else {
        printf("event add OK [fd=%d], op=%d, events[%0X]\n", ev->fd, op, events);
    }

    return;
}

/**
 * 从 epoll 监听的红黑树上删除一个文件描述符
 */
void eventdel(int efd, struct myevent_s *ev)
{
    struct epoll_event epv = {0, {0}};

    if (ev->status != 1) {     /* 没有在红黑树上，直接返回 */
        return;
    }

    epv.data.ptr = ev;
    ev->status = 0;
    epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);

    return;
}

/**
 * 创建 socket，初始化 lfd
 */
void initlistensocket(int efd, short port)
{
    struct sockaddr_in   sin;

    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(lfd, F_SETFL, O_NONBLOCK);        /* 将 lfd 设置为非阻塞*/

    memset(&sin, 0, sizeof(sin));
    sin.sin_family  = AF_INET;
    sin.sin_port    = htons(port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    /* 把地址和端口捆绑到套接字上 */
    bind(lfd, (struct sockaddr *)&sin, sizeof(sin));

    /* 把未连接的套接字转换成监听套接字 */
    listen(lfd, SOMAXCONN);

    /* 给 lfd 设置一个 myevent_s 结构体且回调函数为 acceptconn，lfd 放在 g_events 数组的最后位置 */
    eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);
    /* 将 lfd 添加到红黑树上并监听 EPOLLIN 事件 */
    eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);

    return;
}

void acceptconn(int lfd, int events, void *arg)
{
    struct  sockaddr_in     cin;
    socklen_t               len = sizeof(cin);
    int                     cfd, i;

    if ((cfd = accept(lfd, (struct sockaddr *)&cin, &len)) == -1) {
        if (errno != EAGAIN && errno != EINTR) {
            /* 暂时不做出错处理 */
        }
        printf("%s：accept，%s\n", __func__, strerror(errno));
        return;
    }

    do {
        /**
         * 从 g_events 数组中找到第一个空闲的元素，类似于 select 找第一个值 为 -1 的元素
         */
        for (i = 0; i < MAX_EVENTS; i++) {
            if (g_events[i].status == 0) {
                break;
            }
        }

        if (i == MAX_EVENTS) {
            printf("%s：max connect limit[%d]\n", __func__, MAX_EVENTS);
            break;
        }

        /* 将 cfd 也设置为非阻塞 */
        int flag = 0;
        if (fcntl(cfd, F_SETFL, O_NONBLOCK) < 0) {
            printf("%s：fcntl nonblocking failed，%s\n", __func__, strerror(errno));
            break;
        }

        /* 给 cfd 设置一个 myevent_s 结构体且设置回调函数为 recvdata */
        eventset(&g_events[i], cfd, recvdata, &g_events[i]);
        /* 将 cfd 添加到红黑树中并监听读事件 */
        eventadd(g_efd, EPOLLIN, &g_events[i]);

    } while (0);

    printf("new connect [%s:%d][time:%ld], position[%d]\n",
        inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), g_events[i].last_active, i);

    return;
}

void recvdata(int fd, int events, void *arg)
{
    struct myevent_s    *ev = (struct myevent_s *) arg;
    int                 len;

    len = recv(fd, ev->buf, sizeof(ev->buf), 0);
    eventdel(g_efd, ev);

    if (len > 0) {
        ev->len = len;
        ev->buf[len] = '\0';    /* 手动添加字符串结束标记 */
        printf("C[%d]:%s\n", fd, ev->buf);

        /* 设置该 fd 对应的回调函数为 senddata */
        eventset(ev, fd, senddata, ev);
        /* 将该 fd 加入红黑树中，监听其写事件 */
        eventadd(g_efd, EPOLLOUT, ev);
    } else if (len == 0) {
        close(ev->fd);
        /* ev - g_events 地址相减得到偏移元素位置 */
        printf("[fd=%d] position[%d], closed\n", fd, (int)(ev - g_events));
    } else {
        close(ev->fd);
        printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
    }

    return;
}

void senddata(int fd, int events, void *arg)
{
    struct myevent_s    *ev = (struct myevent_s *)arg;
    int                 len;

    len = send(fd, ev->buf, ev->len, 0);

    // printf("fd=%d\tev->buf=%s\tev->len=%d\n", fd, ev->buf, ev->len);
    // printf("send len = %d\n", len);

    eventdel(g_efd, ev);
    if (len > 0) {
        printf("send[fd=%d], [%d]%s\n", fd, len, ev->buf);
    } else {
        close(ev->fd);
        printf("send[fd=%d], error %s\n", fd, strerror(errno));
    }

    return;
}

int main(int argc, char **argv)
{
    unsigned short port = SERV_PORT;

    if (argc == 2) {
        port = atoi(argv[1]);   /* 如果指定端口就使用用户指定的端口，否则使用默认端口 */
    }

    g_efd = epoll_create(MAX_EVENTS + 1);
    if (g_efd <= 0) {
        printf("create efd in %s err %s\n", __func__, strerror(errno));
    }

    initlistensocket(g_efd, port);

    struct  epoll_event  events[MAX_EVENTS + 1];   /* 保存已经满足就绪事件的文件描述符数组 */
    printf("server runing:port[%d]\n", port);
    int     checkpos = 0, i;

    while (1) {
        /**
         * 超时验证，每次测试 100 个链接，如果 listenfd 当客户端 60 秒之内没有和服务器通信，
         * 则关闭此客户端链接
         */
        long now = time(NULL);  /* 当前时间 */
        for (i = 0; i < 100; i++, checkpos++) { /* 每次循环100个，使用 checkpos 控制检测对象 */
            if (checkpos == MAX_EVENTS) {
                checkpos = 0;
            }
            if (g_events[checkpos].status != 1) {  /* 不在红黑树 g_efd 上*/
                continue;
            }

            long duration = now - g_events[checkpos].last_active;  /* 客户端不活跃的时间 */

            if (duration >= 60) {
                close(g_events[checkpos].fd);
                printf("[fd=%d] timeout\n", g_events[checkpos].fd);
                eventdel(g_efd, &g_events[checkpos]);
            }
        }

        /* 监听红黑树 g_efd，将满足的事件的文件描述符添加至 events 数组中，1秒内没有事件满足，返回0 */
        int nfd = epoll_wait(g_efd, events, MAX_EVENTS + 1, 1000);
        if (nfd < 0) {
            printf("epoll_wait error, exit\n");
            break;
        }

        for (i = 0; i < nfd; i++) {
            /* 使用自定义的结构体 myevent_s 类型指针，接收联合体 data 的 void *ptr成员 */
            struct myevent_s *ev = (struct myevent_s *) events[i].data.ptr;
            if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) { /* 读事件就绪 */
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
            if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) { /* 写事件就绪 */
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
        }
    }

    /* 退出前释放所有资源 */
    return 0;
}