set(SRC
	main.cpp
    backend.cpp
    backend_catfish.cpp
)
set(HEADERS
    backend.h
    backend_catfish.h
)
set(INCLUDES
    ../dbus/
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.aos.display.xml
    backend.h
    AsteroidOS::CoprocessorDaemon::Backend
    display_adaptor
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.aos.hands.xml
    backend.h
    AsteroidOS::CoprocessorDaemon::Hands
    hands_adaptor
)

include_directories(${INCLUDES})

add_executable(asteroid-coprocd ${SRC} ${HEADERS})

target_link_libraries(asteroid-coprocd PRIVATE
	Qt6::DBus)

install(TARGETS asteroid-coprocd
	DESTINATION ${CMAKE_INSTALL_BINDIR})
