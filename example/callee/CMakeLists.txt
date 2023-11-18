set(SRC_LIST userService.cc friendService.cc ../user.pb.cc ../friend.pb.cc)
add_executable(provider ${SRC_LIST})
target_link_libraries(provider mprpc protobuf)