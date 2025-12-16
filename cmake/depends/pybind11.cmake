if(NOT TARGET depends::pybind11)
  include(FetchContent)

  FetchContent_Declare(
    depends-pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.12.0
    GIT_SHALLOW    TRUE
  )

  FetchContent_GetProperties(depends-pybind11)
  if(NOT depends-pybind11_POPULATED)
    message(STATUS "Fetching pybind11 sources")
    FetchContent_Populate(depends-pybind11)
    message(STATUS "Fetching pybind11 sources - done")
  endif()

  add_subdirectory(${depends-pybind11_SOURCE_DIR} ${depends-pybind11_BINARY_DIR} EXCLUDE_FROM_ALL)

  add_library(depends::pybind11 ALIAS pybind11::module)
endif()
