set(SYSTEMD_USER_UNIT_DIR "${CMAKE_INSTALL_PREFIX}/lib/systemd/user" CACHE PATH "Directory for systemd user units")

configure_file(asteroid-coprocd.service.in asteroid-coprocd.service @ONLY)
configure_file(asteroid-coproc-sync-time.service.in asteroid-coproc-sync-time.service @ONLY)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/asteroid-coprocd.service
	${CMAKE_CURRENT_BINARY_DIR}/asteroid-coproc-sync-time.service
	asteroid-coproc-sync-time.timer
	DESTINATION ${SYSTEMD_USER_UNIT_DIR})
