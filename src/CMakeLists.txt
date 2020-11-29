add_executable(eb200_connector eb200_connector.cpp)
target_link_libraries(eb200_connector ${OWRX_CONNECTOR_LIBRARIES})
target_include_directories(eb200_connector PRIVATE ${OWRX_CONNECTOR_INCLUDE_DIRS})

install(TARGETS eb200_connector DESTINATION ${CMAKE_INSTALL_BINDIR})
