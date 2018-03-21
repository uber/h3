if(__check_alloca)
  return()
endif()
set(__check_alloca 1)

function(check_alloca var)
    if(DEFINED ${var})
        return()
    endif()

    try_compile(have_alloca
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/alloca_test
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/alloca_test.c)
    set(description "Checking for stack allocation function (alloca)")
    if(have_alloca)
        set(${var} ON PARENT_SCOPE)
        message(STATUS "${description} - Success")
    else()
        message(STATUS "${description} - Failed")
    endif()
endfunction()
