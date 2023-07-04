add_executable(runds_connector runds_connector.cpp protocol.cpp ammos_protocol.cpp eb200_protocol.cpp)
target_link_libraries(runds_connector OwrxConnector::owrx-connector)

install(TARGETS runds_connector DESTINATION ${CMAKE_INSTALL_BINDIR})
