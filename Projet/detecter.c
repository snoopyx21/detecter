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
#include <string.h>
#define MAX 1280
#define SIZE_BUF_TIME 80 
#define SIZE_BUFF 128

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        write(1,"./exec -c -i [intervalle] -t [format] -l [nb_fois] prog arg ...\n",70);
        return 1;
    }
    pid_t pid;
    int status;
    int optch;
    int i;
    int cflag;
    int crt;
    int diff;
    int nb_iter;
    char *tps;
    long nsecs;
    time_t rawtime;
    struct tm *info;
    char buftime[SIZE_BUF_TIME];
    char buf[SIZE_BUFF];
    char s[12]; // pour afficher le code d'erreur,exit prend un int 
        // Or la valeur d'un int est un nombre de 10 chiffres
        // cette valeur peut être aussi négative, d'où taille de 12
        // car il faut aussi prendre en compte le caractère de fin de chaine
        // Même si on devrait avoir un code de retour compris entre 0 et 255 
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
                tps=optarg;
                break;
            case 'i' :
                nsecs=atoi(optarg)*1000;
                if(nsecs <= 0)
                {
                    write(1,"Erreur: intervalle de temps nul ou négatif->impossible\n",60);
                    exit(1);
                }
                break;
            case 'l' :
                nb_iter=atoi(optarg);
                if(nb_iter < 0)
                {
                    write(1,"Erreur: Nombre d'itération négatif impossible\n",50);
                    exit(1);
                }
                break;
            case 'c' :
                cflag=1;
                crt= -3;
                break;
            default:
		break;

        }
    }
    
    if(argv[optind]== NULL)
    {
        write(1,"Erreur: Pas de commande, Pas d'execution\n",43);
        exit(1);
    }

    
    buffer=(char*)malloc(MAX);
    if (buffer == NULL) 
	{
		write(1,"Erreur allocation mémoire\n",30);
		exit(1);
	}

    for(i=1; i!=nb_iter+1 ; i++)
    {
        int tube[2];
        if(tps!=NULL)
        {
            time( &rawtime);
            info=localtime(&rawtime);
            strftime(buftime,SIZE_BUF_TIME,tps,info);
            printf("%s\n", buftime);
  
        }
        if (pipe(tube)==-1) 
		{
			perror("pipe");
			free(buffer); 
            exit(1);
		}
    	pid=fork();
        switch(pid)
        {
            case -1 :
                perror("fork()");
				free(buffer); 
                exit(1);
            case 0 :
                if (close(tube[0])==-1) 
				{
					perror("close");
					free(buffer); 
                	exit(1);
				}
                if (dup2(tube[1],1)==-1) 
				{
					perror("dup2");				
					free(buffer); 
                	exit(1);
				}	
                if (close(tube[1])==-1) 
				{
					perror("close");				
					free(buffer); 
                	exit(1);
				}
                execvp(argv[optind],&argv[optind]);
                perror("execvp");
				free(buffer); 
                exit(1);
            default :
                if (close(tube[1])==-1) 
				{
					perror("close");				
					free(buffer); 
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
									write(1,"erreur allocation mémoire\n",30);
									free(buffer); 
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
									write(1,"erreur allocation mémoire\n",30);
									free(buffer); 
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
					free(buffer); 
                	exit(1);
				}
                if((pid=wait(&status)) == -1) 
				{
					perror("wait(&status)");				
					free(buffer); 
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
                        crt=WEXITSTATUS(status);
                        sprintf(s,"%d",crt);
                        write(1,"exit ",5);   
                        write(1,s,strlen(s));
                        write(1,"\n",1);
                    }
                }
        }
        if(usleep(nsecs) <0)
        {
            perror("Usleep");
            exit(1);
        }
    }
    free(buffer);
    return 0;
}
