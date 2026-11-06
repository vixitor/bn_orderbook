# Presets in this project use schema version 3, supported by CMake 3.21+.
cmake_minimum_required(VERSION 3.21)
# Name the project and enable C++ compilation; no C compiler is required.
project(cpp_bn_orderbook LANGUAGES CXX)

# Use C++20, require compiler support, and avoid GNU-specific language extensions.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Generate build/compile_commands.json with the actual flags for each source file.
# clangd uses it to discover the language standard, SDK, and dependency headers.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# On macOS, find dependencies installed by Homebrew. Query its stable prefix
# instead of hard-coding versioned paths under Cellar that change after upgrades.
if(APPLE)
  # Apple Silicon normally uses /opt/homebrew; Intel Macs use /usr/local.
  find_program(HOMEBREW_EXECUTABLE brew HINTS /opt/homebrew/bin /usr/local/bin)
  if(HOMEBREW_EXECUTABLE)
    execute_process(
      COMMAND "${HOMEBREW_EXECUTABLE}" --prefix
      OUTPUT_VARIABLE HOMEBREW_PREFIX
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE HOMEBREW_RESULT
    )
    if(HOMEBREW_RESULT EQUAL 0)
      # OpenSSL may be installed without links into the main Homebrew prefix.
      list(APPEND CMAKE_PREFIX_PATH
        "${HOMEBREW_PREFIX}"
        "${HOMEBREW_PREFIX}/opt/openssl@3"
      )
    endif()
  endif()
endif()

# REQUIRED stops configuration if a dependency cannot be found.
# CONFIG asks Boost to supply its own package configuration and imported targets.
find_package(Boost 1.75 CONFIG REQUIRED COMPONENTS json)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Compile main.cpp into the cpp_bn_orderbook executable.
add_executable(cpp_bn_orderbook src/main.cpp src/snapshot.cpp src/update.cpp src/orderbook.cpp)

target_include_directories(cpp_bn_orderbook PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
# Imported targets provide include paths and required link settings.
# PRIVATE applies these dependencies to this executable, not to its consumers.
target_link_libraries(cpp_bn_orderbook PRIVATE
  Boost::headers       # Boost.Asio and Beast headers used by this program.
  OpenSSL::SSL         # TLS protocol support for the encrypted connection.
  OpenSSL::Crypto      # Cryptographic primitives used by OpenSSL.
  Threads::Threads     # Platform-specific thread support, when required.
  Boost::json
)

# Compiler warnings apply to our target. They are separate from clang-tidy checks.
# Exceptions remain enabled: synchronous Boost calls report failures by throwing.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(cpp_bn_orderbook PRIVATE
    # Keep source locations, variables, and call stacks available to the debugger.
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    $<$<CONFIG:Debug>:-fno-optimize-sibling-calls>
    -Wall        # A common set of warnings; this does not mean every warning.
    -Wextra      # Additional useful warnings beyond -Wall.
    -Wpedantic   # Diagnose certain departures from standard C++.
    -Wconversion # Warn about implicit conversions that may change a value.
    -Wshadow     # Warn when a declaration hides another declaration.
  )

  # General runtime diagnostics for memory errors and undefined behavior.
  # Both compilation and linking need the sanitizer flags. Release is unaffected.
  option(ENABLE_SANITIZERS "Enable ASan and UBSan in Debug builds" ON)
  if(ENABLE_SANITIZERS)
    target_compile_options(cpp_bn_orderbook PRIVATE
      $<$<CONFIG:Debug>:-fsanitize=address,undefined>
      $<$<CONFIG:Debug>:-fno-sanitize-recover=all>
    )
    target_link_options(cpp_bn_orderbook PRIVATE
      $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
  endif()
elseif(MSVC)
  target_compile_options(cpp_bn_orderbook PRIVATE /W4 /permissive-)
endif()

# macOS runtime symbolizers use the dSYM bundle to resolve source file/line data.
if(APPLE)
  find_program(DSYMUTIL_EXECUTABLE dsymutil REQUIRED)
  add_custom_command(TARGET cpp_bn_orderbook POST_BUILD
    COMMAND "${DSYMUTIL_EXECUTABLE}" "$<TARGET_FILE:cpp_bn_orderbook>"
    COMMENT "Generating dSYM debug symbols"
    VERBATIM
  )
endif()
