#!/bin/sh

HEX_SIZE_ERASE=`cat /proc/mtd | grep -Eo "^mtd0:\s+[0-9a-fA-F]+\s+[0-9a-fA-F]+" | grep -Eo "[0-9a-fA-F]+$" | awk '{print toupper($0)}'`
DEC_SIZE_ERASE=`echo "obase=10;ibase=16;$HEX_SIZE_ERASE"| bc`        #获取擦除块大小
HEX_SIZE_PARTITION=`cat /proc/mtd | grep -Eo "^mtd0:\s+[0-9a-fA-F]+" | grep -Eo "[0-9a-fA-F]+$" | awk '{print toupper($0)}'`
DEC_SIZE_PARTITION=`echo "obase=10;ibase=16;$HEX_SIZE_PARTITION" | bc`  #获取分区大小
DEC_SIZE_FILE=`stat $1 | grep -Eo "Size:\s+[0-9]+" | grep -Eo "[0-9]+"` #文件大小
MOD_BLOCK_FILE=`expr $DEC_SIZE_FILE % $DEC_SIZE_ERASE`                  #获取文件所需块数

if [ $MOD_BLOCK_FILE -gt 0 ]; then
    DEC_BLOCK_FILE=`expr $DEC_SIZE_FILE / $DEC_SIZE_ERASE + 1`          #不能整除+1
else
    DEC_BLOCK_FILE=`expr $DEC_SIZE_FILE / $DEC_SIZE_ERASE`
fi
DEC_BLOCK_SIZE=`expr $DEC_BLOCK_FILE \* $DEC_SIZE_ERASE`
HEX_BLOCK_SIZE=`echo "obase=16;ibase=10;$DEC_BLOCK_SIZE" | bc`
DEC_SIZE_BOOT_MAX=8704       #二次启动代码长度+启动扇区
DEC_SIZE_BOOT=8192           #二次启动代码长度
#echo $HEX_SIZE_ERASE        #分区擦除长度16进制
#echo $DEC_SIZE_ERASE        #分区擦除长度10进制
#echo $HEX_SIZE_PARTITION    #分区大小16进制
#echo $DEC_SIZE_PARTITION    #分区大小10进制
#echo $DEC_SIZE_FILE         #文件大小
#echo $DEC_BLOCK_FILE        #写入文件所需擦除块数
#echo $DEC_BLOCK_SIZE        #写入文件所需擦除的长度10进制
#echo $HEX_BLOCK_SIZE        #写入文件所需擦除的长度16进制
#echo $DEC_SIZE_WRITE        #实际写入大小


if [ $DEC_SIZE_FILE -eq $DEC_SIZE_BOOT_MAX ]; then                          #文件不等于8704
    break
elif [ $DEC_SIZE_FILE -eq $DEC_SIZE_BOOT ]; then                              #文件不等于8192
    break
else
    echo "$1-$DEC_SIZE_FILE is illegal."
    exit 0
fi

#set -x
dd if=/dev/mtd0 of=/tmp/mtd0 bs=4096 count=3 conv=fsync                 #拷贝mtd0的前3个扇区
dd if=/tmp/mtd0 of=/tmp/mtd0_head bs=512 count=1 conv=fsync             #拷贝mtd0的前512字节
dd if=/tmp/mtd0 of=/tmp/mtd0_tail bs=512 skip=17 count=7 conv=fsync     #拷贝mtd0的后512*3字节
                                                                        #拷贝二次启动代码
if [ $DEC_SIZE_FILE -eq $DEC_SIZE_BOOT_MAX ]; then                      #文件不等于启动分区设定
    dd if=$1 of=/tmp/mtd0_mid bs=1 skip=512 count=$DEC_SIZE_BOOT        #写入二次启动代码
else
    dd if=$1 of=/tmp/mtd0_mid bs=1 count=$DEC_SIZE_BOOT                 #写入二次启动代码
fi
cat /tmp/mtd0_head /tmp/mtd0_mid /tmp/mtd0_tail > /tmp/mtd0             #重新拼接成mtd0文件
md5sum /dev/mtdblock0
mtd_debug erase /dev/mtd0 0x0 0x$HEX_BLOCK_SIZE
dd if=/tmp/mtd0 of=/dev/mtdblock0 bs=$DEC_SIZE_ERASE count=$DEC_BLOCK_FILE conv=fsync #重新写入mtd0
md5sum /dev/mtdblock0
#set +x
