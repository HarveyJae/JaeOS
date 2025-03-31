# 添加子目录放最前面


add_subdirectory(start)
add_subdirectory(dev)
add_subdirectory(lib)
add_subdirectory(mmu)
add_subdirectory(trap)
add_subdirectory(lock)
add_subdirectory(cpu)
add_subdirectory(process)
add_subdirectory(signal)
add_library(__main__ OBJECT "")
target_sources(__main__ 
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${START_SOURCES}
	${DEV_SOURCES}
	${LIB_SOURCES}
	${MMU_SOURCES}
	${TRAP_SOURCES}
	${LOCK_SOURCES}
	${CPU_SOURCES}
	${PROCESS_SOURCES}
	${SIGNAL_SOURCES}
)
target_compile_options(__main__
	PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${CFLAGS}>
	"-O0"
)
target_compile_options(__main__
    PRIVATE
    $<$<COMPILE_LANGUAGE:ASM>:${ASMFLAGS}>
    "-O0"
)
target_include_directories(__main__
	PRIVATE
	${CMAKE_SOURCE_DIR}/include
)
