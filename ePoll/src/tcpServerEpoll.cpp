//#include "../include/head.h"
//#include <sys/epoll.h>
#include "head.h"

#define SER_PORT 8888
#define SER_IP "192.168.68.132"

int main(int argc, const char *argv[])
{
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    // 参数1：AF_INET表示使用的是ipv4通信协议
    // 参数2：SOCK_STREAM表示使用的是tcp通信方式
    // 参数3：由于参数二指定了参数，所以参数3填0即可

    if (sfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success sfd = %d\n", sfd);

    // 2.绑定ip地址和端口号
    // 2.1 填充要绑定的ip地址和端口号结构体
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;                // 通信域
    sin.sin_port = htons(SER_PORT);          // 端口号
    sin.sin_addr.s_addr = inet_addr(SER_IP); // ip地址

    // 2.2 绑定工作
    // 参数1：要被绑定的套接字文件描述符
    // 参数2：要绑定的地址信息结构体，需要进行强制类型转换，防止警告
    // 参数3：参数2的大小
    if (bind(sfd, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");

    // 3.启动监听
    // 参数1：要启动监听的文件描述符
    // 参数2：挂起队列的长度
    if (listen(sfd, 128) == -1)
    {
        perror("listen error");
        return -1;
    }
    printf("listen success\n");

    // 4.阻塞等待客户端的连接请求
    // 定义变量，用于接受客户端地址信息结构体
    struct sockaddr_in cin;          // 用于接受地址信息的结构体
    socklen_t socklen = sizeof(cin); // 用于接受地址信息的长度

    int epfd = epoll_create(1);
    if (epfd == -1)
    {
        perror("epoll_create error");
        return -1;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sfd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev) == -1)
    {
        perror("epoll_ctl error");
        return -1;
    }

    struct epoll_event evs[1024];
    int size = sizeof(evs) / sizeof(evs[0]);

    while (1)
    {
        int num = epoll_wait(epfd, evs, size, -1);
        printf("num = %d\n", num);

        for (int i = 0; i < num; i++)
        {
            int fd = evs[i].data.fd;

            if (fd == sfd)
            {
                int newfd = accept(sfd, (struct sockaddr *)&cin, &socklen);
                // 参数1：服务器套接字文件描述符
                // 参数2：用于接收客户端地址信息结构体的容器
                // 参数3：参数2的大小

                if (newfd == -1)
                {
                    perror("accept error");
                    return -1;
                }

                printf("[%s:%d]:已经连接成功,newfd = %d!!!\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
                struct epoll_event ev;
                ev.events = EPOLLIN;
                ev.data.fd = newfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, &ev);
            }else
            {
                char rbuf[128] = "";
                bzero(rbuf, sizeof(rbuf));
                int res = recv(fd, rbuf, sizeof(rbuf), 0);
                if (res == 0)
                {
                    printf("对端已下线\n");

                    //从epoll树中删除
                    epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);

                    close(fd);
                }
                printf("收到数据:%s\n", rbuf);

                // 对收到的数据处理一下，返回给客户端
                strcat(rbuf, "*__*");

                // 发送处理后的数据给客户端
                if (send(fd, rbuf, strlen(rbuf), 0) == -1)
                {
                    perror("send error");
                    return -1;
                }
                printf("发送成功\n");
                
            }
        }
    }

    close(sfd);
    close(epfd);
    std::cout << "hello world" << std::endl;
    return 0;
}