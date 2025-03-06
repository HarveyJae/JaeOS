cmake_minimum_required(VERSION 3.15)

# 项目名称和支持语言
project(JaeOS C ASM)

# 交叉编译配置
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER "riscv64-unknown-elf-gcc")
set(CMAKE_ASM_COMPILER "riscv64-unknown-elf-gcc")
set(CMAKE_C_STANDARD 11)

# 添加编译选项
add_compile_options(
    -march=rv64gc
    -mabi=lp64
    -mcmodel=medany
    -ffreestanding
    -Wall
    -Wextra
    -Wno-unused-parameter
)

# 链接器配置
set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/kernel/boot/linker/qemu/linker.ld")

# 添加链接选项
add_link_options(
    -T ${LINK_SCRIPT}
    -nostdlib
    -Wl,--build-id=none
)

# 递归添加子目录
add_subdirectory(kernel)
add_subdirectory(lib)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

add_executable(JaeOS.elf)
target_sources(JaeOS.elf PRIVATE
    $<TARGET_OBJECTS:kernel_boot>
    $<TARGET_OBJECTS:kernel_dev>
    $<TARGET_OBJECTS:kernel_hal>
    $<TARGET_OBJECTS:kernel_mmu>
    $<TARGET_OBJECTS:lib>
)

# 生成二进制镜像文件
add_custom_command(TARGET JaeOS.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary JaeOS.elf JaeOS.bin
    COMMAND "Generating raw binary image"
)