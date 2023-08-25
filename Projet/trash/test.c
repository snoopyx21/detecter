#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
int main()
{
    char s[4];
    // Open file with write permission (create if doesn't exist).
    sprintf(s,"%d",254);   
        write(1, s,strlen(s));
       sprintf(s,"%d",2);   
        write(1, s, strlen(s));
    sprintf(s,"%d",24);   
        write(1, s, strlen(s));
    sprintf(s,"%d",0);   
        write(1, s,strlen(s));
    
return 0;
}
