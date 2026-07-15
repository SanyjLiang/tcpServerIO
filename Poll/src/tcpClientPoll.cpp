#include "head.h"

#define SER_IP "192.168.68.132"
#define SER_PORT 8888
#define CLI_IP "192.168.68.132"
#define CLI_PORT 9999

int main(int argc, const char *argv[])
{
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1)
    {
        perror("socket error");
        return -1;
    }
    printf("socket success cfd = %d\n", cfd);

    struct sockaddr_in cin;
    cin.sin_family = AF_INET;
    cin.sin_port = htons(CLI_PORT);
    cin.sin_addr.s_addr = inet_addr(CLI_IP);

    if (bind(cfd, (struct sockaddr *)&cin, sizeof(cin)) == -1)
    {
        perror("bind error");
        return -1;
    }
    printf("bind success\n");

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(SER_PORT);
    sin.sin_addr.s_addr = inet_addr(SER_IP);

    if (connect(cfd, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
        perror("connect error");
        return -1;
    }
    printf("connet success\n");

    //使用poll完成终端输入和套接字接收数据的并发执行
    struct pollfd pfds[2];

    //分别给数组中两个文件描述符成员赋值
    pfds[0].fd = 0;             //表示检测0号
    pfds[0].events = POLLIN;    //表示检测的是读时间

    pfds[1].fd = cfd;           //检测cfd文件描述符
    pfds[1].events = POLLIN;    //检测读事件

    char wbuf[128] = "";
    while (1)
    {
        int res = poll(pfds, 2, -1);
        //功能：阻塞等待文件描述符集合中是否有事件产生
        //参数1：文件描述符集合起始地址
        //参数2：文件描述符的个数
        //参数3：表示永久等待
        if (res == -1)
        {
            perror("poll error");
            return -1;
        }

        //程序运行至此，表示文件描述符容器中，有事件产生
        //表示0号文件描述符的事件
        if (pfds[0].events == pfds[0].revents)
        {
            bzero(wbuf, sizeof(wbuf));
            fgets(wbuf, sizeof(wbuf), stdin);
            wbuf[strlen(wbuf) - 1] = 0;

            if (send(cfd, wbuf, sizeof(wbuf), 0) == -1)
            {
                perror("send error");
                return -1;
            }
        }

        //表示有客户端发来消息
        if (pfds[1].events == pfds[1].revents)
        {
            if (recv(cfd, wbuf, sizeof(wbuf), 0) == 0)
            {
                printf("对端已下线\n");
                return -1;
            }
            printf("收到服务器消息为：%s\n", wbuf);
        }
    }

    close(cfd);
    return 0;
}