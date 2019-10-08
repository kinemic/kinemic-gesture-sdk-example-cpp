cmake_minimum_required(VERSION 2.8)

project(kinemic-gesture-sdk-example-cpp)

if (WIN32)
  set(KINEMIC_SDK_VERSION 1.0.0)
  include(cmake/kinemic-gesture-static.cmake)
  set(additional_libraries Kinemic::gesture-static ws2_32 rpcrt4 iphlpapi)
  set(sources "")
else()
    set(KINEMIC_SDK_VERSION 1.0.0)
    find_package(kinemic ${KINEMIC_SDK_VERSION} REQUIRED)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(BLE REQUIRED libblepp)

    if(TARGET kinemic-gesture-static)
        set(additional_libraries ${BLE_LIBRARIES} kinemic-gesture-static)
    else()
        set(additional_libraries ${BLE_LIBRARIES} kinemicsdk)
    endif()
    set(sources "blemanagerblepp.cpp")
endif()

add_executable(${PROJECT_NAME} "main.cpp" ${sources})
target_link_libraries(${PROJECT_NAME} ${additional_libraries})
target_include_directories(${PROJECT_NAME} PUBLIC ${BLE_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PUBLIC ${BLE_CFLAGS_OTHER})

if (WIN32)
	target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32)
endif()
