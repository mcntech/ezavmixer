set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-access-control")

add_executable(server_test server_test.cpp)
target_link_libraries(server_test ${OPENSSL_CRYPTO_LIBRARY})
add_test(server_test server_test)

