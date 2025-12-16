if(NOT TARGET depends::cxxopts)
	include(FetchContent)
	FetchContent_Declare(
		depends-cxxopts
		GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
		GIT_TAG        v3.0.0
		GIT_SHALLOW    TRUE
	)
	FetchContent_GetProperties(depends-cxxopts)
	if(NOT depends-cxxopts_POPULATED)
		message(STATUS "Fetching cxxopts sources")
		FetchContent_Populate(depends-cxxopts)
		message(STATUS "Fetching cxxopts sources - done")
	endif()

	add_library(depends_cxxopts INTERFACE)
	target_include_directories(depends_cxxopts INTERFACE
		${depends-cxxopts_SOURCE_DIR}/include)

	add_library(depends::cxxopts ALIAS depends_cxxopts)
	set(depends-cxxopts-source-dir ${depends-cxxopts_SOURCE_DIR} CACHE INTERNAL "" FORCE)
	mark_as_advanced(depends-cxxopts-source-dir)
endif()

