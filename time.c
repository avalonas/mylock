#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include <signal.h>
#include<unistd.h>
#include <sys/param.h>
#include<linux/reboot.h>
#define PNUM 7
#define SPACE 2
#define USERSPACE 5
static char* sko_token(const char * buf)
{
while(isspace(*buf) )buf++;
while((*buf) && !isspace(*buf))buf++;
return (char*) buf;
}

long total_time(long* ptime)
{
long total=0;
int i ;
for( i = 0 ; i < PNUM ; i++)
   {
    total +=ptime[i];
   }
return total;
}

void de_time(double* ptime,long* de_time,long* de_retime)
{
double total,retotal,batotal;
int i ;
batotal = (double)total_time(de_retime)-(double)total_time(de_time);
for( i = 0 ; i < PNUM ; i++)
   {
    ptime[i]=(de_retime[i]-de_time[i])/batotal;
   }
}

void get_time(long* ptime)
{
char buff[4096+1];
int len , i ;
char* pstr;
FILE* fp;
fp=NULL;
fp=fopen("/proc/stat","r");
if(fp !=NULL)
    {
      len = fread(buff,sizeof(char),sizeof(buff)-1,fp);
      fclose(fp);
    }else
     {
       fclose(fp);
     }
buff[len] ="\0";
pstr=sko_token(buff);
for(i = 0 ; i < PNUM ; i++)
   {
    ptime[i] = strtoul(pstr,&pstr,0);
   }
}

void copy_time(long* de_time,long* de_retime)
{
int i;
for(i = 0; i < PNUM;i++)
   {
    de_time[i]=de_retime[i];
   }
}

void deamon_process()
{
int pid,i;
if(pid=fork())
{
   exit(0);
}else if(pid < 0)
   {
    _exit(1);
   }
setsid();
if(pid=fork())
   {
    exit(0);
   }else if(pid < 0)
    {
     _exit(1);
    }
for( i=0;i< NOFILE;++i)
   {
    close(i);
   }

chdir("/tmp");
umask(0);
signal(SIGCHLD,SIG_IGN);
}

int write_log(const char* pstr)
{
FILE *fp;
time_t timep;
fp = NULL;
if((fp=fopen("test.log","a"))==NULL)
{
   fclose(fp);
   return 0;
}else
   {
    time(&timep);
    fprintf(fp,"%s %s \r\n ",asctime(localtime(&timep)),pstr);
                     fclose(fp);
   }
return 1;
}

void system_reboot()
{
sync();
reboot(LINUX_REBOOT_CMD_RESTART);
}

int main(void)
{
long cp_time[7];
long cp_retime[7];
double d_time[7];
char buf[25];
deamon_process();
sleep(USERSPACE);
get_time(cp_time);
for(;;)
   {
    sleep(SPACE);
    get_time(cp_retime);
    de_time(d_time,cp_time,cp_retime);
    if(!d_time[3])
     {
      write_log("reboot system \r\n");
      system_reboot();
     }
    copy_time(cp_time,cp_retime);
   }
return 0;
}
