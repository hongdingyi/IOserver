#include<stdlib.h>
#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<event2/event.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>
static int num=0;
void callback_read(struct bufferevent *buffer, void *arg)
{
    char readBuf[4096]={0};
    bufferevent_read(buffer,readBuf,sizeof(readBuf));
    printf("client%d send:%s",(int)arg,readBuf);
    bufferevent_write(buffer,readBuf,sizeof(readBuf));
}
void callback_event(struct bufferevent *buffer, short what, void *arg)
{
    if(what&BEV_EVENT_EOF)
    {
        printf("client%d closed\n",(int)arg);
    }
    else if(what&BEV_EVENT_ERROR)
    {
        printf("some other error\n");
    }
    bufferevent_free(buffer);
}

void callback_accept(evutil_socket_t fd, short flag, void *arg)
{

    struct event_base *base=(struct event_base *)arg;
    int socketfd=accept(fd,NULL,NULL);
    if(socketfd<0)
    {
        perror("accept error");
        return;
    }
    num++;
    printf("client%d connect success......\n",num);
    struct bufferevent *buffer=bufferevent_socket_new(base,socketfd,BEV_OPT_CLOSE_ON_FREE);
    if(buffer==NULL)
    {
        perror("bufferevent_socket_new error");
        return;
    }
    bufferevent_setcb(buffer,callback_read,NULL,callback_event,num);
    bufferevent_enable(buffer,EV_READ|EV_PERSIST);

}

int server_init(int port,int listennum)
{
    int listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd<0)
    {
        perror("socker error");
        return -1;
    }
    evutil_make_listen_socket_reuseable(listenfd);
    struct sockaddr_in addr;
    addr.sin_port=htons(port);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    int status=bind(listenfd,(struct sockaddr *)&addr,sizeof(addr));
    if(status!=0)
        goto Error;
    status=listen(listenfd,listennum);
    if(status!=0)
        goto Error;
    evutil_make_socket_nonblocking(listenfd);
    return listenfd;
    Error:
    printf("error");
    evutil_closesocket(listenfd);
    return -2;

}

int main(int argc,char  **argv)
{
    evutil_socket_t listenfd=server_init(8080,20);
    if(listenfd<0)
    {
        perror("server_init error");
        return -1;
    }
   struct event_base *base=event_base_new();
   if(base==NULL)
   {
       perror("event_base_new error");
       return -2;
   }
   struct event * event=event_new(base,listenfd,EV_READ|EV_PERSIST,callback_accept,base);
   if(event==NULL)
   {
       perror("event_new error");
       event_base_free(base);
       return -3;
   }
   event_add(event,NULL);
   event_base_dispatch(base);
   event_base_free(base);
   event_free(event);
   return 0;
}











