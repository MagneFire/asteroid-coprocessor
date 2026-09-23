add_executable(asteroid-coproc main.cpp)
target_include_directories(asteroid-coproc PRIVATE ${PROJECT_SOURCE_DIR}/dbus)
target_link_libraries(asteroid-coproc PRIVATE Qt6::DBus)

install(TARGETS asteroid-coproc
	DESTINATION ${CMAKE_INSTALL_BINDIR})
