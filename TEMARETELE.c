#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <stdbool.h>
#include <time.h> 


#define nameSize 300

int p1[2], p2[2], fifo[2], s[2], l;
int loghin = 0, ok; 
// p1 - afiseaza p
char start[nameSize], username[nameSize], param[nameSize], inputName[nameSize], command[nameSize], names[nameSize]; 
pid_t pid; 
void mystat(char *myfile)
{
    struct stat status; 
    char perm[10] = "---------";
    /*if(stat(myfile, &status)!=0)
    {
        printf("Nu\n");
        fflush(stdout);
        return;
    }*/
    printf("File Type : "); 
    if((status.st_mode & S_IFMT) == S_IFDIR)
        printf("Directory\n");
    if((status.st_mode & S_IFMT) == S_IFREG)
        printf("Regular File\n");

    if((status.st_mode & S_IFMT) == S_IFIFO)
        printf("FIFO\n");
    if((status.st_mode & S_IFMT) == S_IFLNK)
        printf("Shortcut\n");
    if((status.st_mode & S_IFMT) == S_IFSOCK)
        printf("Socket\n");
    printf("File size : %lld\n", (long long)status.st_size);
    if((status.st_mode & S_IRUSR)) perm[0]='r';
    if((status.st_mode & S_IWUSR)) perm[0]='w';
    if((status.st_mode & S_IXUSR)) perm[0]='x';
    if((status.st_mode & S_IRGRP)) perm[0]='r';
    if((status.st_mode & S_IWGRP)) perm[0]='w';
    if((status.st_mode & S_IXGRP)) perm[0]='x';
    if((status.st_mode & S_IROTH)) perm[0]='r';
    if((status.st_mode & S_IWOTH)) perm[0]='w';
    if((status.st_mode & S_IXOTH)) perm[0]='x';
    printf("Permissions : %s\n", perm);
}


void myfind(char* myfile, char *curr, int *isret){
    DIR* adress1; 
    struct dirent* nextfile; 
    struct stat status; 
    int isDir = 0; 
    char nextcurr[nameSize];
    if(strstr(curr, myfile)){
        printf("FILE FOUND AT: %s \n", curr); 
        *isret = 1; 
        return; 
    }
    stat(curr, &status); 
    if((status.st_mode & S_IFMT) == S_IFDIR )
        isDir = 1;
    if(!isDir)
        return; 
    else
    {
        adress1 = opendir(curr);  
        while((nextfile = readdir(adress1))){
            if(strcmp(".", nextfile->d_name)&&strcmp("..",nextfile->d_name))
            {
                sprintf(nextcurr, "%s/%s", curr, nextfile->d_name); 
                myfind(myfile, nextcurr, isret); 
            }
        }
        closedir(adress1);
    }

}

int main(){
    socketpair(AF_UNIX, SOCK_STREAM, 0, s);
    pipe(p1); pipe(p2);
    mkfifo("fifo",666);
    pid = fork(); 
    
    int fd1 = 0; 
    if(pid>0) { 
        close(p1[0]);
        close(p2[1]);
        while(loghin == 0){
            printf("Login : ");
            memset(inputName, 0, sizeof(inputName));
            scanf("%s", inputName);
            l = strlen(inputName);
            write(p1[1], &l, sizeof(int)); 
            write(p1[1], inputName, l); 
            read(p2[0], &loghin, 4);
            if(loghin==1){
                printf("Login succesful!\nEnter your command below : \n");
                loghin = 1;
            }
            else{
                printf("Incorrect username\n");
            }
        }
        fd1 = open("fifo", O_WRONLY,666); 
        close(s[1]);
        while(1){
            memset(command, 0, nameSize); 
            memset(param, 0, nameSize);
            scanf("%s", command);
            if(!strcmp(command, "mystat") || !(strcmp(command, "myfind"))){
                scanf(" %s",param);
                strcat(command, " "); strcat(command, param);

            }
            l = strlen(command); 
            write(fd1, &l, sizeof(int));
            write(fd1, command, l); 
            if(!(strcmp(command, "quit")))
            {
                
                return 0; 
            } 
            memset(command, 0, 300);
            read(s[0], command, 9);
            printf("%s\n", command);
            fflush(stdout);

        }
        
    }
    if(!pid){
        close(p1[1]); 
        close (p2[0]);
        while(loghin == 0){
            memset(inputName, 0 ,sizeof(inputName));
            read(p1[0], &l, sizeof(int)); 
            read(p1[0], inputName, l);
            FILE * fd = fopen("TEMA.txt", "r"); 
            while(!feof(fd)){
                memset(names, 0 ,sizeof(names));
                fgets(names, sizeof(names), fd); 
                if(names[strlen(names)-1]=='\n') 
                    names[strlen(names)-1]=0; 
                if(!(strcmp(names, inputName)))
                {
                    loghin = 1; 
                    break; 
                }
            
            }
        write(p2[1], &loghin, sizeof(int)); 
        }
        close(s[0]);
        fd1 = open("fifo", O_RDONLY,666);
        while(1){
            read(fd1, &l, sizeof(int));
            read(fd1, command, l); 
            if(!strcmp(command, "quit")){
                return 0; 
            }
            if(command[2]=='f'){
                strcpy(start, "/Users");
                int Found = 0;
                myfind(command+7, start, &Found);
                if(!Found)
                    printf("File not found\n");
                if(Found)
                    printf("Finished\n");
                fflush(stdout);
            }
            if(command[2]=='s'){
                fflush(stdout);
                mystat(command+7);
            }
            memset(command, 0, 300);
            strcpy(command, "finished");
            write(s[1], command, 9 );
            
            
        }
             

    }
}