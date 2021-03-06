/*=============================================================================
#     FileName: mmserver.c
#         Desc: the main file of memory manage server 
#       Author: xiehuc
#        Email: xiehuc@gmail.com
#     HomePage: 
#      Version: 0.0.1
#   LastChange: 2012-12-25 17:10:04
#      History:
=============================================================================*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include "type.h"
#include "mmstore.h"
#include "mmctrl.h"
#include "mmrecord.h"
#include "config.h"

#define tau TAX_RATE
#define max(a,b) a<b?b:a
static double* _x_;
static int prog_quit = 0;
static double old_tau = 0.0;
static double total_mem = 0.0;
static double reverse_mem = 0.0;
static double xi_ = 0.0;
static int reset_ = 0;
static int Verbose = 0;
static const char* Dir = NULL;
static double Tau = 0.0;
static int unit_expand(char u)
{
    switch(u){
        case 'k':
        case 'K':
            return 1;
        case 'm':
        case 'M':
            return 1024;
        case 'G':
            return 1024*1024;
        default:
            return 1;
    }
}
static void build_linear_equ()
{
    int i;
    double Amax = 0;
    double Amean = 0;
    double Total;
    int len = 0;
    Domain* d;
    LIST_FOREACH(d,&domain0.domainu,entries){
        /**
         * 正确计算总内存的数值,因为tg_mem和tot_mem视角不一致,
         * tot_mem是虚拟机内部的内存,系统会占用一部分内存作为内核用,
         * 所以tot_mem<tg_mem.所以一般情况下使用tg_mem.
         *
         * 但是当调节超过气球驱动限制之后,tg_mem>>>tot_mem,tg_mem不再可信
         * 所以要使用tot_mem
         *
         * 另外因为设置了N为固定值,所以tot_mem并不会带来什么严重的问题.
         */
         // 在N=12.5G的时候直接超过70M了, 导致实验错误. 暂时禁用
         //mem_t total = (abs(d->tg_mem - d->tot_mem)> 70*1024)?d->tot_mem:d->tg_mem;
         // 有时候没有调节过来, 导致tg_mem < tot_mem, 然后得出结果为负, 然后整个计算都错误了.
        mem_t total = d->tot_mem;
        double A=(double)(total-d->free_mem);
        _x_[len]=A;
        if(A>Amax) Amax=A;
        Amean+=A;
        len++;
    }
    Total = total_mem;
    Amax/=Total;
    Amean/=len;
#ifdef AUTO_TAX_RATE
    ///*动态曲线1:*/Tau = sqrt(1-pow(1-Amax,2));
    ///*动态曲线2:*/Tau = Tau * 0.8 + 0.2;
    Tau = (xi_ + Amax-1.0/len)/(Amax-Amean/Total);
    Tau = (Tau<0)?0:((Tau>1)?1:Tau);
    Tau = Tau>old_tau?Tau:old_tau-(old_tau-Tau)/10;
    old_tau = Tau;
#else
    Tau = tau;
#endif
    if(reset_) Tau = 0;
    double Part = Total/len-Tau*Amean;
    i=0;
    LIST_FOREACH(d,&domain0.domainu,entries){
        _x_[i]=Part+Tau*_x_[i];
        i++;
    }

    i = 0;
    LIST_FOREACH(d,&domain0.domainu,entries)
        ctrl_update_domain_mem(d,(mem_t)_x_[i++]);
}
static void interupt_server(int sig)
{
    UNUSED(sig);
    prog_quit = 1;
}
static void show_help()
{
    printf("mmserver [-r] -N :Total -f :Reverse \n"
            "\t:Total   the total memory\n"
            "\t:Reverse the reversed free memory\n"
            "\t-r       force set all vm memory are equal\n"
            "\t-v       show more verbose information\n"
            "\t-d       set log output dir\n"
            "example:\n"
            "\tmmserver -N 5G -f 100M\n"
          );
}
/**使网页端显示可行*/
#ifdef ENABLE_SOCK
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
static void * sock_thread(void* no_used)
{
    UNUSED(no_used);
    int server = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9091);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(server,(struct sockaddr*)&addr,sizeof(addr));
    listen(server,10);
    char buf[8192] = {0};
    while(!prog_quit){
        struct sockaddr_in dest_addr;
        socklen_t dest_sz = sizeof(dest_addr);
        int client = 0;
        if((client = accept(server,(struct sockaddr*)&dest_addr,&dest_sz))>0){
            memset(buf,0,sizeof(buf));
            strcat(buf,"[");
            Domain* d;
            LIST_FOREACH(d,&domain0.domainu,entries){
                snprintf(buf+strlen(buf),sizeof(buf)-strlen(buf),"[%lld,%lld,%lld],",d->tot_mem,d->tot_mem-d->free_mem,d->free_mem);
            }
            buf[strlen(buf)-1] = ']';
        }
        write(client,buf,strlen(buf));
        close(client);
    }
    return NULL;
}
#endif
int main(int argc,char** argv)
{
    if(s_h_init()==MM_FAILED){
        return -1;
    }

    char ch;
    char* unit;
    while((ch = getopt(argc, argv, "N:f:hrvd:"))!= -1){
        switch(ch){
            case 'N':
                total_mem = strtod(optarg, &unit);
                total_mem*= unit?unit_expand(*unit):1024*1024;
                break;
            case 'f':
                reverse_mem = strtod(optarg,&unit);
                reverse_mem*= unit?unit_expand(*unit):1024;
                break;
            case 'r':
                reset_ = 1;
                break;
            case 'h':
                show_help();
                return 0;
                break;
            case 'v':
                Verbose = 1;
                break;
            case 'd':
                Dir = optarg;
                break;
        }
    }

    if(total_mem==0.0){
        fprintf(stderr, "-N is required\n");
        return 0;
    }
    if(reverse_mem == 0.0){
        fprintf(stderr, "-f is required\n");
        return 0;
    }
    xi_ = reverse_mem/total_mem;
    ctrl_init();
    s_h_list_domains();
    if(LIST_EMPTY(&domain0.domainu)){
        fprintf(stderr,"it seems you didn't run mmclient in any guest vm.\n"
                "you can install mmclient in your guest vms.\n"
                "and use `sudo services mmclientd start`\n"
                "or `sudo /etc/init.d/mmclientd start` to run mmclient.\n"
               );
        s_h_close();
        return 0;
    }
    signal(SIGINT, interupt_server);
    ctrl_read_domains_maxmem();
    size_t domu_len = 0;
    Domain* d;
    LIST_FOREACH(d,&domain0.domainu,entries){
		  d->min_mem = reverse_mem;
        record_begin(d, Dir);
        domu_len++;
    }
    _x_ = calloc(domu_len, sizeof(*_x_));
#ifdef ENABLE_SOCK
    pthread_t thread = 0;
    pthread_create(&thread,NULL,sock_thread,NULL);
#endif

    //print header 
    printf("Field   ");
    LIST_FOREACH(d,&domain0.domainu, entries){
       char* name = s_h_read_name(d);
       printf("%s   ", name);
       free(name);
    }
    printf("\n");

    while(!prog_quit){
        sleep(2);
        //s_h_wait_change();
        LIST_FOREACH(d,&domain0.domainu,entries){
            s_h_read_domain_mem(d);
            record_mem(d);
        }
        // print table
#ifdef AUTO_TAX_RATE
        printf("τ%.3lf\t",Tau);
#else
        printf("tg:\t");
#endif
        LIST_FOREACH(d,&domain0.domainu,entries)
           printf("%lld\t", d->tg_mem);
        printf("\n");

        if(Verbose){
           printf("tot:\t");
           LIST_FOREACH(d,&domain0.domainu,entries)
              printf("%lld\t", d->tot_mem);
           printf("\nuse:\t");
           LIST_FOREACH(d,&domain0.domainu,entries)
              printf("%lld\t", d->tot_mem - d->free_mem);
           printf("\nfree:\t");
           LIST_FOREACH(d,&domain0.domainu,entries)
              printf("%lld\t", d->free_mem);
           printf("\n\n");
        }
        build_linear_equ();
    }
#ifdef ENABLE_SOCK
    pthread_cancel(thread);
#endif
    LIST_FOREACH(d,&domain0.domainu,entries){
        record_end(d);
    }
    s_h_close();
    ctrl_close();
    free(_x_);
    return 0;
}
