# Get unix name for determining OS
UNAME=$(uname)

# Create an empty 15.56 MiB file (15564800)
time  dd if=/dev/zero of=example_freertos_explorer_board_fat.fs bs=4096 count=3800 

if [ "$UNAME" == "Linux" ] ; then
    MKFS_VFAT_PATH=/sbin
    sudo umount -q fat_mnt
elif [ "$UNAME" == "Darwin" ] ; then
    MKFS_VFAT_PATH=/usr/local/sbin
    hdiutil detach fat_mnt
fi

# Create an empty FAT filesystem in it
$MKFS_VFAT_PATH/mkfs.vfat -v -F12 -s1 -S4096 -n xcore_fs example_freertos_explorer_board_fat.fs
#$MKFS_VFAT_PATH/mkfs.vfat -v -F12 -s1 -S4096 -n xcore_fs example_freertos_explorer_board_fat.fs

mkdir -p fat_mnt

DEMO_TXT=demo.txt
XMOS_IMAGE=IMG_FL_1111_32092_20220803_111940_01.jpg
XMOS_CONFIG=config.ini

#DEMO_TXT=~/XMOS_PRGORAM_BACKUP/explorer_board/filesystem_support_seperate_EmQos/demo.txt
#XMOS_IMAGE=~/XMOS_PRGORAM_BACKUP/explorer_board/filesystem_support_seperate_EmQos/IMG_FL_1111_32092_20220803_111940_01.jpg
#XMOS_CONFIG=~/XMOS_PRGORAM_BACKUP/explorer_board/filesystem_support_seperate_EmQos/config.ini

#W25X101=~/XMOS_PRGORAM_BACKUP/explorer_board/filesystem_support_seperate_EmQos/W25X101.pdf
#W25X102=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X102.pdf
#W25X103=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X103.pdf
#W25X104=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X104.pdf
#W25X105=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X105.pdf
#W25X106=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X106.pdf
#W25X107=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X106.pdf
#W25X108=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X108.pdf
#W25X109=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X109.pdf
#W25X1010=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X1010.pdf
#W25X1011=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X1011.pdf
#W25X1012=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X1012.pdf
#W25X1013=~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/W25X1013.pdf


# Mount the filesystem
if [ "$UNAME" == "Linux" ] ; then
    sudo mount -o loop example_freertos_explorer_board_fat.fs fat_mnt
elif [ "$UNAME" == "Darwin" ] ; then
    hdiutil attach -imagekey diskimage-class=CRawDiskImage -mountpoint fat_mnt example_freertos_explorer_board_fat.fs
fi

# Copy files into filesystem
sudo mkdir fat_mnt/fs
sudo mkdir fat_mnt/wifi
sudo cp $DEMO_TXT fat_mnt/fs/demo.txt
sudo cp $XMOS_IMAGE fat_mnt/IMG_FL_1111_32092_20220803_111940_01.jpeg
sudo cp $XMOS_CONFIG fat_mnt/config.ini

#sudo cp $W25X101 fat_mnt/fs/W25X101.pdf
#sudo cp $W25X102 fat_mnt/fs/W25X102.pdf
#sudo cp $W25X103 fat_mnt/fs/W25X103.pdf
#sudo cp $W25X104 fat_mnt/fs/W25X104.pdf
#sudo cp $W25X105 fat_mnt/fs/W25X105.pdf
#sudo cp $W25X106 fat_mnt/fs/W25X106.pdf
#sudo cp $W25X107 fat_mnt/fs/W25X107.pdf
#sudo cp $W25X108 fat_mnt/fs/W25X108.pdf
#sudo cp $W25X109 fat_mnt/fs/W25X109.pdf
#sudo cp $W25X1010 fat_mnt/fs/W25X1010.pdf
#sudo cp $W25X1011 fat_mnt/fs/W25X1011.pdf
#sudo cp $W25X1012 fat_mnt/fs/W25X1012.pdf
#sudo cp $W25X1013 fat_mnt/fs/W25X1013.pdf

#sudo cp $XMOS_IMAGE fat_mnt/fs/xmos.jpg
#sudo cp $XMOS_IMAGE fat_mnt/wifi/xmos1.jpg
#sudo cp $XMOS_IMAGE fat_mnt/xmos2.jpg
#if [ ! -f networks.dat ]; then
#    ./wifi_profile.py
#fi
#sudo cp networks.dat fat_mnt/wifi

# Unmount the filesystem
if [ "$UNAME" == "Linux" ] ; then
    sudo umount fat_mnt
elif [ "$UNAME" == "Darwin" ] ; then
    hdiutil detach fat_mnt
fi

# Cleanup
sudo rm -rf fat_mnt
