set(SRC
    main.cpp
)
set(HEADERS
)
set(INCLUDES
    ../dbus/
)

qt_add_dbus_interface(SRC
    ../dbus/org.aos.display.xml
    display_interface
)

qt_add_dbus_interface(SRC
    ../dbus/org.aos.hands.xml
    hands_interface
)

include_directories(${INCLUDES})

add_executable(asteroid-coproc ${SRC} ${HEADERS})

target_link_libraries(asteroid-coproc
    Qt6::DBus)
