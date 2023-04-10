cmake_minimum_required(VERSION 3.24)
project(SAOmega)

set(CMAKE_CXX_STANDARD 17)

set(PROJECT_NAME SAOmega)

################################################################################
# Source groups
################################################################################
set(Header_Files
        "includes.h"
        )
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
        )
source_group("Source Files" FILES ${Source_Files})

set(Source_Files__Helpers
        "Helpers/macros.h"
        "Helpers/singleton.h"
        Helpers/BlueprintCache.h
        Helpers/SafeQueue.hpp)
source_group("Source Files\\Helpers" FILES ${Source_Files__Helpers})

set(Source_Files__Plugin
        "Plugin/saomega.h"
        Plugin/spyglass.cpp Plugin/breeding.cpp Plugin/loot.cpp Plugin/experience.cpp Plugin/worldsave.cpp Plugin/saomega.cpp)
source_group("Source Files\\Plugin" FILES ${Source_Files__Plugin})

set(ALL_FILES
        ${Header_Files}
        ${Source_Files}
        ${Source_Files__Helpers}
        ${Source_Files__Plugin}
        )

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} SHARED ${ALL_FILES})

#use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
#set(ROOT_NAMESPACE SAOmega)

set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_GLOBAL_KEYWORD "Win32Proj"
        )
set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION         "TRUE"
        )
################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/ARK-Server-API/version/Core/Public;"
        "${CMAKE_CURRENT_SOURCE_DIR}/.;"
        "${CMAKE_CURRENT_SOURCE_DIR}/Plugin;"
        "${CMAKE_CURRENT_SOURCE_DIR}/Helpers"
        )

################################################################################
# Compile definitions
################################################################################
target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
        "DEBUG_PLUGIN"
        ">"
        "NDEBUG;"
        "ARKSERVERAPITEMPLATE_EXPORTS;"
        "_WINDOWS;"
        "_USRDLL;"
        "UNICODE;"
        "_UNICODE"
        )

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
            /Ot
            >
            /permissive-;
            /Oi;
            /sdl;
            /Gy;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Y-
            )
    target_link_options(${PROJECT_NAME} PRIVATE
            /DEBUG;
            /SUBSYSTEM:WINDOWS;
            /OPT:REF;
            /OPT:ICF;
            /INCREMENTAL:NO
            )
endif()

target_link_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/ARK-Server-API/out_lib"
        )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "postbuild.sh"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running Post Build"
)