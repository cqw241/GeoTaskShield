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

option(GEOTASKSHIELD_BUILD_GUI "Build the Qt Widgets GUI" OFF)

if (GEOTASKSHIELD_BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
    get_filename_component(GEOTASKSHIELD_QT_PREFIX "${Qt6_DIR}/../../.." ABSOLUTE)
    set(GEOTASKSHIELD_QT_PLUGIN_DIR "${GEOTASKSHIELD_QT_PREFIX}/plugins")
    find_program(GEOTASKSHIELD_WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt
        HINTS "${GEOTASKSHIELD_QT_PREFIX}/bin")

    set(GEOTASKSHIELD_GUI_SOURCES
        "gui/MainWindow.cpp"
        "gui/MapCanvas.cpp"
        "gui/ParameterPanel.cpp"
        "gui/ResultPanel.cpp"
        "gui/LogPanel.cpp"
    )

    add_executable(GeoTaskShieldGui WIN32
        "gui/app/main.cpp"
        ${GEOTASKSHIELD_GUI_SOURCES}
    )
    target_link_libraries(GeoTaskShieldGui PRIVATE GeoTaskShieldCore Qt6::Widgets)
    target_compile_features(GeoTaskShieldGui PRIVATE cxx_std_20)
    if (WIN32 AND GEOTASKSHIELD_WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET GeoTaskShieldGui POST_BUILD
            COMMAND "${GEOTASKSHIELD_WINDEPLOYQT_EXECUTABLE}"
                    --no-translations
                    --no-compiler-runtime
                    "$<TARGET_FILE:GeoTaskShieldGui>"
            COMMENT "Deploying Qt runtime for GeoTaskShieldGui")
    endif()

    if (BUILD_TESTING)
        add_executable(GeoTaskShieldGuiSmokeTests
            "gui/tests/test_gui_smoke.cpp"
            ${GEOTASKSHIELD_GUI_SOURCES}
        )
        target_link_libraries(GeoTaskShieldGuiSmokeTests PRIVATE GeoTaskShieldCore Qt6::Widgets)
        target_compile_features(GeoTaskShieldGuiSmokeTests PRIVATE cxx_std_20)
        add_test(NAME GeoTaskShieldGuiSmokeTests COMMAND GeoTaskShieldGuiSmokeTests)
        set_tests_properties(GeoTaskShieldGuiSmokeTests PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${GEOTASKSHIELD_QT_PLUGIN_DIR}"
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
    endif()
endif()
