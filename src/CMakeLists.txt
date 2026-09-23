set(SRC
	main.cpp
    backend.cpp
    display.cpp
    hands.cpp
    propertieschangedrelay.cpp
)
set(HEADERS
    backend.h
    display.h
    hands.h
    propertieschangedrelay.h
)
set(INCLUDES
    ../dbus/
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.asteroid.Coprocessor.Display.xml
    display.h
    Coprocessor::Display
    display_adaptor
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.asteroid.Coprocessor.Hands.xml
    hands.h
    Coprocessor::Hands
    hands_adaptor
)

include_directories(${INCLUDES})

add_executable(asteroid-coprocd ${SRC} ${HEADERS})

target_link_libraries(asteroid-coprocd PRIVATE
	Qt6::DBus)

install(TARGETS asteroid-coprocd
	DESTINATION ${CMAKE_INSTALL_BINDIR})
