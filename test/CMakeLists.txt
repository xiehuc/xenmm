add_definitions(-Wall)

add_executable(mm_test_mono
    mono.c
    util.c
    )
set_target_properties(mm_test_mono
    PROPERTIES
    COMPILE_FLAGS "-DLOW_CPU=0"
    )

add_executable(mm_test_random
    random.c
    util.c
    )
set_target_properties(mm_test_random
    PROPERTIES
    COMPILE_FLAGS "-DLOW_CPU=0"
    )

add_executable(mm_test_static
    static.c
    util.c
    )
set_target_properties(mm_test_static
    PROPERTIES
    COMPILE_FLAGS "-DLOW_CPU=1"
    )
add_executable(mm_test_client 
   mono-client.c
   ../src/mmstore.c
   ../src/type.c
   )
target_link_libraries(mm_test_client
   xenstore
   )
add_executable(mm_util_swap
    swap.c
    )

install(TARGETS 
    mm_test_mono 
    mm_test_random 
    mm_test_static
    mm_test_client
    mm_util_swap
    
    DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
