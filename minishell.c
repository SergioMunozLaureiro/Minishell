#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

//CONSTS
#define PATH_MAX 4096
#define MAX_JOBS 64
typedef struct{
    int activo; // 0 active, 1 no active
    char comando[1024];
    pid_t pid;
    int numero_job;
}tjobs;
int main(void) {
    char buf[1024];
    char ruta[PATH_MAX];
    tline *line;
    int *pipes;
    pid_t *my_pid;
    int c_hijos;
    int i,j,k;
    int hay_error;
    int fd_redir;
    char *destino;
    int n_jobs=1;
    tjobs jobs[MAX_JOBS]={0};
    pid_t pid_devuelto;
    int status;

    for(i=0;i<MAX_JOBS;i++){
        jobs[i].activo=0;
    }
    //LOOP
    while(1){
            //LOOP CLENAING ZOMBIE SONS

            while((pid_devuelto=waitpid(-1, NULL, WNOHANG))>0){
                
                for(i=0;i<MAX_JOBS;i++){
                    if(jobs[i].activo==1 && jobs[i].pid==pid_devuelto){
                        printf("[%d]- Done                       %s\n",jobs[i].numero_job,jobs[i].comando);
                        n_jobs--;
                        jobs[i].numero_job=n_jobs;
                        jobs[i].activo=0;
                        break;
                    }
                }
            }
            //PROMPT
           printf("msh> ");
           fflush(stdout);
           signal(SIGINT,SIG_IGN);
           signal(SIGTSTP,SIG_IGN);
           if( fgets(buf,sizeof(buf),stdin)==NULL){
            break;
            }
            buf[strcspn(buf, "\n")] = '\0';
            //USE parser.h
            line = tokenize(buf);
            if (line == NULL) {
                continue;
                }
            
            //NO COMMANDS
            if(line->ncommands == 0) {
                continue;
                }
            if(line->ncommands == 1){
            //INTERN COMMANDS
                if(strcmp("exit",line->commands[0].argv[0])==0){
                    if(line->commands[0].argc>1){
                        
                        fprintf(stderr,"%s: Error, demasiados argumentos para comando\n",line->commands[0].argv[0]);
                        continue;
                
                    }
                    else{
                        break;
                    }
                    }
                
                else if(strcmp("cd",line->commands[0].argv[0])==0){
                    if(line->commands[0].argc == 1){
                        destino=getenv("HOME");
                        if(destino ==NULL){
                            fprintf(stderr,"Error a la hora de hacer getenv(`Home`)");
                        }
                        else{
                           if( chdir(destino)!=0){
                                perror("cd");
                                continue;
                           }
                           if(getcwd(ruta,sizeof(ruta))==NULL){
                                perror("getcwd");
                                continue;
                           }
                            
                        }
                        printf("%s\n",ruta);
                    }
                    else if(line->commands[0].argc == 2){
                        
                        destino=line->commands[0].argv[1];
                        if( chdir(destino)!=0){
                                perror("cd");
                                continue;
                           }
                        if(getcwd(ruta,sizeof(ruta))==NULL){
                                perror("getcwd");
                                continue;
                           }
                        printf("%s\n",ruta);

                    }
                    else{
                        fprintf(stderr,"%s: Error, demasiados argumentos para comando\n",line->commands[0].argv[0]);
                        continue;
                    }
                    continue;
                    }
                else if(strcmp("bg",line->commands[0].argv[0])==0){
                    printf("COMANDO INTERNO bg NO IMPLEMENTADO\n");
                    continue;
                    }
                else if(strcmp("jobs",line->commands[0].argv[0])==0){
                    printf("COMANDO INTERNO jobs NO IMPLEMENTADO\n");
                    continue;
                    }
                else if(strcmp("umask",line->commands[0].argv[0])==0){
                    printf("COMANDO INTERNO umask NO IMPLEMENTADO\n");
                    continue;
                    }
                }
            
            hay_error=0;
            //COMMAND DO NOT EXIST
            for(i=0; i<line->ncommands ; i++){
                if((strcmp("exit",line->commands[i].argv[0])==0)||(strcmp("cd",line->commands[i].argv[0])==0) ||
                        (strcmp("bg",line->commands[i].argv[0])==0) || (strcmp("umask",line->commands[i].argv[0])==0) || (strcmp("jobs",line->commands[i].argv[0])==0)){
                            fprintf(stderr,"%s: Mandato no se puede usar con pipes\n",line->commands[i].argv[0]);
                            hay_error=1;
                        }
                else if(line->commands[i].filename==NULL){
                                fprintf(stderr,"%s: No se encuentra el mandato\n",line->commands[i].argv[0]);
                                
                                hay_error=1;
                            }
            }
            if(hay_error==1){
                continue;
            }
            //DYNAMIC LIST FOR NUMBER OF COMMANDS
            my_pid=(pid_t*) malloc(line->ncommands*sizeof(pid_t));
            if(line->ncommands>1){
                //DYNAMIC LIST FOR NUMBER OF PIPES PER COMMAND
                //ONLY ACTIVE WHEN LINE->NCOMMANDS >1
                pipes=(int*) malloc((line->ncommands-1)*2*sizeof(int));
                if (pipes == NULL) {
                    perror("pipes");
                    continue;
                    }
                //OPEN PIPES
                for(j=0;j<line->ncommands-1;j++){
                    pipe(&pipes[2*j]);// PARES= pipe[2*j] IMPARES= pipe[2*j+1]
                    }
                }
            if (my_pid == NULL) {
                perror("malloc");
                continue;
                }
            
            c_hijos=0;
            
            for(i=0; i<line->ncommands ; i++){
                
                my_pid[i]=fork();
                
                if(my_pid[i]<0){
                    perror("fork");
                    break;
                    
                    }
                //SON
                else if(my_pid[i]==0){
                    if(line->background==0){
                        signal(SIGINT,SIG_DFL);
                        signal(SIGTSTP,SIG_DFL);
                    }
                    
                    
                    //MIDDLE COMMAND REDIRECTIONS
                    if(i>0){
                        dup2(pipes[2*(i-1)],STDIN_FILENO);
                        
                    }
                    
                    if(i<line->ncommands -1){
                        dup2(pipes[2*i+1],STDOUT_FILENO);
                    }
                    //FIRST COMMAND INPUT REDIRECTION
                    if(i==0 && line->redirect_input !=NULL){
                        
                        fd_redir=open(line->redirect_input,O_RDONLY);
                        if(fd_redir<0){
                            fprintf(stderr,"%s: Error. %s\n",line->redirect_error,strerror(errno));
                            exit(1);
                        }
                        dup2(fd_redir,STDIN_FILENO);
                        close(fd_redir);
                    }
                    //LAST COMMAND OUTPUT REDIRECTION
                    if(i==line->ncommands -1 && line->redirect_output !=NULL){
                        fd_redir=open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                        if(fd_redir<0){
                            fprintf(stderr,"%s: Error. %s\n",line->redirect_error,strerror(errno));
                            exit(1);
                        }
                        dup2(fd_redir,STDOUT_FILENO);
                        close(fd_redir);
                    }
                    //LAST COMMAND ERROR REDIRECTION
                    if(i==line->ncommands -1 && line->redirect_error !=NULL){
                        fd_redir=open(line->redirect_error, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                        if(fd_redir<0){
                            fprintf(stderr,"%s: Error. %s\n",line->redirect_error,strerror(errno));
                            exit(1);
                        }
                        dup2(fd_redir,STDERR_FILENO);
                        close(fd_redir);
                    }
                    //CLOSE ALL PIPES
                    for(k=0;k<2*(line->ncommands- 1);k++){
                        close(pipes[k]);
                    }
                    //EXECUTE
                    execv(line->commands[i].filename,line->commands[i].argv);
                    perror("execv");
                    exit(1);
                            
                        }
                    //COUNT NUMEBR OF HIJOS
                    c_hijos++;
                    }
            //BACKGROUND
            if(line->background==1){
                for(k=0;k<2*(line->ncommands- 1) ;k++){
                        close(pipes[k]);
                    }
                
                for(i=0;i<MAX_JOBS;i++){
                    if(jobs[i].activo == 0){
                        
                        jobs[i].numero_job=n_jobs++;;
                        jobs[i].pid=my_pid[line->ncommands - 1];
                        jobs[i].activo=1;
                        strcpy(jobs[i].comando, buf);    
                        int len = strlen(jobs[k].comando);
                        while (len > 0 && (jobs[k].comando[len - 1] == '&' || 
                            jobs[k].comando[len - 1] == ' ' || 
                            jobs[k].comando[len - 1] == '\t')) {
                            jobs[k].comando[len - 1] = '\0';
                            len--;
                            }                
                    
                        printf("[%d] %d\n",jobs[i].numero_job,jobs[i].pid);
                        break;
                        }
                    }
                    
                }
                
            //FOREGORUND
            //FATHER
            else{
                //CLOSE ALL PIPES
                for(k=0;k<2*(line->ncommands- 1) ;k++){
                        close(pipes[k]);
                    }  
                for( i=0 ; i<c_hijos ; i++){

                    //WAIT FOR NUMBER OF HIJOS

                    waitpid(my_pid[i],&status,WUNTRACED);
                    if(WIFSIGNALED(status)){
                        printf("\n");
                        break;
                    }
                    if(WIFSTOPPED(status) ){
                        printf("\nGUARDARIA EN JOBS-NO IMPLEMENTADO AÚN");
                        printf("\n[%d]+  Stopped                 %s\n",my_pid[i], buf);
                        break;
                    }
                    }
                      
                }
            //FREE DYNAMIC LISTS
            free(my_pid); 
            if(line->ncommands >1){
                free(pipes);  
            }
            
                    
            }
            if(strcmp("exit",line->commands[0].argv[0])==0){
                exit(0);
                }
        
        return 0;
    
}