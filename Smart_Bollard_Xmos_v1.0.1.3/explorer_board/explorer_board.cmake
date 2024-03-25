#**********************
# Gather Sources
#**********************
file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c 
							  ${CMAKE_CURRENT_LIST_DIR}/src/*.xc 
							  ${CMAKE_CURRENT_LIST_DIR}/src/*.S)
#file(GLOB_RECURSE APP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.c)
#file(GLOB_RECURSE APP_SOURCES_XC ${CMAKE_CURRENT_LIST_DIR}/src/*.xc)
#file(GLOB_RECURSE APP_SOURCES_S ${CMAKE_CURRENT_LIST_DIR}/src/*.S)
#file(GLOB_RECURSE APP_SOURCES   src/*.c src/*.xc src/*.S)
#set(APP_INCLUDES src)
#file(GLOB_RECURSE LIB_XC_SOURCES  ${CMAKE_CURRENT_LIST_DIR}/src/*.xc)
#file(GLOB_RECURSE LIB_ASM_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.S )
set(APP_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/src)

#**********************
# Flags
#**********************
set(APP_COMPILER_FLAGS
    -Os
    -g
    -report
    -fxscope
    -lquadspi
    -lquadflash
    -mcmodel=large
    -Wno-xcore-fptrgroup
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
)
set(APP_COMPILE_DEFINITIONS
    DEBUG_PRINT_ENABLE=1
    PLATFORM_USES_TILE_0=1
    PLATFORM_USES_TILE_1=1
    XE_BASE_TILE=0
    XUD_CORE_CLOCK=600
)

set(APP_LINK_OPTIONS
    -lquadspi
    -report
    ${CMAKE_CURRENT_LIST_DIR}/XCORE-AI-EXPLORER.xn
    ${CMAKE_CURRENT_LIST_DIR}/src/config.xscope
)

set(APP_LINK_LIBRARIES
	io::lib_mipi
	io::uart
	#io::lib_gpio
	#io::lib_xassert
	#io::lib_logging
	#io::lib_uart
	#rtos::drivers::uart
    rtos::bsp_config::xcore_ai_explorer
)

#**********************
# Tile Targets
#**********************
set(TARGET_NAME tile0_example_freertos_explorer_board)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=0)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

set(TARGET_NAME tile1_example_freertos_explorer_board)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL)
target_sources(${TARGET_NAME} PUBLIC ${APP_SOURCES})
target_include_directories(${TARGET_NAME} PUBLIC ${APP_INCLUDES})
target_compile_definitions(${TARGET_NAME} PUBLIC ${APP_COMPILE_DEFINITIONS} THIS_XCORE_TILE=1)
target_compile_options(${TARGET_NAME} PRIVATE ${APP_COMPILER_FLAGS})
target_link_libraries(${TARGET_NAME} PUBLIC ${APP_LINK_LIBRARIES})
target_link_options(${TARGET_NAME} PRIVATE ${APP_LINK_OPTIONS})
unset(TARGET_NAME)

#**********************
# Merge binaries
#**********************
merge_binaries(example_freertos_explorer_board tile1_example_freertos_explorer_board tile0_example_freertos_explorer_board 0)

#**********************
# Create run and debug targets
#**********************
create_run_target(example_freertos_explorer_board)
create_debug_target(example_freertos_explorer_board)

#**********************
# Filesystem support targets
#**********************
if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Windows)
    add_custom_command(
        OUTPUT example_freertos_explorer_board_fat.fs
        COMMAND ${CMAKE_COMMAND} -E make_directory %temp%/fatmktmp/fs
        COMMAND ${CMAKE_COMMAND} -E copy demo.txt %temp%/fatmktmp/fs/demo.txt
        COMMAND fatfs_mkimage --input=%temp%/fatmktmp --output=example_freertos_explorer_board_fat.fs
        BYPRODUCTS %temp%/fatmktmp
        DEPENDS example_freertos_explorer_board
        COMMENT
            "Create filesystem"
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
        VERBATIM
    )
else()
    add_custom_command(
        OUTPUT example_freertos_explorer_board_fat.fs
        COMMAND bash -c "tmp_dir=$(mktemp -d) && fat_mnt_dir=$tmp_dir && mkdir -p $fat_mnt_dir && mkdir $fat_mnt_dir/fs && cp ./demo.txt $fat_mnt_dir/fs/demo.txt && fatfs_mkimage --input=$tmp_dir --output=example_freertos_explorer_board_fat.fs --image_size=15564800"
        DEPENDS example_freertos_explorer_board
        COMMENT
            "Create filesystem"
        WORKING_DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/filesystem_support
        VERBATIM
    )
endif()

add_custom_target(flash_fs_example_freertos_explorer_board
	#COMMAND xflash --spi-read-id 0x9f --spi-spec ${CMAKE_CURRENT_LIST_DIR}/WINBOND_W25Q128JV.txt --target XCORE-AI-EXPLORER --verbose
	#COMMAND xflash --spi-spec ${CMAKE_CURRENT_LIST_DIR}/WINBOND_W25Q128JV.txt --target EEZ-XE216-512-EVAL --verbose
	#COMMAND xflash --spi-spec ${CMAKE_CURRENT_LIST_DIR}/WINBOND_W25Q128JV.txt --quad-spi-clock 50MHz --factory example_freertos_explorer_board.xe --boot-partition-size 0x100000 --data ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_freertos_explorer_board_fat.fs --verbose
    #COMMAND xflash --spi-spec ${CMAKE_CURRENT_LIST_DIR}/WINBOND_W25Q128JV.txt --quad-spi-clock 50MHz --factory example_freertos_explorer_board.xe --boot-partition-size 0x100000 --data ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_freertos_explorer_board_fat.fs --verbose
	#COMMAND xflash --spi-read-id 9f --quad-spi-clock 50MHz --factory example_freertos_explorer_board.xe --boot-partition-size 0x100000 --data ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_freertos_explorer_board_fat.fs
    COMMAND xflash --quad-spi-clock 5MHz --factory example_freertos_explorer_board.xe --boot-partition-size 0x100000 --data ${CMAKE_CURRENT_LIST_DIR}/filesystem_support/example_freertos_explorer_board_fat.fs --verbose
	DEPENDS example_freertos_explorer_board_fat.fs
    COMMENT
        "Flash filesystem"
    VERBATIM
)
