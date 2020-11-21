#run:host player
#	sh auction_system.sh 10 12
#	rm host player
#	rm *~
#all:host player
host:host.c
	gcc host.c -o host
player:player.c
	gcc player.c -o player
