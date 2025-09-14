if (MLN_WITH_CLANG_TIDY OR MLN_CLANG_TIDY_COMMAND)
    if(MLN_CLANG_TIDY_COMMAND)
        set(CLANG_TIDY_COMMAND MLN_CLANG_TIDY_COMMAND)
        message(STATUS "Using clang-tidy at ${CLANG_TIDY_COMMAND}")
    else()
        find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)
        if(NOT CLANG_TIDY_COMMAND)
            message(FATAL_ERROR "ENABLE_CLANG_TIDY is ON but clang-tidy is not found!")
        else()
            message(STATUS "Found clang-tidy at ${CLANG_TIDY_COMMAND}")
        endif()
    endif()
    # TODO: there are options which are only available on GCC(e.g. -Werror=maybe-uninitialized),
    # that's why we need to disable this `unknown-warning-option` here.
    # We could check if current compiler supports particular flag before enabling it.
    set(CLANG_TIDY_COMMAND "${CLANG_TIDY_COMMAND};--extra-arg=-Wno-unknown-warning-option;--extra-arg=-Wno-pragmas")
endif()
