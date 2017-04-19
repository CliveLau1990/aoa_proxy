#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <pthread.h>       //for thread
#include "local_service.h"
/*
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
*/
#define HELLO_WORLD_SERVER_PORT    8721
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

struct thread_param{
    int socket_id;
};

struct thread_param threadParam;
struct thread_param mainThreadParam;

pthread_t mainThreadId;
pthread_t recvThreadId;
pthread_t sendThreadId;

int loop_flag = 0;

void *recvThread(void *arg){

    int length;

    struct thread_param *param = (struct thread_param *)arg;
    int socket = param->socket_id;

    printf("recvThread %d\n", socket);

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);

    while(loop_flag){

        length = recv(socket,buffer,BUFFER_SIZE,0 );
        if (length < 0)
        {
            printf("Server Recieve Data Failed!\n");
            loop_flag = 0;
            break;
        }
        printf("receive %s\n", buffer);
    }
}

void *sendThread(void *arg){

    struct thread_param *param = (struct thread_param *)arg;
    int socket = param->socket_id;

    printf("sendThread %d\n", socket);

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);

    while(loop_flag){

        printf("\nStart send\n");

        int length = 12;
        buffer[0] = 'h';
        buffer[1] = 'e';
        buffer[2] = 'l';
        buffer[3] = 'l';
        buffer[4] = 'o';
        buffer[5] = 0;
        //发送buffer中的字符串到new_server_socket,实际是给客户端
        if(send(socket,buffer,length,0)<0)
        {
            printf("Send faield\n");
            loop_flag = 0;
            break;
        }
        sleep(1);
    }
}
 
int start_service(int argc, char **argv)
{
    printf("start_service\n");
    //设置一个socket地址结构server_addr,代表服务器internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

    //创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
    int server_socket = socket(PF_INET,SOCK_STREAM,0);
    if( server_socket < 0)
    {
        printf("Create Socket Failed!");
        exit(1);
    }
    { 
    int opt =1;
    setsockopt(server_socket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    }
     
    //把socket和socket地址结构联系起来
    if( bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr)))
    {
        printf("Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT); 
        exit(1);
    }
 
    //server_socket用于监听
    if ( listen(server_socket, LENGTH_OF_LISTEN_QUEUE) )
    {
        printf("Server Listen Failed!"); 
        exit(1);
    }
    while (1) 
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
 
        int new_server_socket = accept(server_socket,(struct sockaddr*)&client_addr,&length);
        if ( new_server_socket < 0)
        {
            printf("Server Accept Failed!\n");
            break;
        }

        loop_flag = 1;

        threadParam.socket_id = new_server_socket;
        pthread_create(&recvThreadId, NULL, recvThread, &threadParam);
        pthread_create(&sendThreadId, NULL, sendThread, &threadParam);
         
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);

        while(1){
            sleep(1);
            if(loop_flag == 0){
                break;
            }
        }

        printf("close server socket \n");
        //关闭与客户端的连接
        close(new_server_socket);
    }
    //关闭监听用的socket
    close(server_socket);
    return 0;
}

void create_start_service(){

    pthread_create(&mainThreadId, NULL, start_service, &mainThreadParam);
}