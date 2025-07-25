cmake_minimum_required(VERSION VERSION 3.16.0)

project ("VolumetricCloud" C CXX)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

set (CMAKE_CXX_STANDARD 20)

set (CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(WARNING "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS TRUE)
endif()

if(MSVC)
    add_definitions (-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /GL")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

message(STATUS "Build type is ${CMAKE_BUILD_TYPE}")
if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    add_definitions(-D_DEBUG)
    message(STATUS "Running on debug mode")
    message(STATUS ${CMAKE_CXX_FLAGS_DEBUG})
else()
    add_definitions(-DNDEBUG)
    message(STATUS "Runnning on release mode")
    message(STATUS ${CMAKE_CXX_FLAGS_RELEASE})
endif()

if(LINUX)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

function(strip_warnings_as_errors target)
    get_target_property(opts ${target} COMPILE_OPTIONS)
    if (opts)
        list(REMOVE_ITEM opts -Werror /WX)
        set_target_properties(${target} PROPERTIES COMPILE_OPTIONS "${opts}")
    endif()
endfunction()

### OpenMP #############################################

find_package(OpenMP)
if (OpenMP_CXX_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    link_libraries(OpenMP::OpenMP_CXX)
endif()

if(OpenMP_CXX_FOUND)
    link_libraries(OpenMP::OpenMP_CXX)
endif()

### Vulkan ##############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/submodules/Vulkan-Headers")
include_directories("${CMAKE_SOURCE_DIR}/submodules/Vulkan-Headers/include")
add_subdirectory("${CMAKE_SOURCE_DIR}/submodules/Vulkan-Loader")

### glm ##################################################

add_definitions(-DGLM_FORCE_SILENT_WARNINGS)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-DGLM_FORCE_SWIZZLE)
add_definitions(-DGLM_FORCE_LEFT_HANDED)
add_subdirectory("${CMAKE_SOURCE_DIR}/engine/libs/glm/glm")
include_directories("${CMAKE_SOURCE_DIR}/engine/libs/glm/glm")

### Working directory

add_compile_definitions("ASSET_DIR=${CMAKE_SOURCE_DIR}/assets")

### LibConfig ############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/libs/libconfig")
include_directories("${CMAKE_SOURCE_DIR}/engine/libs/libconfig")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/engine/libs/libconfig/stb_image")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/engine/libs/libconfig/nlohmann")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/engine/libs/libconfig/tiny_gltf_loader")

## SDL ###################################################

add_subdirectory("${CMAKE_SOURCE_DIR}/submodules/SDL2")
include_directories("${CMAKE_SOURCE_DIR}/submodules/SDL2/include")

### Litehtml #############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/webview/litehtml")
include_directories("${CMAKE_SOURCE_DIR}/webview/litehtml/include")

### Imgui ###############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/libs/imgui")
include_directories("${CMAKE_SOURCE_DIR}/engine/libs/imgui")

### Implot #########################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/libs/implot")
include_directories("${CMAKE_SOURCE_DIR}/engine/libs/implot")

### Bedrock ##############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/bedrock")
include_directories("${CMAKE_SOURCE_DIR}/engine/bedrock")

### Entity system ########################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/entity_system")
include_directories("${CMAKE_SOURCE_DIR}/engine/entity_system")

### Asset system #########################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/asset_system")
include_directories("${CMAKE_SOURCE_DIR}/engine/asset_system")

### Importer #############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/importer")
include_directories("${CMAKE_SOURCE_DIR}/engine/importer")

### TimeSystem ###########################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/time_system")
include_directories("${CMAKE_SOURCE_DIR}/engine/time_system")

### JobSystem ############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/job_system")
include_directories("${CMAKE_SOURCE_DIR}/engine/job_system")

### Renderer #############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/engine/render_system")
include_directories("${CMAKE_SOURCE_DIR}/engine/render_system")

### Shared ###############################################

add_subdirectory("${CMAKE_SOURCE_DIR}/shared")
include_directories("${CMAKE_SOURCE_DIR}/shared")

### WebView ##############################################

#link_libraries(litehtml)
add_subdirectory("${CMAKE_SOURCE_DIR}/webview")
include_directories("${CMAKE_SOURCE_DIR}/webview")

##########################################################

add_subdirectory("${CMAKE_SOURCE_DIR}/executables/volumetric_sphere")

##########################################################
