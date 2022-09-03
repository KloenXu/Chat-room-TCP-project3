#include "allhead.h"
#include "font.h"

/*  编译命令(带上字库)
    arm-linux-gcc *.c -o main -L ./ -lfont
*/

#define N 50

int tsfd; //触摸屏的文件描述符
int x = 0, y = 0;//触摸屏用到的横纵坐标
char sendmsgG[1024] = {0};//发送的信息
char recvmsgG[1024] = {0};//接受到的信息
char member[6][50] = {0};//用于存放在线信息并显示列表的数组
char temppath[1024] = {0};//中转路径,为了避免recvmsgG或者sendmsgG在操作中被改变
int flag = 0;//功能标识1
int input = 0;//功能标识2
int touch_quit = 0;//窗口私聊模式用于退出的表示
pthread_mutex_t lock, lock1;//互斥锁，用于防止下载和接受信息的两个recv相冲突的情况

/*
触发一次触摸屏的函数
参数为横坐标和纵坐标
*/
int touch_screen(int *xpoint, int *ypoint) 
{
    int flag1 = 0;
    struct input_event myevent;

    while (1)
    {
        //读取触摸屏点击的坐标位置  阻塞
        read(tsfd, &myevent, sizeof(myevent));
        if (myevent.type == EV_ABS) //说明确实是触摸屏的驱动
        {
            if (myevent.code == ABS_X) //说明是x坐标
            {
                *xpoint = myevent.value;
                flag1++;
            }
            if (myevent.code == ABS_Y) //说明是y坐标
            {
                *ypoint = myevent.value;
                flag1++;
            }
            if (flag1 == 2)
            {
                flag1 = 0;
                break;
            }
        }
    }
    return 0;
}

char buf[50] = {0};//用于存放输入的信息的数组
int enter = 0;//用于判断账号密码输入完毕
/*
键盘函数，开发板上用于输入账号密码的函数
*/
void keyboard()
{
    bzero(buf, sizeof(buf));
    char show;
    int len = 0, flagk = 0; //, x = 0, y = 0;
    while (1)
    {
        x = 0;
        y = 0;
        flagk = 0;
        usleep(100000);
        touch_screen(&x, &y);
        if (len <= 5)
        {
            if (x >= 15 && x <= 70 && y >= 220 && y <= 275)
                buf[len] = '1';
            else if (x >= 80 && x <= 135 && y >= 220 && y <= 275)
                buf[len] = '2';
            else if (x >= 145 && x <= 200 && y >= 220 && y <= 275)
                buf[len] = '3';
            else if (x >= 210 && x <= 265 && y >= 220 && y <= 275)
                buf[len] = '4';

            else if (x >= 15 && x <= 70 && y >= 285 && y <= 340)
                buf[len] = '5';
            else if (x >= 80 && x <= 135 && y >= 285 && y <= 340)
                buf[len] = '6';
            else if (x >= 145 && x <= 200 && y >= 285 && y <= 340)
                buf[len] = '7';
            else if (x >= 210 && x <= 265 && y >= 285 && y <= 340)
                buf[len] = '8';

            else if (x >= 15 && x <= 70 && y >= 350 && y <= 405)
                buf[len] = '9';
            else if (x >= 80 && x <= 135 && y >= 350 && y <= 405)
                buf[len] = '0';
            else if (x >= 145 && x <= 200 && y >= 350 && y <= 405)
                buf[len] = 'A';
            else if (x >= 210 && x <= 265 && y >= 350 && y <= 405)
                buf[len] = 'B';

            else if (x >= 15 && x <= 70 && y >= 415 && y <= 470)
                buf[len] = 'C';
            else if (x >= 80 && x <= 135 && y >= 415 && y <= 470)
                buf[len] = 'D';
            else if (x >= 145 && x <= 200 && y >= 415 && y <= 470)
                buf[len] = 'E';
            else if (x >= 210 && x <= 265 && y >= 415 && y <= 470)
                buf[len] = 'F';

            else if (x >= 300 && x <= 390 && y >= 400 && y <= 445)
            {
                printf("\n你按了退格键\n");
                flagk = 1;
                buf[len] = '\0';
                len--;
                x = 0;
                y = 0;
                if (len < 0)
                {
                    len = 0;
                }
            }
            else if (x >= 310 && x <= 365 && y >= 255 && y <= 365)
            {
                enter = 1;
                printf("\n你按了回车键");
                break; //右边的回车键
            }
            else
                continue;
        }
        if (x >= 300 && x <= 390 && y >= 400 && y <= 445)
        {
            printf("\n你按了退格键\n");
            flagk = 1;
            buf[len] = '\0';
            len--;
            x = 0;
            y = 0;
            if (len < 0)
            {
                len = 0;
            }
        }
        else if (x >= 310 && x <= 365 && y >= 255 && y <= 365)
        {
            enter = 1;
            printf("\n你按了回车键");
            break; //右边的回车键
        }
        // enter为0时是在输入账号栏的字符,为1时在输入密码栏的字符,在account.c中会给enter做置0
        if (flagk == 0 && enter == 0)
        {
            printf("%c", buf[len]); //到时候可能需要改成字符串%s
            fflush(stdout);
            Clean_Area(245, 107, 300, 27, 0xFFFFFFFF);        //清除区域
            Display_characterX(240, 105, buf, 0x00FF0000, 2); //显示字体
            len++;
        }
        else if (flagk == 1 && enter == 0)
        {
            printf("%c", buf[len]); //到时候可能需要改成字符串%s
            fflush(stdout);
            Clean_Area(245, 107, 300, 27, 0xFFFFFFFF);        //清除区域
            Display_characterX(240, 105, buf, 0x00FF0000, 2); //显示字体
            flagk = 0;
        }
        else if (flagk == 0 && enter == 1)
        {
            printf("%c", buf[len]); //到时候可能需要改成字符串%s
            fflush(stdout);
            Clean_Area(245, 169, 300, 23, 0xFFFFFFFF);        //清除区域
            Display_characterX(240, 165, buf, 0x00FF0000, 2); //显示字体
            len++;
        }
        else if (flagk == 1 && enter == 1)
        {
            printf("%c", buf[len]); //到时候可能需要改成字符串%s
            fflush(stdout);
            Clean_Area(245, 169, 300, 23, 0xFFFFFFFF);        //清除区域
            Display_characterX(240, 165, buf, 0x00FF0000, 2); //显示字体
            flagk = 0;
        }
    }
    printf("\n");
}

/*
登录函数 - 判断用户名是否是已注册的用户名
txtpath - 存放用户名的文件路径
*/
int enter_menu(char *txtpath)
{
    char account[N] = {0};
    char passwd[N] = {0};
    char result[N] = {0};
    char *now_account;
    char *now_passwd;
    int x = 0, y = 0;

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
        Display_characterX(615, 115, "1", 0x00FF0000, 1); //显示字体
        touch_screen(&x, &y);
        if (x >= 240 && x <= 610 && y >= 100 && y <= 140)
        {
            printf("登录:请输入账号:");
            fflush(stdout);
            Display_characterX(620, 115, "1", 0x00FF0000, 1); //显示字体
            keyboard();
            strcpy(account, buf);
            break;
        }
    }

    while (1)
    {
        Display_characterX(615, 175, "1", 0x00FF0000, 1); //显示字体
        touch_screen(&x, &y);
        if (x >= 240 && x <= 610 && y >= 165 && y <= 200)
        {
            printf("登录:请输入密码:");
            fflush(stdout);
            Display_characterX(620, 175, "1", 0x00FF0000, 1); //显示字体
            keyboard();
            strcpy(passwd, buf);
            break;
        }
    }

    enter = 0; //让下一次回到账号密码处时从账号开始显示数据

    if (strlen(passwd) == 0 || strlen(account) == 0)
    {
        printf("账号或密码为空\n");
        return 2;
    }

    while (1)
    {
        fgets(result, sizeof(result), txtfile);
        if (feof(txtfile) == 1)
        {
            printf("账号或密码错误\n");
            return 2;
            break;
        }
        now_account = strtok(result, "@");
        now_passwd = strtok(NULL, "@");
        printf("%s\n", now_account);
        printf("%s\n", now_passwd);
        if ((strcmp(account, now_account) == 0) &&
            (strncmp(passwd, now_passwd, (strlen(now_passwd) - 1)) == 0) &&
            ((strlen(account)) == (strlen(now_account))) &&
            ((strlen(passwd)) == (strlen(now_passwd) - 1)))
        {
            if (strcmp(account, "bbb") != 0)
                return 1;
        }
    }

    fclose(txtfile);
    return 0;
}

/*
打开触摸屏的函数
*/
int open_touchscreen() 
{
    //打开触摸屏的驱动
    tsfd = open("/dev/input/event0", O_RDWR);
    if (tsfd == -1)
    {
        perror("打开触摸屏失败!\n");
        return -1;
    }
    return 0;
}

/*
关闭触摸屏的函数
*/
int close_touchscreen()
{
    //关闭触摸屏
    close(tsfd);
    return 0;
}

/*
发送文件函数 - 用于发送文件到目标客户端
sendpath - 所要发送文件的路径
sockfd   - 与服务器相连的套接字
*/
int sendfile(char *sendpath, int sockfd)
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
        if (ret == 0)
        {
            // printf("传输完成\n");
            break;
        }
        send(sockfd, sendfile, ret, 0);
    }
    close(sendfd);
    return 0;
}

/*
显示全屏图片函数
bmppath - 文件路径
*/
int showfullpicture(const char *bmppath)
{
    int bmpfd, lcdfd, *lcdpoint;
    int i;
    int x = 0, y = 0; //最终效果为从(0,0)开始打印图片,但是实际上是从最后一行479行开始往上打印图片
    char bmpbuf[800 * 480 * 3] = {0};
    int lcdbuf[800 * 480] = {0};

    //打开图片文件
    bmpfd = open(bmppath, O_RDWR);
    if (bmpfd == -1)
    {
        perror("打开picture失败\n");
        return -1;
    }

    //打开液晶屏的驱动
    lcdfd = open("/dev/fb0", O_RDWR);
    if (lcdfd == -1)
    {
        perror("打开lcd失败\n");
        return -1;
    }

    //映射得到lcd在内存中的首地址
    lcdpoint = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcdfd, 0);
    if (lcdpoint == NULL)
    {
        perror("映射lcd失败\n");
        return -1;
    }

    //跳过图片最前面的54字节的头信息
    lseek(bmpfd, 54, SEEK_SET);

    //从第55字节开始读取
    read(bmpfd, bmpbuf, 800 * 480 * 3);

    //把三个字节的BGR数据转换成四个字节ARGB
    for (i = 0; i < 800 * 480; i++)
    {
        lcdbuf[i] = 0x00 << 24 | bmpbuf[3 * i + 2] << 16 | bmpbuf[3 * i + 1] << 8 | bmpbuf[3 * i];
    }

    //把图片每一行像素点拷贝到液晶屏对应的位置
    for (i = 0; i < 480; i++)
    {
        memcpy(lcdpoint + (y + 479 - i) * 800 + x, &lcdbuf[i * 800], 800 * 4); //最终效果为从(0,0)开始打
    }                                                                          //印图片,实际为从最后一行
                                                                               //开始向首行打印图片
    //关闭文件
    close(bmpfd);
    close(lcdfd);
    //解除映射
    munmap(lcdpoint, 800 * 480 * 4);
}

/*
显示任意大小图片函数
bmppath - 文件路径
x - 起始横坐标
y - 起始纵坐标
w - 宽度
h - 高度
*/
int showanybmp(const char *bmppath, int x, int y, int w, int h)
{
    int bmpfd;
    int lcdfd;
    int i;

    int w_change;
    w_change = w;
    while (w_change % 4 != 0)
    {
        w_change++;
    }

    //定义指针保存lcd在内存中的首地址
    int *lcdpoint;
    //定义数组存放你读取到的bmp图片的颜色值
    char bmpbuf[w_change * h * 3];
    //定义数组存放转换得到的ARGB数据
    int lcdbuf[w_change * h]; // int类型数据占4个字节

    //打开你要显示的w*h的bmp图片
    bmpfd = open(bmppath, O_RDWR);
    if (bmpfd == -1)
    {
        perror("打开bmp图片失败了!\n");
        return -1;
    }

    //打开液晶屏的驱动
    lcdfd = open("/dev/fb0", O_RDWR);
    if (lcdfd == -1)
    {
        perror("打开lcd失败了!\n");
        return -1;
    }

    //映射得到lcd在内存中的首地址
    lcdpoint = mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, lcdfd, 0);
    if (lcdpoint == NULL)
    {
        perror("获取lcd的首地址失败了!\n");
        return -1;
    }

    //跳过图片最前面的54字节的头信息
    lseek(bmpfd, 54, SEEK_SET);

    //从第55字节开始读取
    read(bmpfd, bmpbuf, w_change * h * 3);

    //把三个字节的BGR数据转换成四个字节ARGB
    for (i = 0; i < w_change * h; i++)
        lcdbuf[i] = 0x00 << 24 | bmpbuf[3 * i + 2] << 16 | bmpbuf[3 * i + 1] << 8 | bmpbuf[3 * i];

    //把图片每一行像素点拷贝到液晶屏对应的位置
    // 399x250  被填充成400*250     499x200 被填充成500x200
    for (i = 0; i < h; i++)
        memcpy(lcdpoint + (y + h - 1 - i) * 800 + x, &lcdbuf[i * w_change], w * 4);

    //关闭
    close(bmpfd);
    close(lcdfd);
    //解除映射
    munmap(lcdpoint, 800 * 480 * 4);
    return 0;
}

/*
下载文件函数
downloadpath - 下载后存放的地址路径
sockfd - 与服务器相连的套接字
*/
int downloadfile(char *downloadpath, int sockfd)
{
    int recvfd, ret;
    recvfd = open(downloadpath, O_RDWR | O_CREAT | O_TRUNC);
    if (recvfd == -1)
    {
        recvfd = open(downloadpath, O_RDWR | O_TRUNC);
    }
    char recvfile[1024] = {0};
    while (1)
    {
        bzero(recvfile, sizeof(recvfile));
        ret = recv(sockfd, recvfile, sizeof(recvfile), 0);
        // printf("%d\n", ret);
        write(recvfd, recvfile, ret);
        if (ret < sizeof(recvfile))
        {
            // printf("上传完成1\n");
            break;
        }
    }
    close(recvfd);
    return 0;
}

/*
触屏任务线程
*/
void *touch_task(void *arg)
{
    int sockfd = *((int *)arg);
    while (1)
    {
        if (flag == 3 && input == 8)
        {
            touch_screen(&x, &y);
            if (x > 750 && x < 799 && y > 410 && y < 478) //窗口模式下退出按钮
            {
                printf("点击了窗口私聊退出键,请再按一次回车确认退出\n");
                Clean_Area(1, 1, 700, 470, 0xFFFFFFFF);
                Display_characterX(1, 1, "please press enter again to quit", 0x00000000, 2);
                touch_quit = 1;
                flag = 0;
            }
            continue;
        }
        pthread_mutex_lock(&lock);
        touch_screen(&x, &y);
        if (x > 2 && x < 130 && y > 2 && y < 60) //刷新在线人数按钮
        {
            printf("点击了刷新在线人数按钮\n");
            send(sockfd, "getlist\n", 9, 0);
            flag = 1;
        }
        else if (x > 425 && x < 500 && y > 35 && y < 105)
        {
            printf("点击了发送所有人文字信息\n");
            input = 1;
        }
        else if (x > 590 && x < 735 && y > 35 && y < 105)
        {
            pthread_mutex_unlock(&lock);
            printf("点击了发送表情包\n");
            input = 7;
            usleep(10000);
            pthread_mutex_lock(&lock);
        }
        else if (x > 590 && x < 665 && y > 1 && y < 34)
        {
            pthread_mutex_unlock(&lock);
            printf("点击了上传文件\n");
            input = 2;
            usleep(10000);
            pthread_mutex_lock(&lock);
        }
        else if (x > 665 && x < 735 && y > 1 && y < 34)
        {
            printf("点击了下载文件\n");
            input = 5;
        }
        else if (x > 750 && x < 798 && y > 1 && y < 105)
        {
            printf("点击了退出键,执行quit命令\n");
            input = 3;
        }
        else if (x > 501 && x < 575 && y > 35 && y < 105)
        {
            printf("点击了私发文字信息\n");
            input = 4;
        }
        else if (x > 280 && x < 410 && y > 2 && y < 60)
        {
            pthread_mutex_unlock(&lock);
            printf("点击了窗口私聊\n");
            input = 8;
            flag = 3;
            usleep(10000);
            pthread_mutex_lock(&lock);
        }
        pthread_mutex_unlock(&lock);
    }
}

/*
接收信息线程
*/
void *myrecvmsg(void *arg)
{
    int sockfd = *((int *)arg);
    int ret;
    int window_left = 0;
    char tempmsg[1024] = {0};
    while (1)
    {
        pthread_mutex_unlock(&lock1);
        bzero(recvmsgG, sizeof(recvmsgG));
        bzero(tempmsg, sizeof(tempmsg));
        ret = recv(sockfd, recvmsgG, sizeof(recvmsgG), 0);
        printf("%s", recvmsgG);
        if (strcmp(recvmsgG, "lockon\n") == 0)
        {
            sleep(1);
            pthread_mutex_lock(&lock1);
            printf("unlock\n");
            continue;
        }
        if (strcmp(recvmsgG, "accept_file\n") == 0)
        {
            char path[1024] = {0};
            recv(sockfd, path, sizeof(path), 0);
            printf("%s\n", path);
            strcpy(temppath, path);
            input = 6;
            continue;
        }
        strcpy(tempmsg, recvmsgG);
        if (ret == 0)
            exit(0);
        if (flag == 1)
        {
            Clean_Area(1, 120, 399, 340, 0x00FFFFFF);
            int i, count = 0;
            for (i = 0; i <= strlen(tempmsg) - 1; i++)
            {
                if (tempmsg[i] == '#')
                    count++;
            }
            for (i = 0; i <= count - 1; i++)
                bzero(member[i], sizeof(member[i]));
            strcpy(member[0], strtok(tempmsg, "#"));
            for (i = 1; i <= count - 1; i++)
            {
                strcpy(member[i], strtok(NULL, "#"));
            }
            for (i = 0; i <= count - 1; i++)
            {
                printf("%s\n", member[i]);
                Display_characterX(10, 130 + i * 60, member[i], 0x00FF0000, 2);
            }
            flag = 0;
        }
        if (flag == 3)
        {
            if (strncmp(tempmsg, "用户名:", 10) != 0)
            {
                continue;
            }
            else
            {
                char print_window[1024] = {0};
                char news[512] = {0};
                char newsname[10] = {0};
                strtok(tempmsg, ":,");
                strcpy(newsname, strtok(NULL, ":,"));
                strtok(NULL, ":,");
                strcpy(news, strtok(NULL, ":,"));
                sprintf(print_window, "user:%s %s", newsname, news);
                Display_characterX(10, 10 + window_left * 60, print_window, 0x00FF0000, 2);
                window_left++;
                if (window_left >= 8)
                {
                    sleep(3);
                    Clean_Area(1, 1, 360, 470, 0xFFFFFFFF);
                    window_left = 0;
                }
                if (strcmp(news, "quit\n") == 0)
                {
                    flag = 0;
                }
            }
        }
    }
}

/*
主线程
*/
int main()
{
    Init_Font();
    open_touchscreen();
    int one = 1;
    while (1)
    {
        showfullpicture("login.bmp");
        touch_screen(&x, &y);
        if (x >= 625 && x <= 775 && y >= 350 && y <= 415) //登陆键
        {
            Display_characterX(395, 210, "please touch account", 0x00FF0000, 2); //显示字体
            Display_characterX(395, 260, "and password to input", 0x00FF0000, 2);
            if (one == enter_menu("/project3/password.txt"))
            {
                break;
            }
        } //显示字体
        else
            continue;
    }

    int sockfd;
    int ret;
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&lock1, NULL);

    showfullpicture("choose.bmp");
    showanybmp("emoji1.bmp", 450, 200, 100, 100);
    showanybmp("emoji2.bmp", 650, 200, 100, 100);
    showanybmp("emoji3.bmp", 450, 360, 100, 100);
    showanybmp("emoji4.bmp", 650, 360, 100, 100);

    //定义ipv4地址结构体变量，用来存放你需要绑定的ip和端口号
    struct sockaddr_in bindaddr;
    bzero(&bindaddr, sizeof(bindaddr));
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY宏定义作用:自动匹配本机IP
    bindaddr.sin_port = htons(10004);             //你要绑定的端口号，我自己指定的

    //定义ipv4地址结构体变量，用来存放对方的ip和端口号
    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr("192.168.1.44"); //对方的ip地址
    serveraddr.sin_port = htons(10000);                     //对方的端口号

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

    //拨号
    ret = connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    if (ret == -1)
    {
        perror("连接失败!\n");
        return -1;
    }

    //多线程进行双向通信
    pthread_t id;
    pthread_create(&id, NULL, myrecvmsg, &sockfd);

    //第一次打开客户端时输入用户名
    //fgets(sendmsgG, 1024, stdin);
    strcpy(sendmsgG,"bbb");
    send(sockfd, sendmsgG, strlen(sendmsgG), 0);

    //多线程进行触摸屏操作
    pthread_t touch_id;
    pthread_create(&touch_id, NULL, touch_task, &sockfd);

    char factmsg[2048] = {0};
    while (1)
    {
        if (input == 1)
        {
            setbuf(stdin, NULL);
            char command[10] = "/all ";
            bzero(sendmsgG, sizeof(sendmsgG));
            bzero(factmsg, sizeof(factmsg));
            printf("请输入所有人信息\n");
            fgets(sendmsgG, 1024, stdin);
            sprintf(factmsg, "%s%s", command, sendmsgG);
            ret = send(sockfd, factmsg, strlen(factmsg), 0);
            if (ret == 0)
                break;
            input = 0;
        }
        else if (input == 2)
        {
            setbuf(stdin, NULL);
            char command[10] = "upload\n";
            send(sockfd, command, strlen(command), 0);
            pthread_mutex_lock(&lock);
            char sendpath[512] = {0};
            printf("请点击表情包\n");
            while (1)
            {
                touch_screen(&x, &y);
                if (x > 450 && x < 550 && y > 200 && y < 300)
                {
                    strcpy(sendpath, "emoji1.bmp");
                    break;
                }
                else if (x > 650 && x < 750 && y > 200 && y < 300)
                {
                    strcpy(sendpath, "emoji2.bmp");
                    break;
                }
                else if (x > 450 && x < 550 && y > 360 && y < 460)
                {
                    strcpy(sendpath, "emoji3.bmp");
                    break;
                }
                else if (x > 650 && x < 750 && y > 360 && y < 460)
                {
                    strcpy(sendpath, "emoji4.bmp");
                    break;
                }
                else
                    continue;
            }
            send(sockfd, sendpath, strlen(sendpath), 0);
            sleep(1);
            sendfile(sendpath, sockfd);
            printf("上传完成\n");
            input = 0;
            pthread_mutex_unlock(&lock);
        }
        else if (input == 3)
        {
            char command[10] = "quit\n";
            send(sockfd, command, strlen(command), 0);
            input = 0;
            break;
        }
        else if (input == 4)
        {
            char command[10] = "/one ";
            bzero(sendmsgG, sizeof(sendmsgG));
            bzero(factmsg, sizeof(factmsg));
            printf("请输入悄悄话对象与信息,用空格隔开\n");
            fgets(sendmsgG, 1024, stdin);
            sprintf(factmsg, "%s%s", command, sendmsgG);
            ret = send(sockfd, factmsg, strlen(factmsg), 0);
            if (ret == 0)
                break;
            input = 0;
        }
        else if (input == 5)
        {
            char command[10] = "download\n";
            send(sockfd, command, strlen(command), 0);
            char clientpath[1024] = {0};
            char recvpath[512] = {0};
            printf("请输入需要下载的文件名\n");
            scanf("%s", recvpath);
            pthread_mutex_lock(&lock1);
            send(sockfd, recvpath, strlen(recvpath), 0);
            sprintf(clientpath, "download/%s", recvpath);
            downloadfile(clientpath, sockfd);
            printf("下载完成\n");
            pthread_mutex_unlock(&lock1);
            input = 0;
        }
        else if (input == 6)
        {
            char clientpath[1024] = {0};
            pthread_mutex_lock(&lock1);
            sprintf(clientpath, "download/%s", temppath);
            printf("%s\n", clientpath);
            downloadfile(clientpath, sockfd);
            printf("下载完成\n");
            pthread_mutex_unlock(&lock1);
            input = 0;
            bzero(temppath, sizeof(temppath));
            chmod(clientpath,0777);
            showanybmp(clientpath,1,379,100,100);
            sleep(2);
            Clean_Area(1,379,100,100,0xFFFFFFFF);
            setbuf(stdin, NULL);
        }
        else if (input == 7)
        {
            setbuf(stdin, NULL);
            char command[10] = "sendfile\n";
            send(sockfd, command, strlen(command), 0);
            pthread_mutex_lock(&lock);
            char sendpath[512] = {0};
            printf("请点击表情包\n");
            while (1)
            {
                touch_screen(&x, &y);
                if (x > 450 && x < 550 && y > 200 && y < 300)
                {
                    strcpy(sendpath, "emoji1.bmp");
                    break;
                }
                else if (x > 650 && x < 750 && y > 200 && y < 300)
                {
                    strcpy(sendpath, "emoji2.bmp");
                    break;
                }
                else if (x > 450 && x < 550 && y > 360 && y < 460)
                {
                    strcpy(sendpath, "emoji3.bmp");
                    break;
                }
                else if (x > 650 && x < 750 && y > 360 && y < 460)
                {
                    strcpy(sendpath, "emoji4.bmp");
                    break;
                }
                else
                    continue;
            }
            send(sockfd, sendpath, strlen(sendpath), 0);
            sleep(1);
            sendfile(sendpath, sockfd);
            printf("上传完成\n");
            pthread_mutex_unlock(&lock);

            char name[10] = {0};
            printf("请输入要传输给的对象用户名:\n");
            scanf("%s", name);
            send(sockfd, name, strlen(name), 0);
            input = 0;
        }
        else if (input == 8)
        {
            flag = 3;
            pthread_mutex_lock(&lock1);
            bzero(factmsg, sizeof(factmsg));
            char command[10] = "/window ";
            char com_name[100] = {0};
            strcat(com_name, command);
            char tempname[10] = {0};
            char findname[50] = {0};
            while (1)
            {
                touch_screen(&x, &y);
                if (x > 1 && x < 410 && y > 130 && y < 160)
                {
                    printf("%s\n", member[0]);
                    strcpy(tempname, strtok(member[0], "@"));
                    strcat(com_name, tempname);
                    sprintf(findname, "find %s", tempname);
                    send(sockfd, findname, strlen(findname), 0);
                    strcat(com_name, " ");
                    printf("%s\n", com_name);
                    break;
                }
                else if (x > 1 && x < 410 && y > 185 && y < 215)
                {
                    printf("%s\n", member[1]);
                    strcpy(tempname, strtok(member[1], "@"));
                    strcat(com_name, tempname);
                    sprintf(findname, "find %s", tempname);
                    send(sockfd, findname, strlen(findname), 0);
                    strcat(com_name, " ");
                    printf("%s\n", com_name);
                    break;
                }
                else if (x > 1 && x < 410 && y > 250 && y < 280)
                {
                    printf("%s\n", member[2]);
                    strcpy(tempname, strtok(member[2], "@"));
                    strcat(com_name, tempname);
                    sprintf(findname, "find %s", tempname);
                    send(sockfd, findname, strlen(findname), 0);
                    strcat(com_name, " ");
                    printf("%s\n", com_name);
                    break;
                }
            }
            showfullpicture("window.bmp");
            char smsg[512] = {0};
            int window_right = 0;
            while (1)
            {
                bzero(smsg, sizeof(smsg));
                bzero(factmsg, sizeof(factmsg));
                if (touch_quit == 0)
                {
                    fgets(smsg, 512, stdin);
                    sprintf(factmsg, "%s%s", com_name, smsg);
                    send(sockfd, factmsg, strlen(factmsg), 0);
                    Display_characterX(380, 10 + window_right * 60, smsg, 0x000000FF, 2);
                    window_right++;
                    if (window_right >= 8)
                    {
                        sleep(3);
                        Clean_Area(378, 1, 360, 470, 0xFFFFFFFF);
                        window_right = 0;
                    }
                    if (strcmp(smsg, "quit\n") == 0)
                    {
                        break;
                    }
                }
                else if (touch_quit == 1)
                {
                    strcpy(smsg, "quit\n");
                    sprintf(factmsg, "%s%s", com_name, smsg);
                    send(sockfd, factmsg, strlen(factmsg), 0);
                    Display_characterX(380, 10 + window_right * 60, smsg, 0x000000FF, 2);
                    window_right++;
                    if (window_right >= 8)
                    {
                        sleep(3);
                        Clean_Area(378, 1, 360, 470, 0xFFFFFFFF);
                        window_right = 0;
                    }
                    break;
                }
            }
            input = 0;
            flag = 0;
            touch_quit = 0;
            showfullpicture("choose.bmp");
            showanybmp("emoji1.bmp", 450, 200, 100, 100);
            showanybmp("emoji2.bmp", 650, 200, 100, 100);
            showanybmp("emoji3.bmp", 450, 360, 100, 100);
            showanybmp("emoji4.bmp", 650, 360, 100, 100);
            pthread_mutex_unlock(&lock1);
        }
    }

    close(sockfd);
    close_touchscreen();
    UnInit_Font();
    pthread_mutex_destroy(&lock);
    return 0;
}