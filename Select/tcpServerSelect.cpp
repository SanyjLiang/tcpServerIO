#include <sys/select.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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

    // 定义文件描述符集合
    fd_set readfds, tempfds; // 读文件描述符集合

    // 将文件描述符结合清空
    FD_ZERO(&readfds);

    // 将文件描述符放入到文件描述符集合中
    FD_SET(0, &readfds);
    FD_SET(sfd, &readfds);

    int maxfd = sfd;
    int newfd = -1;

    //定义一个地址信息结构体数组来存储客户端对应的地址信息
    struct sockaddr_in cin_arr[1024];

    while (1)
    {
        tempfds = readfds;
        // 调用阻塞函数select用来监视事件的发生
        int res = select(maxfd + 1, &tempfds, NULL, NULL, NULL);
        if (res == -1)
        {
            perror("select error");
            return -1;
        }
        else if (res == 0)
        {
            printf("time out!!!\n");
            return -1;
        }

        // 程序执行至此，表示一定有其中至少一个文件描述符产生了事件，只需要判断哪个文件描述符还在集合中
        // 就说明该文件描述符产生了事件

        // 表示sfd文件描述符触发了事件
        if (FD_ISSET(sfd, &tempfds))
        {
            newfd = accept(sfd, (struct sockaddr *)&cin, &socklen);
            // 参数1：服务器套接字文件描述符
            // 参数2：用于接收客户端地址信息结构体的容器
            // 参数3：参数2的大小
            {
                if (newfd == -1)
                {
                    perror("accept error");
                    return -1;
                }
            }

            printf("[%s:%d]:已经连接成功,newfd = %d!!!\n", inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), newfd);
            
            //将该客户端对应的套接字地址信息结构体放入数组对应的位置上
            //newfd文件描述符对应的地址信息结构体为cin_arr[newfd]
            cin_arr[newfd]=cin;

            // 将newfd加入到文件描述符集合中
            FD_SET(newfd, &readfds);

            // 更新文件描述符中最大的文件描述符
            if(maxfd<newfd)
            maxfd = newfd;
        }

        // 判断0号文件描述符是否产生了事件
        if (FD_ISSET(0, &tempfds))
        {
            char wbuf[128] = "";
            fgets(wbuf, sizeof(wbuf), stdin);
            printf("触发了键盘输入事件：%s\n", wbuf);

            //将输入的消息转发给所有客户端
            for(int i=4;i<=maxfd;i++)
            {
                send(i,wbuf,strlen(wbuf),0);
            }
        }

        // 判断是否是客户端发来消息
        for (int i = 4; i <= maxfd; i++)
        {
            if (FD_ISSET(i, &tempfds))
            {
                char rbuf[128] = "";
                bzero(rbuf, sizeof(rbuf));
                int res = recv(i, rbuf, sizeof(rbuf), 0);
                if (res == 0)
                {
                    printf("对端已下线\n");
                    close(i);

                    //需要将文件描述符从readfds中删除
                    FD_CLR(i,&readfds);

                    //更新最大的maxfd
                    for(int k=maxfd;k>=0;k--)
                    {
                        if(FD_ISSET(k,&readfds))
                        {
                            maxfd=k;
                            break;          //结束向下进行的循环
                        }
                    }

                    continue;   //本轮循环结束，继续下一次的select阻塞
                }
                printf("[%s:%d]:%s\n", inet_ntoa(cin_arr[i].sin_addr), ntohs(cin_arr[i].sin_port), rbuf);

                // 对收到的数据处理一下，返回给客户端
                strcat(rbuf, "*__*");

                // 发送处理后的数据给客户端
                if (send(i, rbuf, strlen(rbuf), 0) == -1)
                {
                    perror("send error");
                    return -1;
                }
                printf("发送成功\n");
            }
        }
    }

    close(sfd);
    std::cout << "hello world" << std::endl;
    return 0;
}