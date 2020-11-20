#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){
	if(argv<=0){fprintf(stderr,"No parameter\n");return -1;}
	int player_id = atoi(argv[1]);
	int bid_list[21]={
		20,18,5,21,8,7,2,19,14,13,
		9,1,6,10,16,11,4,12,15,17,3
	};
	for(int i=0;i<10;i++){
		printf("%d %d\n",player_id ,bid_list[player_id+i-1]*100);
	}
	fflush(stdout);fsync(fileno(stdout));
	return 0;
}
