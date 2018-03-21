if(__check_vla)
  return()
endif()
set(__check_vla 1)

function(check_vla var)
    if(DEFINED ${var})
        return()
    endif()

    try_compile(have_vla
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/vla_test
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/vla_test.c)
    set(description "Checking for VLA support")
    if(have_vla)
        set(${var} ON PARENT_SCOPE)
        message(STATUS "${description} - Success")
    else()
        message(STATUS "${description} - Failed")
    endif()
endfunction()
