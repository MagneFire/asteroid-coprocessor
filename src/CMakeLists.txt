set(SRC
	main.cpp
    backend.cpp
    display.cpp
    hands.cpp
    propertieschangedrelay.cpp
    settingsstore.cpp
    login1.cpp
    backend_casio.cpp
    backend_narwhal.cpp
)
set(HEADERS
    backend.h
    display.h
    hands.h
    propertieschangedrelay.h
    settingsstore.h
    settingskeys.h
    login1.h
    backend_casio.h
    backend_narwhal.h
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

if(HYBRIS_FOUND)
	target_sources(asteroid-coprocd PRIVATE backend_mobvoi.cpp)
	target_compile_definitions(asteroid-coprocd PRIVATE HAVE_HYBRIS)
	target_link_libraries(asteroid-coprocd PRIVATE PkgConfig::HYBRIS)
endif()

if(GBINDER_FOUND)
	target_compile_definitions(asteroid-coprocd PRIVATE HAVE_GBINDER)
	target_link_libraries(asteroid-coprocd PRIVATE PkgConfig::GBINDER)
endif()

if(MLITE_FOUND)
	target_sources(asteroid-coprocd PRIVATE dconfsettingsstore.cpp)
	target_compile_definitions(asteroid-coprocd PRIVATE HAVE_MLITE)
	target_link_libraries(asteroid-coprocd PRIVATE PkgConfig::MLITE)
endif()

install(TARGETS asteroid-coprocd
	DESTINATION ${CMAKE_INSTALL_BINDIR})
