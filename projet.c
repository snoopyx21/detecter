#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#define MAX 1000
#define SIZE_BUF_TIME 80 

void recup_cmd(const int opt, const int argcc, char ** argvv, char ** cmd)
{
    int k,i,j,taille;
    for(i=opt,k=0; i<argcc;i++,k++)
    {
        
        for(j=0;argvv[i][j] != '\0'; j++);
        taille=j+1;
        cmd[k]=(char *)malloc(taille*sizeof(char));
        for(j=0;argvv[i][j] != '\0' ;j++)
            cmd[k][j]=argvv[i][j];
        cmd[k][j]='\0';
    }
    cmd[k]=(char*) NULL;
} 

int main(int argc, char ** argv)
{
   int diff=0;
    int nb1=1;
   pid_t pid;
    int status;
    int dwhile=0; 
    int optch=0;
    int nsecs=10000;
    int i;
    int cflag=0;
    int crt;
    char ** cmd;
    char *tps= (char *) NULL;
    int nb_iter=0;
    time_t rawtime;
    int tube[2];
    
    ssize_t m;
    char * string;
    string=(char *)malloc(sizeof(char)*MAX);
    struct tm *info;
    char buftime[SIZE_BUF_TIME];    
    opterr=1;
    while((optch = getopt(argc, argv, "+ci:l:t:"))!=-1) 
    {
        
        switch(optch)
        {
            case 't' :
                printf("parametre t rencontre avec agr %s \n",optarg);
                for(i=0;optarg[i] != '\0'; i++);
                tps=(char *) malloc(i*sizeof(char));
                for(i=0;optarg[i] != '\0';i++)
                    tps[i]=optarg[i];
                tps[i]='\0';
                break;
            case 'i' :
                nsecs=atoi(optarg)*1000;
                printf("parametre i rencontre avec agr %s \n",optarg);
                break;
            case 'l' :
                nb_iter=atoi(optarg);
                printf("parametre l rencontre avec agr %s \n",optarg);
                break;
            case 'c' :
                cflag=1;
                crt= -3;
                printf("parametre c rencontre \n");
                break;     
            default : 
                break;      
        }
    }
    cmd=(char **) malloc((argc-(optind-1))*sizeof(char *));
    recup_cmd(optind, argc, argv, cmd);
    for(i=0;i<(argc-(optind-1));i++)   
        printf("cmd[%d] : %s \n", i, cmd[i]);
    pipe(tube);
    for(i=1; i!=nb_iter+1 ; i++)
    {
        if(tps!=NULL)
        {
            time( &rawtime);
            info=localtime(&rawtime);
            strftime(buftime,SIZE_BUF_TIME,tps,info);
            printf("Formatted date & time : |%s|\n", buftime);
  
        }
  
        pid=fork();
        switch(pid)
        {
            case -1 :
                perror("fork()");
                exit(1);
            case 0 :
               // printf("process [son] main \n");
                close(tube[0]);
                dup2(tube[1], 1);
                //dup2(tube[1], 2); 
                close(tube[1]);
                execvp(cmd[0],cmd);
                perror("execvp");
                exit(1);
            default :
                close(tube[1]);
                if((pid=waitpid(pid,&status,0)) == -1)
                    perror("wait(&status)");
                if(WIFEXITED(status) )
                {
                    if(cflag==1 && crt!=WEXITSTATUS(status)) 
                    {
                        printf("Code Retour :%d \n",WEXITSTATUS(status));
                        crt=WEXITSTATUS(status);
                    }
                    while ((m=read(tube[0], string, sizeof(string))) != 0);
                    if ((diff==1) || (nb1==1))
                    {
                        write(1,string, m);
                        nb1=0;
                    }    
                }
                //printf("process [father] main \n");
        }
        usleep(nsecs);
    }
    free(string);
}
























/*int modif(void) // verifier si le fichier est different -> exec cmp
{
    int file_in;
    ssize_t m;
    char buffer[MAX];
    if ((file_in=open("toto",O_RDONLY) == -1))
    {
        perror("open");
        return 0;
    }
    while((m=read(file_in, buffer, sizeof(buffer))) > 0)
    { }
    if( n !=m) 
    {
        n=m;
        return 1;
    }
}
*/
