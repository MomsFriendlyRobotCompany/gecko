message(STATUS "Gecko Tools ----------------------")
list(APPEND tools
    # logserver/gecko-logserver.cpp
    # topic/geckotopic.cpp
    # bag/gecko-bag.cpp
    # core/geckocore.cpp
    version/version.cpp
    # version/host.cpp
    # version/gecko-uname.cpp
    ping-test/gecko-ping.cpp
    # ping-test/send.cpp
)

# find_package(Boost COMPONENTS system program_options REQUIRED)
# find_package(Boost COMPONENTS system REQUIRED)

foreach(src ${tools})
    get_filename_component(name ${src} NAME_WE)
    message(STATUS " -> ${name}")
    add_executable(${name} ${src})
    target_link_libraries(${name} gecko ${CMAKE_THREAD_LIBS_INIT})

    # install(TARGETS ${name} DESTINATION bin)
endforeach()

# install(TARGETS geckocore DESTINATION ${GECKO_BIN})
# install(TARGETS gecko-uname DESTINATION ${GECKO_BIN})
# install(TARGETS host DESTINATION ${GECKO_BIN})
# install(TARGETS gecko-ping DESTINATION ${GECKO_BIN})
