set(SRC
	coprocessor_plugin.cpp
	coprocessor.cpp
	hands.cpp
)
set(HEADERS
	coprocessor_plugin.h
	coprocessor.h
	hands.h
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

add_library(asteroidcoprocessorplugin ${SRC} ${HEADERS})

target_link_libraries(asteroidcoprocessorplugin
	Qt5::DBus
	Qt5::Qml
	Qt5::Quick)

install(TARGETS asteroidcoprocessorplugin
	DESTINATION ${INSTALL_QML_IMPORT_DIR}/org/asteroid/coprocessor)
install(FILES qmldir
	DESTINATION ${INSTALL_QML_IMPORT_DIR}/org/asteroid/coprocessor)
