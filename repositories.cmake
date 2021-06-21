
option(use_https "Use https" OFF)

if (use_https)
    set(nanostack_libservice_repo       https://github.com/PelionIoT/nanostack-libservice.git)
else()
    set(nanostack_libservice_repo       git@github.com:PelionIoT/nanostack-libservice.git)
endif()

#Googletest for testing
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.10.x.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(nanostack_libservice_decl
    GIT_REPOSITORY      ${nanostack_libservice_repo}
    GIT_TAG             "cmake_refactor"
)

if (${CMAKE_PROJECT_NAME} EQUAL mbedTrace)
    FetchContent_MakeAvailable(nanostack_libservice_decl)
endif()
