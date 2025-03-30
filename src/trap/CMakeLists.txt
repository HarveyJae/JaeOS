set(TRAP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ktrap_vector.S
    ${CMAKE_CURRENT_SOURCE_DIR}/trap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/trampoline.S
    ${CMAKE_CURRENT_SOURCE_DIR}/signal_trampoline.S
    PARENT_SCOPE
)