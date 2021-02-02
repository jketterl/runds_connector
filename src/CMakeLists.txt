add_executable(runds_connector runds_connector.cpp ammos_parser.cpp eb200_parser.cpp)
target_link_libraries(runds_connector ${OWRX_CONNECTOR_LIBRARIES})
target_include_directories(runds_connector PRIVATE ${OWRX_CONNECTOR_INCLUDE_DIRS})

install(TARGETS runds_connector DESTINATION ${CMAKE_INSTALL_BINDIR})
