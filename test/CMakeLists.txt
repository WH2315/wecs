macro(AddTest name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} PRIVATE wecs)
    add_test(NAME ${name} COMMAND $<TARGET_FILE:${name}>)
    target_include_directories(${name} PRIVATE ../)
endmacro(AddTest)

add_subdirectory(entity)