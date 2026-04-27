# CMakeList.txt: GeoTaskShield 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

set(GEOTASKSHIELD_CORE_SOURCES
    "simulation/DataGenerator.cpp"
    "simulation/SimulationEngine.cpp"
    "privacy/GridPrivacy.cpp"
    "privacy/KAnonymityPrivacy.cpp"
    "privacy/LaplaceNoisePrivacy.cpp"
    "privacy/PrivacyFactory.cpp"
    "assignment/NearestGreedyAlgorithm.cpp"
    "assignment/ScoreGreedyAlgorithm.cpp"
    "assignment/HungarianAlgorithm.cpp"
    "assignment/AssignmentAlgorithmFactory.cpp"
    "evaluation/MetricsCalculator.cpp"
    "data/CsvExporter.cpp"
)

add_library(GeoTaskShieldCore STATIC ${GEOTASKSHIELD_CORE_SOURCES})
target_include_directories(GeoTaskShieldCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(GeoTaskShieldCore PUBLIC cxx_std_20)

add_executable(GeoTaskShield "app/console/main.cpp")
target_link_libraries(GeoTaskShield PRIVATE GeoTaskShieldCore)
target_compile_features(GeoTaskShield PRIVATE cxx_std_20)

if (BUILD_TESTING)
    add_executable(GeoTaskShieldTests "tests/test_core.cpp")
    target_link_libraries(GeoTaskShieldTests PRIVATE GeoTaskShieldCore)
    target_compile_features(GeoTaskShieldTests PRIVATE cxx_std_20)
    add_test(NAME GeoTaskShieldCoreTests COMMAND GeoTaskShieldTests)
endif()
