set(SRC
	main.cpp
    backend.cpp
)
set(HEADERS
    backend.h
)
set(INCLUDES
    ../dbus/
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.asteroid.Coprocessor.Display.xml
    backend.h
    Coprocessor::Backend
    display_adaptor
)

qt_add_dbus_adaptor(SRC
    ../dbus/org.asteroid.Coprocessor.Hands.xml
    backend.h
    Coprocessor::Hands
    hands_adaptor
)

include_directories(${INCLUDES})

add_executable(asteroid-coprocd ${SRC} ${HEADERS})

target_link_libraries(asteroid-coprocd PRIVATE
	Qt6::DBus)

install(TARGETS asteroid-coprocd
	DESTINATION ${CMAKE_INSTALL_BINDIR})
