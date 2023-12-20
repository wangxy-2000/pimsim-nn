
# project
cmake_minimum_required(VERSION 3.1)
#cmake_policy(SET CMP0091 NEW)
project(pimsim-nn LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 11 CACHE STRING "Use C++ 11")
set(CMAKE_VERBOSE_MAKEFILE ON)

# dependency
add_subdirectory(packages/fmt)
add_subdirectory(packages/systemc)
add_subdirectory(packages/json)
add_subdirectory(packages/zlib)
add_subdirectory(packages/filesystem)



add_library(pimsim-nn STATIC "" )
set_target_properties(pimsim-nn PROPERTIES OUTPUT_NAME "pimsim-nn")
target_include_directories(pimsim-nn PRIVATE
        src
        )


# header-only dependency
target_include_directories(pimsim-nn PUBLIC packages/header-only)
target_include_directories(pimsim-nn PUBLIC packages/header-only/zstr/src)

add_dependencies(pimsim-nn systemc zlibstatic nlohmann_json fmt ghc_filesystem)

target_link_libraries(pimsim-nn PUBLIC
        systemc
        zlibstatic
        nlohmann_json
        fmt
        ghc_filesystem
        )


target_sources(pimsim-nn PRIVATE
        src/chip/Chip.cpp
        src/comm/InitiatorSocket.cpp
        src/comm/TargetSocket.cpp
        src/core/BaseCoreModule.cpp
        src/core/component/Controller.cpp
        src/core/component/InstDecode.cpp
        src/core/component/InstFetch.cpp
        src/core/component/MatrixUnit.cpp
        src/core/component/RegFile.cpp
        src/core/component/ScalarUnit.cpp
        src/core/component/TransferUnit.cpp
        src/core/component/VectorUnit.cpp
        src/core/Core.cpp
        src/core/xbar/XbarArray.cpp
        src/memory/ExternalMemory.cpp
        src/memory/Memory.cpp
        src/memory/MemoryWrapper.cpp
        src/network/Network.cpp
        src/network/Switch.cpp
        src/network/SwitchSocket.cpp
        src/Simulator.cpp
        src/utils/ClockDomain.cpp
        src/utils/EnergyCounter.cpp
        src/utils/Timer.cpp
        src/isa/Instruction.cpp
        src/core/payloads/ExecInfo.cpp
        src/analysis/PerformanceCounter.cpp
        src/core/xbar/ArrayGroup.cpp
        src/core/component/ReorderBuffer.cpp 
        src/core/component/Dispatcher.cpp
        )


# entry point
add_executable(ChipTest "")
add_dependencies(ChipTest pimsim-nn)
target_include_directories(ChipTest PRIVATE
    src
)

target_link_libraries(ChipTest PRIVATE
        pimsim-nn
    )

#target_link_libraries(ChipTest PRIVATE
#    systemc
#    pimsim-nn
#    zlibstatic
#)

target_sources(ChipTest PRIVATE
    src/ChipTest.cpp
)

# compress
add_executable(zc "")
target_include_directories(zc PRIVATE packages/header-only/zstr/src)

target_link_libraries(zc PRIVATE zlibstatic)

target_sources(zc PRIVATE tools/zc.cpp)

