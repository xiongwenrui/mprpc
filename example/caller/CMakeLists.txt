#set(SRC_LIST callUserService.cc ../user.pb.cc)
set(SRC_LIST callFriendService ../friend.pb.cc)
add_executable(consumer ${SRC_LIST})
target_link_libraries(consumer mprpc protobuf)