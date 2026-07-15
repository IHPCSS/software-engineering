add_executable(laplace main.cpp)
target_link_libraries(laplace PRIVATE laplace_lib)
install(TARGETS laplace RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
