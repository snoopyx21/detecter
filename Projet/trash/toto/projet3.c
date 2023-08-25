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




int main(int argc, char ** argv)
{
    //voir si y'a les argument et tout
    pid_t pid;
    int status;
    int optch=0;
    int nsecs=10000000;
    int i;
    int cflag=0;
    int crt;
    int diff;
    char *tps= (char *) NULL;
    int nb_iter=-1;
    time_t rawtime;
    struct tm *info;
    char buftime[SIZE_BUF_TIME];
    char buf[SIZE_BUFF];
    char *buffer;
    size_t tailcurbuf=MAX;
    size_t indcur;
    size_t j;
    size_t nbread;    
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
   
    
    buffer=(char*)malloc(MAX);

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
        pipe(tube);
		pid=fork();     
        switch(pid)
        {
            case -1 :
                perror("fork()");
                exit(1);
            case 0 :
                close(tube[0]);
                dup2(tube[1],1);
                close(tube[1]);
                execvp(argv[optind],&argv[optind]);
                perror("execvp");
                exit(1);
            default :
                close(tube[1]);
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
                        	}
                        	buffer[indcur]=buf[j];    
                    	}
                    	else
                    	{
                        	if(indcur>tailcurbuf)
                        	{
                            	tailcurbuf+=MAX;
                            	buffer=realloc(buffer,tailcurbuf);
                        	}
                        	if(buf[j]!=buffer[indcur])
                        	{
                            	diff=1;
                            	buffer[indcur]=buf[j];
                        	}
                    	}
                	}
				}
                close(tube[0]);
               	if((pid=waitpid(pid,&status,0)) == -1)
                    perror("wait(&status)");
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
    if (tps!=NULL)
        free(tps);
    return 0;
}













//    cmd=(char **) malloc((argc-(optind-1))*sizeof(char *));
//    recup_cmd(optind, argc, argv, cmd);





/*
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
*/





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
