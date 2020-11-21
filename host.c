#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
typedef struct Bid{int player_id;int price;}BID;
void merge(FILE *pfi[],int depth,int *id){
	BID input[2][10]={};
	for(int i=0;i<2;i++)
		for(int j=0;j<10;j++)
			fscanf(pfi[i],"%d%d",&input[i][j].player_id,&input[i][j].price);
	if(depth==0){
		/*for(int j=0;j<10;j++){
			for(int i=0;i<2;i++)fprintf(stderr,"%d %d ",input[i][j].player_id,input[i][j].price);
			fprintf(stderr,"\n");}*/
		BID pl[8]={};//price->times of win
		for(int i=0;i<8;i++){pl[i].player_id=id[i];}
		for(int j=0;j<10;j++){
			int i=(input[1][j].price>input[0][j].price);
			for(int k=0;k<8;k++){
				if(pl[k].player_id==input[i][j].player_id){pl[k].price++;break;}
			}
		}
		int times[15]={},max=-1,cnt=0;
		for(int i=0;i<8;i++){
			times[pl[i].price]++;
			if(max < pl[i].price) max=pl[i].price;
		}
		for(int i=max;i>=0;i--){
			int temp=cnt+1;
			cnt+=times[i];
			times[i]=temp;
		}
		for(int i=0;i<8;i++)
			printf("%d %d\n",pl[i].player_id,times[pl[i].price]);
	}
	else{
		for(int j=0;j<10;j++){
			int isbigger=(input[1][j].price>input[0][j].price);
			printf("%d %d\n",input[isbigger][j].player_id,input[isbigger][j].price);
		}
	}
	fflush(stdout);fsync(fileno(stdout));
	return;
}
int main(int argc, char **argv,char **envp){
	if(argc!=4){printf("Usage:./host [host_id] [key] [depth]\n");exit(1);}
	int host_id=atoi(argv[1]),depth=atoi(argv[3]);
	FILE *pfi[2]={};
	int PID[2]={};
	if(depth>2||depth<0){fprintf(stderr,"Error depth\n");exit(1);}
	else if(depth==2){//leaf
		while(1){
			//fprintf(stderr,"leaf host\n");
			int player_id[2]={},isfinish=1;
			for(int i=0;i<(1<<(3-depth));i++){scanf("%d",&player_id[i]);isfinish&=(player_id[i]==-1);}
			if(isfinish){
				//fputs("finish 3\n",stderr);fflush(stderr);
				exit(0);
			}
			for(int i=0;i<2;i++){
				int fd[2];pipe(fd);//ch -> fa
				if((PID[i]=fork())<0){fprintf(stderr,"%d Fork error",depth);exit(1);}
				else if(PID[i]==0){//children
					close(fd[0]);dup2(fd[1],STDOUT_FILENO);close(fd[1]);
					char idstr[16]="\0";snprintf(idstr,sizeof(idstr),"%d",player_id[i]);
					char *arg[]={"./player",idstr,(char*)0};execve("./player",arg,envp);
				}
				else{/*father*/close(fd[1]);pfi[i]=fdopen(fd[0],"r");wait(NULL);}
			}
			merge(pfi,depth,NULL);
		}
	}
	else if(depth==1){
		//fprintf(stderr,"depth=%d\n",depth);
		int player_id[4]={};
		FILE *out[2]={NULL,NULL};
		for(int i=0;i<2;i++){
			int fd[2][2];pipe(fd[0]);/*ch->fa*/pipe(fd[1]);/*fa->ch*/
			if((PID[i]=fork())<0){fprintf(stderr,"%d Fork error\n",depth);exit(1);}
			if(PID[i]==0){
				close(fd[0][0]);dup2(fd[0][1],STDOUT_FILENO);close(fd[0][1]);
				close(fd[1][1]);dup2(fd[1][0],STDIN_FILENO);close(fd[1][0]);
				char *arg[]={"./host",argv[1],argv[2],"2",(char*)0};execve("./host",arg,envp);
			}
			else{
				close(fd[0][1]);close(fd[1][0]);
				pfi[i]=fdopen(fd[0][0],"r");
				out[i]=fdopen(fd[1][1],"w");
			}
		}
		while(1){
			int isfinish=1;
			for(int i=0;i<(1<<(3-depth));i++){fscanf(stdin,"%d",&player_id[i]);isfinish&=(player_id[i]==-1);}
			//if(isfinish){fputs("finish 2\n",stderr);fflush(stderr);}
			for(int i=0;i<2;i++){fprintf(out[i],"%d %d\n",player_id[i*2+0],player_id[i*2+1]);
					fflush(out[i]);fsync(fileno(out[i]));}
			if(isfinish){for(int i=0;i<2;i++)wait(NULL);exit(0);}else merge(pfi,depth,NULL);
		}
	}
	else{//depth==0
		//Usage:./host [host_id] [key] [depth]
		int player_id[8]={};
		FILE *out[2]={NULL,NULL};
		//dup2(3+host_id,STDIN_FILENO);close(3+host_id);
		char fname[512]="\0";snprintf(fname,sizeof(fname),"fifo_%d.tmp",host_id);
		int fd=open(fname,O_RDONLY);dup2(fd,STDIN_FILENO);close(fd);
		fd=open("fifo_0.tmp",O_WRONLY);dup2(fd,STDOUT_FILENO);close(fd);
		for(int i=0;i<2;i++){//fork 2 hosts
			int fd[2][2]={};
			pipe(fd[0]);pipe(fd[1]);
			if((PID[i]=fork())<0){fprintf(stderr,"%d:Fork error\n",depth);exit(1);}
			if(PID[i]==0){//child
				close(fd[0][0]);dup2(fd[0][1],STDOUT_FILENO);close(fd[0][1]);
				close(fd[1][1]);dup2(fd[1][0],STDIN_FILENO);close(fd[1][0]);
				char idstr[16]="\0";
				snprintf(idstr,sizeof(idstr),"%d",(depth+1));
				char *arg[]={"./host",argv[1],argv[2],"1",(char*)0};
				execve("./host",arg,envp);
			}
			else{//father
				close(fd[0][1]);close(fd[1][0]);
				pfi[i]=fdopen(fd[0][0],"r");
				out[i]=fdopen(fd[1][1],"w");
			}
		}
		while(1){
			int isfinish=1;
			for(int i=0;i<(1<<(3-depth));i++){fscanf(stdin,"%d",&player_id[i]);isfinish&=(player_id[i]==-1);}
			if(!isfinish)printf("%s\n",argv[2]);
//fputs("input ",stderr);for(int j=0;j<8;j++)fprintf(stderr,"%d%c",player_id[j]," \n"[j==7]);fflush(stderr);
			for(int i=0;i<(1<<(3-depth));i++)
				if(player_id[i]==0){fprintf(stderr,"%d Input Error\n",depth);exit(1);}
			for(int i=0;i<2;i++){
//fputs("to pipe ",stderr);for(int j=0;j<(1<<(2-depth));j++)fprintf(stderr,"%d%c",player_id[j+4*i]," \n"[j==4]);
				for(int j=0;j<(1<<(2-depth));j++)fprintf(out[i],"%d ",player_id[j+4*i]);
				fprintf(out[i],"\n");fflush(out[i]);fsync(fileno(out[i]));
			}
			if(isfinish){for(int i=0;i<2;i++)wait(NULL);exit(0);}
			else{merge(pfi,depth,player_id);}
		}
	}
	return 0;
}
