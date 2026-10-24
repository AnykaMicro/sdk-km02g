#!/bin/sh
DIR_RESERVE="/tmp/reserve"
DEV_NODE="/dev/reserved_ram0"
HEX_MAX_SIZE=2000000                                                             #最大的内存保留区间大小32MB,此处为16进制
if [ -z "$1" ]; then
    FS_TYPE="vfat"
else
    FS_TYPE=$1
fi

mount_reserve_dir()
{
    HEXDUMP_ADDR=`hexdump -C /sys/firmware/devicetree/base/ion/ion_carveout_heap/base-address | grep -Eo "([0-9A-Fa-f]{2} ){8}+" | sed "s# ##g" | sed 's#^0*##g' | awk '{print toupper($0)}'`
    HEXDUMP_SIZE=`hexdump -C /sys/firmware/devicetree/base/ion/ion_carveout_heap/size | grep -Eo "([0-9A-Fa-f]{2} ){8}+" | sed "s# ##g" | sed 's#^0*##g' | awk '{print toupper($0)}'`

    #DEC_HEXDUMP_SIZE=`echo "obase=10;ibase=16;$HEXDUMP_SIZE"|bc`                #获取ion分区的大小10进制值
    #DEC_MAX_SIZE=`echo "obase=10;ibase=16;$HEX_MAX_SIZE"|bc`                    #获取最大可设定值10进制
    #
    #if [ $DEC_HEXDUMP_SIZE -gt $DEC_MAX_SIZE ]; then                            #大于最大的内存保留分区大小则mount默认值
    #    MOUNT_SIZE=$HEX_MAX_SIZE
    #else
    #    MOUNT_SIZE=$HEXDUMP_SIZE
    #fi

    umount ${DEV_NODE}
    rmmod ak_ramdisk
    INSMOD="insmod /usr/modules/ak_ramdisk.ko disk_addr=0x${HEXDUMP_ADDR} disk_size=0x${HEXDUMP_SIZE}"
    echo $INSMOD
    $INSMOD
    mkdir -p ${DIR_RESERVE}
    if [ "$FS_TYPE" == "vfat" ]; then
        mkfs.vfat /dev/reserved_ram0
        mount -t vfat ${DEV_NODE} ${DIR_RESERVE}
    else
        mount -t $FS_TYPE ${DEV_NODE} ${DIR_RESERVE}
    fi
    df
    mount
}
mount_reserve_dir