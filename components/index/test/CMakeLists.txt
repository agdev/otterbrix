project(test_index)

add_definitions(-DDEV_MODE)

set(${PROJECT_NAME}_SOURCES
        single_field_index.cpp
        create_index.cpp
)

add_executable(${PROJECT_NAME} main.cpp ${${PROJECT_NAME}_SOURCES})

target_link_libraries(
        ${PROJECT_NAME} PRIVATE
        otterbrix::log
        otterbrix::document
        otterbrix::index
        otterbrix::test_generaty
        Boost::boost
        Catch2::Catch2
)

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
