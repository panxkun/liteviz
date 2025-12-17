if(NOT TARGET depends::torch)
  # Get PyTorch cmake prefix path from Python
  execute_process(
    COMMAND python3 -c "import torch; print(torch.utils.cmake_prefix_path)"
    OUTPUT_VARIABLE Torch_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  
  find_package(Torch REQUIRED PATHS ${Torch_DIR})
  
  add_library(depends::torch INTERFACE IMPORTED GLOBAL)
  target_link_libraries(depends::torch INTERFACE ${TORCH_LIBRARIES})
  target_include_directories(depends::torch INTERFACE ${TORCH_INCLUDE_DIRS})
  
  # PyTorch requires old C++11 ABI
  target_compile_definitions(depends::torch INTERFACE _GLIBCXX_USE_CXX11_ABI=0)
endif()
