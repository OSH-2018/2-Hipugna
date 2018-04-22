#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

/* 输入的命令行 */
char cmd[256];


int work(char *sen){
	/* 命令行拆解成的各部分，以空指针结尾 */
	char *args[128];


	char *start;
	int i;

	/* 去除句首的空格 */
	for(i = 0; sen[i] ==' ';i++)
		;
	start=sen+i;
    /* 拆解命令行 */
	args[0] = start;
    for (i = 0; *args[i]; i++)
        for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
            if (*args[i+1] == ' ') {
                *args[i+1] = '\0';
				if(args[i+1][1]!=' '){
					*args[i+1]++;
					break;
				}
            }
        args[i] = NULL;

    /* 没有输入命令 */
    if (!args[0]){
        return 1;
	}
    /* 内建命令 */
    if (strcmp(args[0], "cd") == 0) {
        if (args[1])
            chdir(args[1]);
        return 1;
    }
    if (strcmp(args[0], "pwd") == 0) {
        char wd[4096];
        puts(getcwd(wd, 4096));
        return 1;
    }
	if(strcmp(args[0],"export")== 0){
		char *equal;
		for( equal = args[1] ; *equal != '=' && *equal != '\0'; equal++ )
			;
		if(*equal == '\0'){
			printf("input error\n");
			return 1;
		}
		else{
			*equal='\0';
			setenv(args[1],equal+1,1);
			return 1;
		}
	}
    if (strcmp(args[0], "exit") == 0)
        return 0;

    /* 外部命令 */
    pid_t pid = fork();
    if (pid == 0) {
        /* 子进程 */
        execvp(args[0], args);
        /* execvp失败 */
        exit(1);
    }
    /* 父进程 */
    wait(NULL);
	return 1;
}

int control(char *s){

	/* 记录管道情况 */
	int am,i,fd[2],result;
	for( am = 0,i = 0; s[i] != '\0' ; i++ ){
		if( s[i] == '|' ){
			am++;
			break;
		}
	}

	/* 非管道工作 */
	if(am==0){
	return( work(s) );
		
	}
	/* 管道工作 */
	else{
		s[i-1]='\0';//划断
		result=pipe(fd);
		if(result == -1){
			printf("fail to create pipe\n");
			exit(-1);
		}

		pid_t pid=fork();
		if(pid<0){
			printf("fail to fork\n");
			exit(1);
		}
		else if(pid == 0){
			close(fd[0]);
			dup2(fd[1],STDOUT_FILENO);
			work(s);
			close(fd[1]);
			exit(0);
		}
		else{
			int sp;
			sp=dup(STDIN_FILENO);

			close(fd[1]);
			dup2(fd[0],STDIN_FILENO);
			control(s+i+1);
			close(fd[0]);
			wait(NULL);
			dup2(sp,STDIN_FILENO);
			return 1;
		}
	}
}



int main() {
    while (1) {
        /* 提示符 */
        printf("# ");
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */
        int i;
        for (i = 0; cmd[i] != '\n'; i++)
            ;
        cmd[i] = '\0';
		int re;
		re=control(cmd);
		if(!re)return 0;
    }
}
