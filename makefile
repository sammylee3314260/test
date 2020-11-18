#run:host player
#	sh auction_system.sh 10 12
#	rm host player
#	rm *~
host:host.c
	gcc host.c -o host
player:player.c
	gcc player.c -o player
