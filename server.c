#include "allhead.h"

char online_list[1024] = {0};//存放所有在线客户端的信息的长数组
char member[6][50] = {0};//将上述长数组切割后,分别存放线上客户端的信息的数组
int online_num = 0;//在线人数

/*
链表中每个节点含有的元素
*/
struct clientlist
{
    int sock;               //存放套接字
    char ipaddr[32];        //存放客户端的ip
    unsigned short portnum; //存放客户端的端口号
    char name[10];          //存放客户端用户名
    int line_num;           //存放序号
    struct clientlist *next;
} * head;

/*
建立链表头结点并初始化
*/
struct clientlist *clientlist_init()
{
    struct clientlist *head = malloc(sizeof(struct clientlist));
    head->next = NULL;
    head->portnum = 0;
    head->sock = -1;
    head->line_num = -1;
    bzero(head->ipaddr, sizeof(head->ipaddr));
    bzero(head->name, sizeof(head->name));
    return head;
}

/*
链表 尾插法 - 用于新的客户端连入时将其信息尾插入链表
head - 用于存放客户端信息的链表的头结点
sock - 客户端与服务器建立连接的accept产生的套接字
ipaddr - 客户端的ip地址
portnum - 客户端的端口号
*/
int clientlist_insert(struct clientlist *head, int sock, char *ipaddr, unsigned short portnum)
{
    struct clientlist *newnode = malloc(sizeof(struct clientlist));
    newnode->sock = sock;
    strcpy(newnode->ipaddr, ipaddr);
    newnode->portnum = portnum;
    newnode->next = NULL;

    struct clientlist *p = head;
    while (p->next != NULL)
    {
        p = p->next;
    }
    p->next = newnode;

    return 0;
}

/*
删除一个链表中的节点 - 用于客户端退出时将其从链表中删除
head - 用于存放客户端信息的链表的头结点
sock - 客户端与服务器建立连接的accept产生的套接字
*/
int clientlist_delete(struct clientlist *head, int sock)
{
    struct clientlist *p1 = head;
    struct clientlist *p2 = head->next;
    if (p2 == NULL)
    {
        printf("当前没有客户端在服务器内\n");
        return -1;
    }
    while (p2->sock != sock)
    {
        if (p2 == NULL)
        {
            printf("该端口没有接入服务器\n");
            return -1;
        }
        p1 = p1->next;
        p2 = p2->next;
    }
    p1->next = p2->next;
    p2->next = NULL;
    close(p2->sock);
    free(p2);
    return 0;
}

/*
寻找目标函数 - 通过用户名在存放客户端信息的链表中寻找目标客户端
head - 用于存放客户端信息的链表的头结点
p_name - 中转结点
name - 目标客户端的用户名
*/
struct clientlist *searchpoint(struct clientlist *head, struct clientlist *p_name, char *name)
{
    p_name = head;
    while ((strcmp(p_name->name, name) != 0))
    {
        p_name = p_name->next;
        if (p_name == NULL)
        {
            // printf("不存在该用户\n");
            return NULL;
        }
    }
    return p_name;
}

/*
打印链表 - 检查链表是否出错
head - 用于存放客户端信息的链表的头结点
*/
int print_list(struct clientlist *head)
{
    struct clientlist *print = head;
    while (print->next != NULL)
    {
        print = print->next;
        printf("%d\n", print->sock);
    }
    return 0;
}

/*
广播函数 - 将收到的信息发送给所有人
head - 用于存放客户端信息的链表的头结点
sendallmsg - 被广播出去的信息
*/
void *broadcast(struct clientlist *head, char *sendallmsg)
{
    struct clientlist *p_broad = head;
    while (p_broad->next != NULL)
    {
        p_broad = p_broad->next;
        send(p_broad->sock, sendallmsg, strlen(sendallmsg), 0);
    }
}

/*
发送文件函数 - 用于发送文件到目标客户端
sendpath - 所要发送文件的路径
p_recv   - 目标客户端在链表中的节点
*/
int sendfile(char *sendpath, struct clientlist *p_recv)
{
    int sendfd, ret;
    sendfd = open(sendpath, O_RDWR);
    if (sendfd == -1)
    {
        perror("打开文件失败\n");
        return -1;
    }
    char sendfile[1024] = {0};
    while (1)
    {
        bzero(sendfile, sizeof(sendfile));
        ret = read(sendfd, sendfile, sizeof(sendfile));
        send(p_recv->sock, sendfile, ret, 0);
        if (ret == 0)
        {
            printf("传输完成\n");
            break;
        }
    }
    close(sendfd);
    return 0;
}

/*
接收文件函数 - 用于接收文件到服务端,随后转发到目标客户端
recvpath - 接收路径
p_send   - 目标客户端在链表中的节点
*/
int recvfile(char *recvpath, struct clientlist *p_send)
{
    int recvfd, ret;
    printf("%s\n", recvpath);
    recvfd = open(recvpath, O_RDWR | O_CREAT | O_TRUNC);
    if (recvfd == -1)
        recvfd = open(recvpath, O_RDWR | O_TRUNC);
    char recvfile[1024] = {0};
    while (1)
    {
        bzero(recvfile, sizeof(recvfile));
        ret = recv(p_send->sock, recvfile, sizeof(recvfile), 0);
        write(recvfd, recvfile, ret);
        if (ret < sizeof(recvfile))
        {
            // printf("上传完成1\n");
            break;
        }
    }
    // printf("上传完成2\n");
    close(recvfd);
    return 0;
}

/*
登录函数 - 判断用户名是否是已注册的用户名
txtpath - 存放用户名的文件路径
user    - 传入进来用于判断的用户名
*/
int enter_menu(char *txtpath, char *user)
{
    char user_window[10] = {0};           //用户名文件放在windows系统上需要160,161,162这三行转换一下
    strtok(user, "\n");                   //用户名文件放在windows系统上需要160,161,162这三行转换一下
    sprintf(user_window, "%s\r\n", user); //用户名文件放在windows系统上需要160,161,162这三行转换一下
    char account[50] = {0};
    char passwd[50] = {0};
    char result[50] = {0};
    char *now_account;
    char *now_passwd;

    FILE *txtfile;
    txtfile = fopen(txtpath, "r+");
    if (txtfile == NULL)
    {
        perror("open txt fail\n");
        return -1;
    }

    rewind(txtfile); //将文件偏移值回到最开始的位置
    while (1)
    {
        fgets(result, sizeof(result), txtfile);
        if (feof(txtfile) == 1)
        {
            printf("账号或密码错误\n");
            return 2;
            break;
        }
        if ((strcmp(user_window, result) == 0) && ((strlen(user_window)) == (strlen(result))))
        {
            return 1;
        }
    }

    fclose(txtfile);
    return 0;
}

/*
子线程-接收信息
用于对客户端发送过来的信息进行判断,执行对应的命令
*/
void *recvmsg_func(void *arg)
{
    struct clientlist *p = (struct clientlist *)arg;
    struct clientlist *p2 = head;
    int ret;
    char recvmsg[512] = {0};
    char factmsg[2048] = {0};
    char command[512] = {0};
    char temp[512] = {0};
    pthread_t id_one;
    int time = 0;
    while (1)
    {
        if (time == 0)
        {
            int pass = 0;
            send(p->sock, "请输入用户名\n", 20, 0);
            char usrname[10] = {0};
            ret = recv(p->sock, usrname, sizeof(usrname), 0);
            pass = enter_menu("user.txt", usrname);
            if (pass != 1)
                continue;
            if (ret == 0) //退出
            {
                printf("IP地址:%s,端口:%hu退出了\n", p->ipaddr, p->portnum);
                sprintf(factmsg, "IP地址:%s,端口:%hu用户退出了\n", p->ipaddr, p->portnum);
                clientlist_delete(head, p->sock);
                broadcast(head, factmsg);
                if (head->next == NULL)
                    printf("当前没有客户端在服务器内\n");
                break;
            }
            send(p->sock, "登录成功\n", 14, 0);
            strcpy(p->name, strtok(usrname, "\n"));
            printf("用户名为:%s\n", p->name);

            char port[10] = {0};
            sprintf(port, "%hu", p->portnum);
            sprintf(member[online_num], "%s@%s@%s", p->name, p->ipaddr, port);
            strcat(online_list, member[online_num]);
            strcat(online_list, "#");

            p->line_num = online_num;
            online_num++;
            time = 1;
            bzero(port, sizeof(port));
        }
        else
        {
            bzero(temp, sizeof(temp));
            bzero(recvmsg, sizeof(recvmsg));
            bzero(command, sizeof(command));
            bzero(factmsg, sizeof(factmsg));
            ret = recv(p->sock, recvmsg, sizeof(recvmsg), 0);
            // printf("ret = %d\n",ret);
            printf("%s", recvmsg);
            if (strcmp(recvmsg, " \n") == 0 || strcmp(recvmsg, "\n") == 0)
            {
                printf("没有该命令\n");
                char ask[20] = "没有该命令\n";
                send(p->sock, ask, strlen(ask), 0);
                continue;
            }
            if (ret == 0) //退出
            {
                printf("用户名:%s,IP地址:%s,端口:%hu退出了\n", p->name, p->ipaddr, p->portnum);
                sprintf(factmsg, "用户名:%s,IP地址:%s,端口:%hu用户退出了\n", p->name, p->ipaddr, p->portnum);
                clientlist_delete(head, p->sock);
                broadcast(head, factmsg);

                int i;
                for (i = p->line_num; i <= online_num - 1; i++)
                {
                    if (i == online_num - 1)
                    {
                        bzero(member[i], sizeof(member[i]));
                        break;
                    }
                    bzero(member[i], sizeof(member[i]));
                    strcpy(member[i], member[i + 1]);
                }

                online_num--;
                bzero(online_list, sizeof(online_list));
                for (i = 0; i <= online_num - 1; i++)
                {
                    strcat(online_list, member[i]);
                    strcat(online_list, "#");
                }

                if (head->next == NULL)
                    printf("当前没有客户端在服务器内\n");
                break;
            }
            strcpy(temp, recvmsg);
            strcpy(command, strtok(temp, " \n"));
            printf("command:%s\n", command);
            if (strcmp(command, "quit") == 0) //退出
            {
                printf("用户名:%s,IP地址:%s,端口:%hu退出了\n", p->name, p->ipaddr, p->portnum);
                sprintf(factmsg, "用户名:%s,IP地址:%s,端口:%hu用户退出了\n", p->name, p->ipaddr, p->portnum);
                clientlist_delete(head, p->sock);
                broadcast(head, factmsg);

                int i;
                for (i = p->line_num; i <= online_num - 1; i++)
                {
                    if (i == online_num - 1)
                    {
                        bzero(member[i], sizeof(member[i]));
                        break;
                    }
                    bzero(member[i], sizeof(member[i]));
                    strcpy(member[i], member[i + 1]);
                }

                online_num--;
                bzero(online_list, sizeof(online_list));
                for (i = 0; i <= online_num - 1; i++)
                {
                    strcat(online_list, member[i]);
                    strcat(online_list, "#");
                }

                if (head->next == NULL)
                    printf("当前没有客户端在服务器内\n");
                break;
            }
            else if (strcmp(command, "/all") == 0) //  /all xxxxx
            {
                printf("公共频道: 用户名:%s,IP地址:%s,端口:%hu,信息:%s", p->name, p->ipaddr, p->portnum, recvmsg);
                bzero(recvmsg, sizeof(recvmsg));
                strcpy(recvmsg, strtok(NULL, " "));
                sprintf(factmsg, "公共频道: 用户名:%s,信息:%s", p->name, recvmsg);
                broadcast(head, factmsg);
            }
            else if (strcmp(command, "/one") == 0) //  /one name xxxxx
            {
                char toname[10] = {0};
                strcpy(toname, strtok(NULL, " \n"));
                struct clientlist *p_to;
                p_to = searchpoint(head, p_to, toname);
                if (p_to == NULL)
                {
                    printf("悄悄话: 用户名:%s,IP地址:%s,端口:%hu 失败\n", p->name, p->ipaddr, p->portnum);
                    send(p->sock, "没有该用户\n", 17, 0);
                    continue;
                }
                printf("悄悄话: 用户名:%s,IP地址:%s,端口:%hu\n目标用户名:%s,IP地址:%s,端口:%hu,信息:%s",
                       p->name, p->ipaddr, p->portnum, p_to->name, p_to->ipaddr, p_to->portnum, recvmsg);
                bzero(recvmsg, sizeof(recvmsg));
                strcpy(recvmsg, strtok(NULL, " "));
                sprintf(factmsg, "用户名:%s,悄悄话:%s", p->name, recvmsg);
                send(p_to->sock, factmsg, strlen(factmsg), 0);
            }
            else if (strcmp(command, "upload") == 0)
            {
                bzero(recvmsg, sizeof(recvmsg));
                recv(p->sock, recvmsg, sizeof(recvmsg), 0);
                char serverpath[1024] = {0};
                sprintf(serverpath, "serverlib/%s", recvmsg);
                recvfile(serverpath, p);
                printf("用户名:%s,IP地址:%s,端口:%hu用户上传完成\n", p->name, p->ipaddr, p->portnum);
                chmod(serverpath, 0777);
                continue;
            }
            else if (strcmp(command, "download") == 0)
            {
                bzero(recvmsg, sizeof(recvmsg));
                recv(p->sock, recvmsg, sizeof(recvmsg), 0);
                send(p->sock, "lockon\n", 8, 0);
                char serverpath[1024] = {0};
                sprintf(serverpath, "serverlib/%s", recvmsg);
                sleep(1);
                sendfile(serverpath, p);
                printf("用户名:%s,IP地址:%s,端口:%hu用户下载完成\n", p->name, p->ipaddr, p->portnum);
            }
            else if (strcmp(command, "getlist") == 0) // aaa@192.168.1.44@10001#bbb@192.168.1.44@10002#...
            {
                printf("%s\n", online_list);
                sprintf(factmsg, "%s\n", online_list);
                send(p->sock, factmsg, strlen(factmsg), 0);
            }
            else if (strcmp(command, "sendfile") == 0)
            {
                bzero(recvmsg, sizeof(recvmsg));
                recv(p->sock, recvmsg, sizeof(recvmsg), 0);
                char filename[512] = {0};
                strcpy(filename, recvmsg);
                char serverpath[1024] = {0};
                sprintf(serverpath, "serverlib/%s", recvmsg);

                recvfile(serverpath, p);
                printf("用户名:%s,IP地址:%s,端口:%hu用户上传完成\n", p->name, p->ipaddr, p->portnum);
                chmod(serverpath, 0777);

                char toname[10] = {0};
                printf("要传输给的对象用户名是:\n");
                recv(p->sock, toname, sizeof(toname), 0);
                printf("%s\n", toname);
                struct clientlist *p_to;
                p_to = searchpoint(head, p_to, toname);
                if (p_to == NULL)
                {
                    printf("私发: 用户名:%s,IP地址:%s,端口:%hu 失败\n", p->name, p->ipaddr, p->portnum);
                    send(p->sock, "没有该用户\n", 17, 0);
                    continue;
                }
                printf("私发文件: 用户名:%s,IP地址:%s,端口:%hu\n目标用户名:%s,IP地址:%s,端口:%hu\n",
                       p->name, p->ipaddr, p->portnum, p_to->name, p_to->ipaddr, p_to->portnum);

                send(p_to->sock, "accept_file\n", 14, 0);
                sleep(1);
                send(p_to->sock, filename, strlen(filename), 0);
                // printf("%s\n",filename);
                sleep(1);
                send(p_to->sock, "lockon\n", 8, 0);
                printf("%s\n", serverpath);
                sleep(1);
                sendfile(serverpath, p_to);
                printf("用户名:%s,IP地址:%s,端口:%hu用户下载完成\n", p_to->name, p_to->ipaddr, p_to->portnum);
            }
            else if (strcmp(command, "/window") == 0) //  /one name xxxxx
            {
                char toname[10] = {0};
                strcpy(toname, strtok(NULL, " \n"));
                struct clientlist *p_to;
                p_to = searchpoint(head, p_to, toname);
                if (p_to == NULL)
                {
                    printf("私聊: 用户名:%s,IP地址:%s,端口:%hu 失败\n", p->name, p->ipaddr, p->portnum);
                    send(p->sock, "没有该用户\n", 17, 0);
                    continue;
                }
                printf("私聊: 用户名:%s,IP地址:%s,端口:%hu\n目标用户名:%s,IP地址:%s,端口:%hu,信息:%s",
                       p->name, p->ipaddr, p->portnum, p_to->name, p_to->ipaddr, p_to->portnum, recvmsg);
                bzero(recvmsg, sizeof(recvmsg));
                strcpy(recvmsg, strtok(NULL, " "));
                sprintf(factmsg, "用户名:%s,私聊:%s", p->name, recvmsg);
                send(p_to->sock, factmsg, strlen(factmsg), 0);
            }
            else if (strcmp(command, "find") == 0)
            {
                char toname[10] = {0};
                strcpy(toname, strtok(NULL, " \n"));
                struct clientlist *p_to;
                p_to = searchpoint(head, p_to, toname);
                if (p_to == NULL)
                {
                    printf("私聊: 用户名:%s,IP地址:%s,端口:%hu 失败\n", p->name, p->ipaddr, p->portnum);
                    send(p->sock, "没有该用户\n", 17, 0);
                    continue;
                }
                send(p_to->sock, "window\n", 8, 0);
                sleep(1);
                send(p_to->sock, p->name, strlen(p->name), 0);
            }
            else
            {
                printf("没有该命令\n");
                char ask[20] = "没有该命令\n";
                send(p->sock, ask, strlen(ask), 0);
                continue;
            }
        }
    }
    pthread_exit(NULL);
}

/*
主线程-等待连入
用于服务器待机状态，即随时停顿在accep函数中等待新的客户端连入
*/
int main()
{
    int sockfd;
    int ret;
    //定义ipv4地址结构体变量，用来存放你需要绑定的ip和端口号
    struct sockaddr_in bindaddr;
    bzero(&bindaddr, sizeof(bindaddr));
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY宏定义作用:自动匹配本机IP
    bindaddr.sin_port = htons(10000);             //你要绑定的端口号，我自己指定的

    //定义ipv4地址结构体变量，用来存放对方的ip和端口号
    struct sockaddr_in clientaddr; //不需要初始化,清零即可,因为连接时并不能确定是哪个客户端连接过来了
    bzero(&clientaddr, sizeof(clientaddr));
    int clientaddress_len = sizeof(clientaddr);

    //创建套接字--》用来通信(套接字类比手机)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("创建套接字失败!\n");
        return -1;
    }

    //设置端口可复用，不用再等半分钟才能再执行
    int again = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &again, sizeof(again));

    //绑定ip和端口号
    ret = bind(sockfd, (struct sockaddr *)&bindaddr, sizeof(bindaddr));
    if (ret == -1)
    {
        perror("绑定失败!\n");
        return -1;
    }

    //拨号，连接
    ret = connect(sockfd, (struct sockaddr *)&clientaddr, sizeof(clientaddr));
    if (ret == -1)
    {
        perror("连接失败!\n");
        return -1;
    }

    //监听
    ret = listen(sockfd, 5);
    if (ret == -1)
    {
        perror("监听失败!\n");
        return -1;
    }

    head = clientlist_init();
    int newsock;
    pthread_t id;
    while (1)
    {
        //接收连接请求
        newsock = accept(sockfd, (struct sockaddr *)&clientaddr, &clientaddress_len);
        if (newsock == -1)
        {
            perror("接受客户端连接请求失败!\n");
            return -1;
        }
        printf("IP地址:%s,端口:%hu进入服务器,", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        clientlist_insert(head, newsock, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        struct clientlist *p = head;
        while (p->next != NULL)
            p = p->next;
        //创建线程
        pthread_create(&id, NULL, recvmsg_func, p);
    }

    struct clientlist *q = head;
    close(sockfd);
    while (q->next != NULL)
        close(q->sock);
    return 0;
}