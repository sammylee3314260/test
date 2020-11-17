#!/bin/sh
hostnum=0
key[8]=0
curhost=0
function addscore()
{
	for i in `seq 1 8`
	do
		score[$i]=$((${score[${id[$i]}]}+8-${ans[$i]}))
	done
}

function waithost()
{
	#echo 'waithost'
	for i in `seq 1 9`
	do
		#echo $i
		read -u 3 -a arr
		if [ $i -eq 1 ]; then tmpkey=${arr[0]}
		else 
			id[$((i-1))]=${arr[0]}
			ans[$(($i-1))]=${arr[1]};fi
	done
	hostnum=$(($hostnum+1))
	#echo 'ans' ${ans[*]}
	addscore
}
#call host===============================================================
function callhost()
{
		#echo 'call host function' ${player[*]}
		curhost=$(($curhost+1))
		#curhost=1
		if [ $hostnum -eq 0 ]
			then 
				waithost
				for i in `seq 1 $1`
				do 
					#if [ -eq $tmpkey ]; then curhost=$i; break; fi
					#echo [ ${key[$i]} -eq $tmpkey ]
					if [ ${key[$i]} -eq $tmpkey ]; then curhost=$i; break; fi
				done
		fi
		echo ${player[*]} > `echo 'fifo_'$curhost'.tmp'`
		#echo 'hello'
		hostnum=$(($hostnum-1))
}
#end call host===========================================================
#generate players========================================================
max=$2
function player_gen()
{
	#echo $1
	if [ $1 -eq $(($2+1)) ]
	then #echo 'call host function'
		#echo ${player[*]}
		callhost
	elif [ $1 -gt 1 ]&&[ ${player[$(($1-1))]} -eq $max ];then return;
	else
	#j=$1
		start=1
		if [ $1 -ne 1 ]; then start=`expr ${player[$(($1-1))]} + 1`;fi
		#for j in `seq $start 1 $max`
		for j in `seq $start  $(($max-$2+$1))`
		do
			#echo 'player' $1
			player[$1]=$j
			player_gen `expr $1 + 1` $2
			#if [ $j -eq $(($max-$2+$1)) ]; then return; fi
		done
	fi
}
#end generate players====================================================

if [ $# -lt 2 ]
	then echo 'usage: sh auction_system.sh [n_host] [n_player]'; exit;
fi
#initialise score======
for i in `seq 1 $2`
do
	score[$i]=0
done
#end init=============
#creat fifo==============================================================
i=0
while [ $i -le $1 ]
do
	mkfifo `echo 'fifo_'$i'.tmp'`
	i=$(($i+1))
done
#end creat fifo==========================================================

#run $2 host=============================================================
#run hosts============================
exec 3<> fifo_0.tmp
fd=4
i=1
while [ $i -le $1 ]
do
	#echo $i
	key[$i]=$(($RANDOM+$RANDOM))
	#./host $i ${key[$i]} 0 3<>`echo 'fifo_'$i'.tmp'`& # > fifo_0.tmp &
	#echo 'exec '$fd'<>`echo fifo_'$i'.tmp`"' # > fifo_0.tmp &
	eval "exec $fd<>`echo 'fifo_'$i'.tmp'`" # > fifo_0.tmp &
	fd=$(($fd+1))
	./host $i ${key[$i]} 0 &
	hostnum=$(($hostnum+1))
	i=$(($i+1))
done
#end run host=========================
#end run $2 host=========================================================
#echo 'player_gen'
player_gen 1 8 #generate player and start the auctions

#wait
if [ $hostnum -ne $1 ]; then waithost; fi
#echo ${score[*]}
for i in `seq 1 $2`
do
	echo $i ${score[$i]}
done
for i in `seq 1 $1`
do
	#cat `echo 'fifo_'$i'.tmp'` &
	echo '-1 -1 -1 -1 -1 -1 -1 -1' > `echo 'fifo_'$i'.tmp'`
done
wait
#delete fifo files=======================================================
rm fifo_*.tmp
#end delete fifo files===================================================
