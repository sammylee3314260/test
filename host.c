#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
typedef struct Bid{int player_id;int price;}BID;
int cmpid(const void *a,const void *b){
	const BID *A=(const BID*)a;
	const BID *B=(const BID*)b;
	return ((A->player_id)-(B->player_id));}
int cmppr(const void *a,const void *b){
	const BID *A=(const BID*)a;
	const BID *B=(const BID*)b;
	return -1*((A->price)-(B->price));}
void merge(FILE *pfi[],int depth,int *id){
	//fprintf(stderr,"merge %d\n",depth);
	BID input[2][10]={};
	for(int i=0;i<2;i++)
		for(int j=0;j<10;j++)
			fscanf(pfi[i],"%d%d",&input[i][j].player_id,&input[i][j].price);
	if(depth==0){
		/*for(int j=0;j<10;j++){
			for(int i=0;i<2;i++)
				fprintf(stderr,"%d %d ",input[i][j].player_id,input[i][j].price);
			fprintf(stderr,"\n");}*/
		BID pl[8]={};//price->times of win
		for(int i=0;i<8;i++){pl[i].player_id=id[i];}
		for(int j=0;j<10;j++){
			int i=(input[1][j].price>input[0][j].price);
			for(int k=0;k<8;k++){
				if(pl[k].player_id==input[i][j].player_id){pl[k].price++;break;}
			}
		}
		qsort(pl,8,sizeof(BID),cmppr);
		int cur=0,cnt=0;
		for(int i=0;i<8;i++){
			cnt++;
			if(i==0){
				cur=pl[i].price;
				pl[i].price=cnt;
			}
			else{
				if(cur>pl[i].price){cur=pl[i].price;pl[i].price=cnt;}
				else{pl[i].price=pl[i-1].price;}
			}
		}
		qsort(pl,8,sizeof(BID),cmpid);
		for(int i=0;i<8;i++)
			printf("%d %d\n",pl[i].player_id,pl[i].price);
	}
	else{
		for(int j=0;j<10;j++){
			int isbigger=(input[1][j].price>input[0][j].price);
			printf("%d %d\n",input[isbigger][j].player_id,input[isbigger][j].price);
			//fprintf(stderr,"%d %d\n",input[isbigger][j].player_id,input[isbigger][j].price);
			/*int cur1=0,cur2=0;
			while(cur1<(1<<(2-depth))||cur2<(1<<(2-depth))){
				if(cur1>=(1<<(2-depth))){
					printf("%d %d ",input[1][j][cur2].player_id,input[1][j][cur2].price);cur2++;}
				else if(cur2>=(1<<(2-depth))||input[0][j][cur1].price>=input[1][j][cur2].price){
					printf("%d %d ",input[0][j][cur1].player_id,input[0][j][cur1].price);cur1++;}
				else{printf("%d %d ",input[1][j][cur2].player_id,input[1][j][cur2].price);cur2++;}
			}*/
		//printf("\n");
		}
	}
	fflush(stdout);
	return;
}
int main(int argc, char **argv,char **envp){
	if(argc!=4){printf("Usage:./host [host_id] [key] [depth]\n");exit(1);}
	int host_id=atoi(argv[1]),key=atoi(argv[2]),depth=atoi(argv[3]);
	FILE *pfi[2]={};
	if(depth>2||depth<0){fprintf(stderr,"Error depth\n");exit(1);}
	else if(depth==2){//leaf
		while(1){
			//fprintf(stderr,"leaf host\n");
			int pid=0,player_id[2]={},isfinish=1;
			for(int i=0;i<(1<<(3-depth));i++){scanf("%d",&player_id[i]);isfinish&=(player_id[i]==-1);}
			if(isfinish)return 0;
			for(int i=0;i<2;i++){
				int fd[2];pipe(fd);//ch -> fa
				if((pid=fork())<0){fprintf(stderr,"%d Fork error",depth);exit(1);}
				else if(pid==0){//children
					close(fd[0]);dup2(fd[1],STDOUT_FILENO);close(fd[1]);
					char idstr[16]="\0";snprintf(idstr,sizeof(idstr),"%d",player_id[i]);
					char *arg[]={"./player",idstr,(char*)0};execve("./player",arg,envp);
				}
				else{/*father*/close(fd[1]);pfi[i]=fdopen(fd[0],"r");/*input pipe fd*/wait(NULL);}
			}
			merge(pfi,depth,NULL);
		}
	}
	else if(depth==1){
		//fprintf(stderr,"depth=%d\n",depth);
		int player_id[4]={};
		FILE *out[2]={NULL,NULL};
		for(int i=0;i<2;i++){
			int PID=0, fd[2][2];pipe(fd[0]);/*ch->fa*/pipe(fd[1]);/*fa->ch*/
			if((PID=fork())<0){fprintf(stderr,"%d Fork error\n",depth);exit(1);}
			if(PID==0){
				close(fd[0][0]);dup2(fd[0][1],STDOUT_FILENO);close(fd[0][1]);
				close(fd[1][1]);dup2(fd[1][0],STDIN_FILENO);close(fd[1][0]);
				char idstr[16]="\0";snprintf(idstr,sizeof(idstr),"%d",(depth+1));
				char *arg[]={"./host",argv[1],"0","2",(char*)0};execve("./host",arg,envp);
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
			for(int i=0;i<2;i++){fprintf(out[i],"%d %d\n",player_id[i*2+0],player_id[i*2+1]);fflush(out[i]);}
			if(isfinish){for(int i=0;i<2;i++)wait(NULL);return 0;}else merge(pfi,depth,NULL);
		}
	}
	else{//depth==0
		//Usage:./host [host_id] [key] [depth]
		int player_id[8]={};
		FILE *out[2]={NULL,NULL};
		dup2(3+host_id,STDIN_FILENO);close(3+host_id);
		int fd=open("fifo_0.tmp",O_WRONLY);dup2(fd,STDOUT_FILENO);close(fd);
		for(int i=0;i<2;i++){//fork 2 hosts
			int PID=0,fd[2][2]={};
			pipe(fd[0]);pipe(fd[1]);
			if((PID=fork())<0){fprintf(stderr,"%d:Fork error\n",depth);exit(1);}
			if(PID==0){//child
				close(fd[0][0]);dup2(fd[0][1],STDOUT_FILENO);close(fd[0][1]);
				close(fd[1][1]);dup2(fd[1][0],STDIN_FILENO);close(fd[1][0]);
				char idstr[16]="\0";
				snprintf(idstr,sizeof(idstr),"%d",(depth+1));
				char *arg[]={"./host",argv[1],"0","1",(char*)0};
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
			//fprintf(stderr,"input\n");
			for(int i=0;i<(1<<(3-depth));i++){
				if(i==((1<<(3-depth))-1)&&player_id[i]==0){fprintf(stderr,"%d Input Error\n",depth);return 0;}
				else if(player_id[i]!=0)break;}
			for(int i=0;i<2;i++){
				//for(int j=0;j<(1<<(2-depth));j++)fprintf(stderr,"w%d ",player_id[j+4*i]);fprintf(stderr,"\n");
				for(int j=0;j<(1<<(2-depth));j++)fprintf(out[i],"%d ",player_id[j+4*i]);
				fprintf(out[i],"\n");fflush(out[i]);
			}
			if(isfinish){for(int i=0;i<2;i++)wait(NULL);return 0;}
			else{merge(pfi,depth,player_id);}
		}
	}
	return 0;
}
