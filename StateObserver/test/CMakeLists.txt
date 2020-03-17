find_package(jsonrpc REQUIRED)
  
   add_executable(STOBSThunderClient STOBSThunderClient.cpp)
  
   set_target_properties(STOBSThunderClient PROPERTIES
           CXX_STANDARD 11
           CXX_STANDARD_REQUIRED YES
           )
  
  target_link_libraries(STOBSThunderClient
      PRIVATE
      jsonrpc::jsonrpc
          )
 
  install(TARGETS STOBSThunderClient DESTINATION bin)

