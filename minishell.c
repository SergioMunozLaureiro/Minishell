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

int main(void) {
    char buf[1024];
    tline *line;
    int *pipes;
    pid_t *my_pid;
    int c_hijos;
    int i,j,k;
    int hay_error;
    int fd_redir;
    //LOOP
    while(1){
            
           printf("msh> ");
           fflush(stdout);
           if( fgets(buf,sizeof(buf),stdin)==NULL){
            break;
            }
            //USE parser.h
            line = tokenize(buf);
            if (line == NULL) {
                continue;
                }
            
            //NO COMMANDS
            if(line->ncommands == 0) {
                continue;
                }
            
            //INTERN COMMANDS
            if(strcmp("exit",line->commands[0].argv[0])==0){
                printf("COMANDO INTERNO exit NO IMPLEMENTADO\n");
                break;
                }
            else if(strcmp("cd",line->commands[0].argv[0])==0){
                printf("COMANDO INTERNO cd NO IMPLEMENTADO\n");
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

            hay_error=0;
            //COMMAND DO NOT EXIST
            for(i=0; i<line->ncommands ; i++){
                 if(line->commands[i].filename==NULL){
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
                    //MIDDLE COMMAND REDIRECTIONS
                    if(i>0){
                        dup2(pipes[2*(i-1)],STDIN_FILENO);
                        
                    }
                    
                    if(i<line->ncommands -1){
                        dup2(pipes[2*i+1],STDOUT_FILENO);
                    }
                    //FIRST COMMAND REDIRECTION
                    if(i==0 && line->redirect_input !=NULL){
                        fd_redir=open(line->redirect_input,O_RDONLY);
                        if(fd_redir<0){
                            fprintf(stderr,"%s: Error. Descripción del error\n",line->redirect_input);
                            exit(1);
                        }
                        dup2(fd_redir,STDIN_FILENO);
                        close(fd_redir);
                    }
                    //LAST COMMAND REDIRECTION
                    if(i==line->ncommands -1 && line->redirect_output !=NULL){
                        fd_redir=open(line->redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                        if(fd_redir<0){
                            fprintf(stderr,"%s: Error. Descripción del error",line->redirect_output);
                            exit(1);
                        }
                        dup2(fd_redir,STDOUT_FILENO);
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
                printf("COMANDO EN BACKGROUND NO IMPLEMENTADO\n");
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
                    waitpid(my_pid[i],NULL,0);
                    }
                      
                }
            //FREE DYNAMIC LISTS
            free(my_pid); 
            if(line->ncommands >1){
                free(pipes);  
            }
            
                    
            }
        
          
        
        
        return 0;
    
}