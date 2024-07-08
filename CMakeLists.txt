cmake_minimum_required(VERSION 3.5)

set(PICO_SDK_FETCH_FROM_GIT off)
set(PICO_BOARD=pico)
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk)

include(pico_sdk_import.cmake)
set(CMAKE_C_FLAGS "-Ofast -Wall -mcpu=cortex-m0plus -mtune=cortex-m0plus -funroll-loops")

set(PICO_COPY_TO_RAM 1)

project(deskhop_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(PICO_PIO_USB_DIR ${CMAKE_CURRENT_LIST_DIR}/Pico-PIO-USB)

add_library(Pico-PIO-USB STATIC
    ${PICO_PIO_USB_DIR}/src/pio_usb.c
    ${PICO_PIO_USB_DIR}/src/pio_usb_host.c
    ${PICO_PIO_USB_DIR}/src/usb_crc.c
)
pico_generate_pio_header(Pico-PIO-USB ${PICO_PIO_USB_DIR}/src/usb_tx.pio)
pico_generate_pio_header(Pico-PIO-USB ${PICO_PIO_USB_DIR}/src/usb_rx.pio)

target_link_libraries(Pico-PIO-USB PRIVATE
  pico_stdlib
  pico_multicore
  hardware_pio
  hardware_dma
)
target_include_directories(Pico-PIO-USB PRIVATE ${PICO_PIO_USB_DIR})

set(COMMON_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/usb_descriptors.c
        ${CMAKE_CURRENT_LIST_DIR}/src/defaults.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hid_parser.c
        ${CMAKE_CURRENT_LIST_DIR}/src/utils.c
        ${CMAKE_CURRENT_LIST_DIR}/src/handlers.c
        ${CMAKE_CURRENT_LIST_DIR}/src/setup.c
        ${CMAKE_CURRENT_LIST_DIR}/src/keyboard.c
        ${CMAKE_CURRENT_LIST_DIR}/src/mouse.c
        ${CMAKE_CURRENT_LIST_DIR}/src/screens.c
        ${CMAKE_CURRENT_LIST_DIR}/src/led.c
        ${CMAKE_CURRENT_LIST_DIR}/src/uart.c
        ${CMAKE_CURRENT_LIST_DIR}/src/host_msg.c
        ${CMAKE_CURRENT_LIST_DIR}/src/uart_otg.c
        ${CMAKE_CURRENT_LIST_DIR}/src/usb.c
        ${CMAKE_CURRENT_LIST_DIR}/src/main.c
        ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/dcd_pio_usb.c
        ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c
)

set(COMMON_INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${PICO_PIO_USB_DIR}/src
)

set(COMMON_LINK_LIBRARIES
  pico_stdlib
  hardware_flash
  hardware_uart
  hardware_gpio
  hardware_pio

  tinyusb_device 
  tinyusb_host
  pico_multicore
  Pico-PIO-USB
)

# Pico A - Keyboard (board_role = 0) 
#      B - Mouse    (board_role = 1)

# Common settings
set(binaries board_A board_B board_B_boot3_nohdr)

foreach(idx RANGE 0 2)
  list (GET binaries ${idx} binary)
  if(idx EQUAL 0)
    set(board_role 0)
  else()
    set(board_role 1)
  endif()

  add_executable(${binary})

  target_sources(${binary} PUBLIC ${COMMON_SOURCES})
  target_compile_definitions(${binary} PRIVATE BOARD_ROLE=${board_role} PIO_USB_USE_TINYUSB=1 PIO_USB_DP_PIN_DEFAULT=14)

  target_include_directories(${binary} PUBLIC ${COMMON_INCLUDES})

  pico_enable_stdio_usb(${binary} 0)
  pico_add_extra_outputs(${binary})

  target_link_options(${binary} PRIVATE
    -Xlinker
    --print-memory-usage
  )


endforeach()

# Binary of A and B firmware with stock bootloader
target_link_libraries(board_A PUBLIC ${COMMON_LINK_LIBRARIES})
pico_set_linker_script(board_A ${CMAKE_SOURCE_DIR}/memory_map.ld)
target_link_libraries(board_B PUBLIC ${COMMON_LINK_LIBRARIES})
pico_set_linker_script(board_B ${CMAKE_SOURCE_DIR}/memory_map.ld)

add_subdirectory("rp2040-serial-bootloader")
target_compile_definitions(bootloader PRIVATE
  BOOTLOADER_ENTRY_PIN=0
  UART_TX_PIN=16
  UART_RX_PIN=17
  )

# Assemble UART bootloader into elf object library
# Providing .boot3 sections
set(bt_bin_dir $<TARGET_FILE_DIR:bootloader>)
add_library(boot3 OBJECT ${CMAKE_CURRENT_LIST_DIR}/boot3.s)
target_include_directories(boot3 PRIVATE ${bt_bin_dir})
add_dependencies(boot3 bootloader)

# Link .boot3 sections using boot3 memory map
# The CRC field in result is not populated yet
target_link_libraries(board_B_boot3_nohdr PUBLIC ${COMMON_LINK_LIBRARIES} boot3)
pico_set_linker_script(board_B_boot3_nohdr ${CMAKE_SOURCE_DIR}/memory_map_boot3.ld)

# Build app image for B
# App image only contains the main app after boot3 section
set(app_img ${CMAKE_CURRENT_BINARY_DIR}/board_B_app_img.bin)
add_custom_target(board_B_app_img ALL DEPENDS ${app_img})
add_custom_command(OUTPUT ${app_img}
  DEPENDS board_B_boot3_nohdr
  COMMAND ${CMAKE_OBJCOPY} -R .boot3 -R .img_offset -R .img_size -R .img_crc 
  -Obinary $<TARGET_FILE:board_B_boot3_nohdr> ${app_img}
)

find_package (Python3 REQUIRED COMPONENTS Interpreter)
# Calculate CRC checksum for app image
set(crc_le ${CMAKE_CURRENT_BINARY_DIR}/board_B_app_img.crc.le.bin)
set(crc_script ${CMAKE_CURRENT_LIST_DIR}/calc_crc.py)
add_custom_command(OUTPUT ${crc_le}
  DEPENDS ${app_img} ${crc_script}
  COMMAND ${Python3_EXECUTABLE} ${crc_script} ${app_img} ${crc_le}
)

set(boot3_target ${CMAKE_CURRENT_BINARY_DIR}/board_B_boot3)
add_custom_target(board_B_boot3 ALL DEPENDS ${boot3_target}.elf)
add_custom_command(OUTPUT ${boot3_target}.elf
  DEPENDS board_B_boot3_nohdr ${crc_le}
  COMMAND ${CMAKE_OBJCOPY} --update-section .img_crc=${crc_le}
    $<TARGET_FILE:board_B_boot3_nohdr> ${boot3_target}.elf
)

add_custom_command(TARGET board_B_boot3 POST_BUILD
  BYPRODUCTS ${boot3_target}.bin
  COMMAND ${CMAKE_OBJCOPY} -Obinary ${boot3_target}.elf ${boot3_target}.bin
)

add_custom_command(TARGET board_B_boot3 POST_BUILD
  BYPRODUCTS ${boot3_target}.uf2
  COMMAND ELF2UF2 ${boot3_target}.elf ${boot3_target}.uf2
)

