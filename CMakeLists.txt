# CMakeList.txt : CMake project for CustomLayout, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.14)
set(CMAKE_CXX_STANDARD 20)
add_definitions(-DNOMINMAX)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("HalftoneDemo")


# FetchContent for fastgltf
include(FetchContent)

FetchContent_Declare(
  fastgltf
  GIT_REPOSITORY https://github.com/spnda/fastgltf.git
  GIT_TAG        main
)

FetchContent_MakeAvailable(fastgltf)

# Add source to this project's executable.
add_executable (HalftoneDemo
	"main.cpp"
	"engine.cpp"
	"Classes/camera.cpp"
	"Classes/model.cpp"
	"Shaders/RenderPasses/crosshatch_pass.cpp"
	"Shaders/RenderPasses/geometry_pass.cpp"
	"Shaders/RenderPasses/halftone_pass.cpp"
	"Shaders/RenderPasses/lighting_pass.cpp"
	"Shaders/RenderPasses/sobel_pass.cpp"
	"Subsystems/gui_system.cpp"
	"Subsystems/input_system.cpp"
	"Subsystems/render_system.cpp"

	"include/imgui/imgui.cpp"
	"include/imgui/imgui_demo.cpp"
	"include/imgui/imgui_draw.cpp"
	"include/imgui/imgui_tables.cpp"
	"include/imgui/imgui_widgets.cpp"
	"include/imgui/backends/imgui_impl_dx11.cpp"
	"include/imgui/backends/imgui_impl_win32.cpp"
	"include/miniply/miniply.cpp"
	"Shaders/RenderPasses/pioaru_pass.cpp")

target_include_directories(HalftoneDemo
	PRIVATE
		"${CMAKE_SOURCE_DIR}/include"
		"${CMAKE_SOURCE_DIR}/include/imgui"
		"${CMAKE_SOURCE_DIR}/include/imgui/backends"
		"${CMAKE_SOURCE_DIR}/include/miniply"
		"${CMAKE_SOURCE_DIR}/Classes"
		"${CMAKE_SOURCE_DIR}/Shaders"
		"${CMAKE_SOURCE_DIR}/Shaders/RenderPasses"
		"${CMAKE_SOURCE_DIR}/Subsystems"
)

target_link_libraries(HalftoneDemo
	PRIVATE
		d3d11
		fastgltf::fastgltf
)

file(GLOB_RECURSE ALL_RESOURCES
    "${CMAKE_SOURCE_DIR}/Shaders/*.vs"
	"${CMAKE_SOURCE_DIR}/Shaders/*.ps"
	"${CMAKE_SOURCE_DIR}/Models/*.ply"
	"${CMAKE_SOURCE_DIR}/Models/*.glb"
)

foreach(RESOURCE_FILE ${ALL_RESOURCES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_SOURCE_DIR}" "${RESOURCE_FILE}")

    set(DEST_PATH "${CMAKE_BINARY_DIR}/${REL_PATH}")

    get_filename_component(DEST_DIR "${DEST_PATH}" DIRECTORY)
    file(MAKE_DIRECTORY "${DEST_DIR}")

    configure_file(
        "${RESOURCE_FILE}"
        "${DEST_PATH}"
        COPYONLY
    )
endforeach()

target_compile_definitions(HalftoneDemo
  PRIVATE
    UNICODE
    _UNICODE
)

set_target_properties(HalftoneDemo PROPERTIES
    WIN32_EXECUTABLE TRUE
)


# TODO: Add tests and install targets if needed.
