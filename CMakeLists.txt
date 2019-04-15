cmake_minimum_required(VERSION 2.8)

project(kinemic-gesture-sdk-example-cpp)

if (WIN32)

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
endif()

set(sources "blemanagerblepp.cpp")

add_executable(${PROJECT_NAME} "main.cpp" ${sources})
target_link_libraries(${PROJECT_NAME} ${additional_libraries})
target_include_directories(${PROJECT_NAME} PUBLIC ${BLE_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PUBLIC ${BLE_CFLAGS_OTHER})
