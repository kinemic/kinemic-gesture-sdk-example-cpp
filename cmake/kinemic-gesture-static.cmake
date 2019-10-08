get_filename_component(NUGET_DIR "${CMAKE_BINARY_DIR}/packages" ABSOLUTE)
set(KINEMIC_SDK_ID Kinemic.Gesture.CPP)
if(UWP)
    set(KINEMIC_SDK_ID Kinemic.Gesture.UWP.CPP)
endif()

get_filename_component(KINEMIC_SDK_DIR "${NUGET_DIR}/${KINEMIC_SDK_ID}.${KINEMIC_SDK_VERSION}" ABSOLUTE)
# need to create these on configure time for include directories to work
file(MAKE_DIRECTORY ${KINEMIC_SDK_DIR}/build/native/include)

include(ExternalProject)
ExternalProject_Add(kinemic_sdk_nuget
    DOWNLOAD_COMMAND nuget install ${KINEMIC_SDK_ID} -PreRelease -Version ${KINEMIC_SDK_VERSION} -OutputDirectory ${NUGET_DIR}
    SOURCE_DIR ${KINEMIC_SDK_DIR}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")
add_library(Kinemic::gesture-static STATIC IMPORTED)
add_dependencies(Kinemic::gesture-static kinemic_sdk_nuget)

set(arch_subdir unknown)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(arch_subdir x64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    set(arch_subdir x86)
endif()

set_target_properties(Kinemic::gesture-static PROPERTIES IMPORTED_LOCATION_RELEASE ${KINEMIC_SDK_DIR}/build/native/lib/${arch_subdir}/Release/kinemic-gesture-static.lib)
set_target_properties(Kinemic::gesture-static PROPERTIES IMPORTED_LOCATION_DEBUG ${KINEMIC_SDK_DIR}/build/native/lib/${arch_subdir}/Debug/kinemic-gesture-static.lib)

target_include_directories(Kinemic::gesture-static INTERFACE ${KINEMIC_SDK_DIR}/build/native/include)

target_compile_definitions(Kinemic::gesture-static INTERFACE KINEMICSDK_STATIC_DEFINE KINEMIC_BUNDLED_BLE_MANAGER)

set(KINEMIC_SDK_MODEL_DIR ${KINEMIC_SDK_DIR}/content/models)
