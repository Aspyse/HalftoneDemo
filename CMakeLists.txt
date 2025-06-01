# CMakeList.txt : CMake project for CustomLayout, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("HalftoneDemo")

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
)

target_include_directories(HalftoneDemo
	PRIVATE
		"${CMAKE_SOURCE_DIR}/include/fastgltf"
		"${CMAKE_SOURCE_DIR}/include/imgui"
		"${CMAKE_SOURCE_DIR}/include/imgui/backends"
		"${CMAKE_SOURCE_DIR}/include/miniply"
		"${CMAKE_SOURCE_DIR}/Classes"
		"${CMAKE_SOURCE_DIR}/Shaders"
		"${CMAKE_SOURCE_DIR}/Shaders/RenderPasses"
		"${CMAKE_SOURCE_DIR}/Subsystems"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET HalftoneDemo PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
