#!/bin/sh
hostnum=0
key[8]=0
curhost=0
isoverhostnum=1
function addscore()
{
	for i in `seq 1 8`
	do
		score[${id[$i]}]=$((${score[${id[$i]}]}+8-${ans[$i]}))
	done
#	echo ${score[*]}
}

function waithost()
{
#	echo -e 'waithost \c'
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
#	echo 'ans' ${ans[*]}
#	cnt=1
#	for i in `seq 1 $2`
#	do
#		if [ $i -eq ${id[$cnt]} ];then echo ${ans[$cnt]} '\c'; cnt=$(($cnt+1));
#		else echo '0 \c'; fi
#	done
	addscore
}
#call host===============================================================
function callhost()
{
#		echo 'call host function' ${player[*]}
		curhost=$(($curhost+1))
		#curhost=1
#		echo -e 'hostnum='$hostnum' \c'
		if [ $hostnum -eq 0 ]|| [ $isoverhostnum -eq 0 ]
			then 
				isoverhostnum=0
				waithost
				for i in `seq 1 $1`
				do if [ ${key[$i]} -eq $tmpkey ]
					then curhost=$i; break; fi;done
		fi
#		echo 'curhost='$curhost 'hostnum='$hostnum
		echo ${player[*]} > `echo 'fifo_'$curhost'.tmp'`
		#echo 'hello'
		hostnum=$(($hostnum-1))
}
#end call host===========================================================
#generate players========================================================
max=$2
function player_gen()
{
#	echo $1
	if [ $1 -eq $(($2+1)) ]
	then 
#		echo 'call host function';echo ${player[*]}
		callhost
	elif [ $1 -gt 1 ]&&[ ${player[$(($1-1))]} -eq $max ];then return;
	else
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
check=0 
if [ $1 -gt 10 ] || [ $1 -lt 0 ]
then echo 'n_host = '$1 ' error:0<=n_host<=10'; check=1
elif [ $2 -gt 12 ] || [ $2 -lt 0 ]
then echo 'n_player = '$2 ' error:0<=n_player<=12';check=1
fi
if [ $check -eq 1 ];then exit; fi
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
	key[$i]=$RANDOM
	eval "exec $fd<>`echo 'fifo_'$i'.tmp'`"
	fd=$(($fd+1))
	./host $i ${key[$i]} 0 &
	hostnum=$(($hostnum+1))
	i=$(($i+1))
done
#end run host=========================
#end run $2 host=========================================================
#echo 'hostnum' $hostnum
player_gen 1 8 #generate player and start the auctions

#wait
while [ $hostnum -ne $1 ]; do waithost; done
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
