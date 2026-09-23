configure_file(org.asteroid.Coprocessor.service.in org.asteroid.Coprocessor.service @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.asteroid.Coprocessor.service
	DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
