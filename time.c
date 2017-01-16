#include<stdio.h>
#include<stdlib.h>
#include<event2/event.h>
#include<event2/util.h>
void callback_print(evutil_socket_t fd, short flag, void *arg)
{
    char *buf=(char *)arg;
    static int num=0;
    printf("%c",buf[num]);
    fflush(stdout);
    num++;
    if(num==11){
        num=0;
        printf("\n");
    }
}

int main(int argc,char **argv)
{
    struct timeval time;
    time.tv_sec=1;
    time.tv_usec=0;
    struct event_base *base=event_base_new();
    if(base==NULL)
    {
        perror("event_base_new error");
        return -1;
    }
    char buf[1024]="I LOVE YOU!";
    struct event *event=event_new(base,-1,EV_READ|EV_PERSIST,callback_print,buf);
    if(event==NULL)
    {
        perror("event_new error");
        event_base_free(base);
        return -2;
    }
    event_add(event,&time);
    event_base_dispatch(base);
    event_free(event);
    event_base_free(base);
    return 0;
}



