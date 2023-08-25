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
#define MAX 1280
#define SIZE_BUF_TIME 80 
#define SIZE_BUFF 128

void free_mem(char * buf1)
{
    free(buf1);	
}

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("./exec -c -i [intervalle] -t [format] -l [nb_fois] prog arg ...\n");
        return 0;
    }
    pid_t pid;
    int status;
    int optch;
    int nsecs;
    int i;
    int cflag;
    int crt;
    int diff;
    char *tps;
    int nb_iter;
    time_t rawtime;
    struct tm *info;
    char buftime[SIZE_BUF_TIME];
    char buf[SIZE_BUFF];
    char *buffer;
    size_t tailcurbuf;
    size_t indcur;
    size_t j;
    size_t nbread;    
    
    tailcurbuf=MAX;
    tps= (char*) NULL;
    nb_iter=-1;
    optch=0;
    nsecs=10000000;
    opterr=1;

    
    while((optch = getopt(argc, argv, "+ci:l:t:"))!=-1) 
    {
        
        switch(optch)
        {
            case 't' :
                printf("parametre t rencontre avec agr %s \n",optarg);
                tps=optarg;
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
            default:
		break;

        }
    }
    
    buffer=(char*)malloc(MAX);
    if (buffer == NULL) 
	{
		printf("erreur allocation mémoire\n");
		exit(1);
	}

    for(i=1; i!=nb_iter+1 ; i++)
    {
        if(tps!=NULL)
        {
            time( &rawtime);
            info=localtime(&rawtime);
            strftime(buftime,SIZE_BUF_TIME,tps,info);
            printf("Formatted date & time : |%s|\n", buftime);
  
        }
        int tube[2];
        if (pipe(tube)==-1) 
		{
			perror("pipe");
			free_mem(buffer); 
            exit(1);
		}
    	pid=fork();
        switch(pid)
        {
            case -1 :
                perror("fork()");
				free_mem(buffer); 
                exit(1);
            case 0 :
                if (close(tube[0])==-1) 
				{
					perror("close");
					free_mem(buffer); 
                	exit(1);
				}
                if (dup2(tube[1],1)==-1) 
				{
					perror("dup2");				
					free_mem(buffer); 
                	exit(1);
				}	
                if (close(tube[1])==-1) 
				{
					perror("close");				
					free_mem(buffer); 
                	exit(1);
				}
                execvp(argv[optind],&argv[optind]);
                perror("execvp");
				free_mem(buffer); 
                exit(1);
            default :
                if (close(tube[1])==-1) 
				{
					perror("close");				
					free_mem(buffer); 
                	exit(1);
				}
                indcur=0;
                diff=0;
                while((nbread=read(tube[0],buf,SIZE_BUFF)) > 0)
                {
                    for(j=0;j<nbread;j++,indcur++)
                    {
                        if(diff==1)
                        {
                            if(indcur>tailcurbuf)
                            {
                                tailcurbuf+=MAX;
                                buffer=realloc(buffer,tailcurbuf);
                                if (buffer == NULL) 
								{
									printf("erreur allocation mémoire\n");
									free_mem(buffer); 
                					exit(1);
								}
                            }
                            buffer[indcur]=buf[j];    
                        }
                        else
                        {
                            if(indcur>tailcurbuf)
                            {
                                tailcurbuf+=MAX;
                                buffer=realloc(buffer,tailcurbuf);
                                if (buffer == NULL) 
								{
									printf("erreur allocation mémoire\n");
									free_mem(buffer); 
                					exit(1);
								}
                            }
                            if(buf[j]!=buffer[indcur])
                            {
                                diff=1;
                                buffer[indcur]=buf[j];
                            }
                        }
                    }
                }

                if (close(tube[0])==-1) 
				{
					perror("close");				
					free_mem(buffer); 
                	exit(1);
				}
 
                if((pid=wait(&status)) == -1) 
				{
					perror("wait(&status)");				
					free_mem(buffer); 
                	exit(1);
				}	
                if(WIFEXITED(status))
                {
                    if(diff==1)
                    {
                        write(1,buffer,indcur);
                    }
                    if(cflag==1 && crt!=WEXITSTATUS(status)) 
                    {
                        printf("Code Retour :%d \n",WEXITSTATUS(status));
                        crt=WEXITSTATUS(status);
                    }
                }
        }
        usleep(nsecs);
    }
    free(buffer);
    return 0;
}




