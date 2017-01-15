#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<event2/event.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>

void callback_input(evutil_socket_t fd, short falg, void *arg)
{
    char buf[4096]={0};
    int len=read(fd,buf,sizeof(buf));
    if(len==1)
    {
        printf("please input message");
        return;
    }
    bufferevent_write((struct bufferevent*)arg,buf,sizeof(buf));
}

void callback_read(struct bufferevent *buffer, void *arg)
{
    char buf[4096]={0};
    bufferevent_read(buffer,buf,sizeof(buf));
    printf("server send:%s\n",buf);
    printf("please input message to server:");
    fflush(stdout);
}

void callback_event(struct bufferevent *buffer, short what, void *arg)
{
    if(what&BEV_EVENT_EOF)
    {
        printf("connection closed\n");
    }
    else if(what&BEV_EVENT_ERROR)
    {
        printf("some other error\n");
    }
    bufferevent_free(buffer);
}

int client_init(char *ip,char *port)
{
    evutil_socket_t socketfd=socket(AF_INET,SOCK_STREAM,0);
    if(socketfd<0)
    {
        perror("socket error");
        return -1;
    }
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(atoi(port));
    inet_aton(ip,&addr.sin_addr);
    int status=connect(socketfd,(struct sockaddr*)&addr,sizeof(addr));
    if(status!=0)
    {
        perror("connect error");
        return -2;
    }
    evutil_make_socket_nonblocking(socketfd);
    return socketfd;
}

int main(int argc,char **argv)
{
    if(argc<3)
    {
        printf("arguement too less,format is[./a.out ip port]\n");
        return -1;
    }
    evutil_socket_t socketfd=client_init(argv[1],argv[2]);
    if(socketfd<0)
    {
        perror("client_init error");
        return -2;
    }
    printf("connect server success......\n");
    struct event_base *base=event_base_new();
    if(base==NULL)
    {
        evutil_closesocket(socketfd);
        perror("event_base_new error");
        return -3;
    }
    struct bufferevent *buffer=bufferevent_socket_new(base,socketfd,BEV_OPT_CLOSE_ON_FREE);
    if(buffer==NULL)
    {
        evutil_closesocket(socketfd);
        perror("bufferevent_socket_new error");
        return -4;
    }
    printf("please input message to server:");
    fflush(stdout);
    struct event *event=event_new(base,STDIN_FILENO,EV_READ|EV_PERSIST,callback_input,buffer);
    if(event==NULL)
    {
        evutil_closesocket(socketfd);
        bufferevent_free(buffer);
        perror("event_new error");
        return -5;
    }
    event_add(event,NULL);
    bufferevent_setcb(buffer,callback_read,NULL,callback_event,event);
    bufferevent_enable(buffer,EV_READ|EV_PERSIST);
    event_base_dispatch(base);
    event_free(event);
    event_base_free(base);
}






