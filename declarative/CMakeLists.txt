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

add_library(asteroidcoprocessorplugin MODULE ${SRC} ${HEADERS})

target_link_libraries(asteroidcoprocessorplugin
	Qt6::DBus
	Qt6::Qml
	Qt6::Quick)

install(TARGETS asteroidcoprocessorplugin
	DESTINATION ${INSTALL_QML_IMPORT_DIR}/org/asteroid/coprocessor)
install(FILES qmldir
	DESTINATION ${INSTALL_QML_IMPORT_DIR}/org/asteroid/coprocessor)
