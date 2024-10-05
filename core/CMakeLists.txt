project(core)

find_package(OpenSSL REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(monix
    main.cpp

    AsyncPatchProtection.cpp
    Command.cpp
    CoreCommand.cpp
    CoreConsole.cpp
    CoreLogger.cpp
    CoreModule.cpp
    CoreSystem.cpp
    ProtectedRegion.cpp
    Settings.cpp
    Signal.cpp

    commands/ExitCommand.cpp
    commands/HelpCommand.cpp
    commands/LoadCommand.cpp
    commands/ShutdownCommand.cpp
    commands/UnameCommand.cpp
)

set_property(TARGET monix PROPERTY CXX_STANDARD 23)
target_compile_options(monix PRIVATE
    -Wall -Wextra -Wpedantic -Werror
)
target_link_options(monix PRIVATE
    -rdynamic
)
target_link_libraries(monix PRIVATE OpenSSL::Crypto)

if(MONIX_USE_SANITIZERS)
    target_compile_options(monix PRIVATE
        -fsanitize=address -fsanitize=leak -fsanitize=undefined
    )
    target_link_options(monix PRIVATE
        -fsanitize=address -fsanitize=leak -fsanitize=undefined
    )
endif()

install(TARGETS monix DESTINATION bin)
