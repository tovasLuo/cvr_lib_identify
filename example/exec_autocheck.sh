#!/bin/bash
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin
export PATH

finded=0
finded_usb_serial=''
dev_serial=''
usb_serial=`ls /dev/ttyUSB*` #找到所有的U转串口设备
echo $usb_serial
for serial in ${usb_serial} #循环所有设备
do
    ./autocheck ${serial} #判断该设备是否连接身份证读开器
    if [ $? == 1 ];then
        finded=1
        finded_usb_serial=${serial}
        dev_serial=`udevadm info -a -n ${serial} | grep '{serial}' | head -n 1` #得到身份证读开器设备的serialnum
        break
    fi
done

if [ ${finded} == 1 ];then
    echo ${finded_usb_serial}
    finded_usb_serial=${finded_usb_serial//\//\\\/} #转义,将/转义为\/
    sed -i "s/.*IdCardSerialport.*/IdCardSerialport=${finded_usb_serial}/g" config.ini #使用sed命令(正则表达式)替换config.ini的内容
    dev_serial=${dev_serial#*\"}
    dev_serial=${dev_serial%\"*}
    echo ${dev_serial}
    sed -i "s/.*SerialNum.*/SerialNum=${dev_serial}/g" config.ini #使用sed命令(正则表达式)替换config.ini中的SerialNum
fi
