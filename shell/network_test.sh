#!/bin/bash
#
#	this program is to check the boot information of boot time and netstate

info_file=/root/boot_info.txt		#存放信息的文件	
time_file=/root/boot_time.txt		
count_file=/root/reboot_count.txt	#存放开机次数的文件
target_ip=192.168.5.30				#IP地址，用于测试
self_ip=192.168.5.218				#
self_MAC=00:1d:f3:12:34:56			#MAC地址
eth_name=eth0						#网卡名称


time_now=`date +%T`				#获取当前时间
date +%F%t%T >>$info_file

if [ -e $info_file ]; then
	read <$time_file time_prev 			#从日志中读取上次开机的时间
	read <$count_file reb_cnt			#读取开机次数

	echo $time_now > $time_file
	echo $(($reb_cnt+1)) >$count_file

	time_prev=`date --date="$time_prev" +%s`
	time_now=`date --date="$time_now" +%s`
	time_tmp=$(($time_now-$time_prev))	#计算两次开机的时间差

	if [ $time_tmp -gt 200 ]; then		#若两次开机间隔超过3分钟，报错
		echo "error, $time_tmp seconds more than 3 minutes" >>$info_file	
	else
		echo "$time_tmp seconds" >>$info_file		#将两次开机时间的间隔写入文件
	fi
fi

#获取IP地址与MAC地址
IP="`ifconfig $eth_name |grep "inet[^6]" |sed 's/^.*inet //g' |sed 's/  netmask.*//g'`"	
MAC="`ifconfig $eth_name |grep "ether" |sed 's/^.*ether //g' |sed 's/  txq.*//g'`"

#比较IP地址与MAC
if [ $IP != "192.168.1.251" ]; then
	echo $IP'error' >>$info_file
elif [ $MAC != $self_MAC ]; then
	echo $MAC'error' >>$info_file
fi

ifconfig $eth_name $self_ip				#配置本机临时IP
dmesg |grep -i 'bug\|panic' |grep -v 'port 1'>> $info_file 	#筛选网卡相关信息
dmesg |grep -i 'sda' |grep -i 'byte' >> $info_file

#ping网测试
ping $target_ip -c 2 -s 65500
ping $target_ip -c 10 -s 65500 |grep packet >> $info_file

echo '===================================================================================' >>$info_file

#end
