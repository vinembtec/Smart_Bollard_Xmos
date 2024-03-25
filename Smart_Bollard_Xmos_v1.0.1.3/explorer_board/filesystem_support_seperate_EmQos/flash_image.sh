rm -rf example_freertos_explorer_board_fat.fs

if [ ! -f example_freertos_explorer_board_fat.fs ]; then
    bash ./create_fs.sh
fi

##xflash --quad-spi-clock 50MHz --factory ../bin/rtos_drivers_wifi.xe --boot-partition-size 0x100000 --data ./fat.fs
##xflash --quad-spi-clock 50MHz --factory ~/xcore_sdk/build/examples/freertos/explorer_board/example_freertos_explorer_board.xe --boot-partition-size 0x100000 --data ~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/example_freertos_explorer_board_fat.fs --verbose

##xflash --quad-spi-clock 41.66MHz --factory ~/xcore_sdk/build/example_freertos_explorer_board.xe --boot-partition-size 0x100000 --data ~/xcore_sdk/examples/freertos/explorer_board/filesystem_support_seperate/example_freertos_explorer_board_fat.fs --verbose

xflash --quad-spi-clock 50MHz --factory example_freertos_explorer_board.xe --boot-partition-size 0x100000 --data example_freertos_explorer_board_fat.fs --verbose
