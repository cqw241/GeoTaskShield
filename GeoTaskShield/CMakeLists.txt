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
    "agent/RuleBasedConfigParser.cpp"
    "agent/ReportGenerator.cpp"
    "agent/ExperimentAgent.cpp"
    "experiment/BatchExperiment.cpp"
    "experiment/BatchExperimentExporter.cpp"
    "experiment/BatchResultCsvLoader.cpp"
    "experiment/BatchResultModel.cpp"
)

if (MSVC)
    foreach(flag_var
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_MINSIZEREL)
        string(REGEX REPLACE "/W[0-4]" "" ${flag_var} "${${flag_var}}")
    endforeach()
endif()

function(geotaskshield_apply_common_options target_name)
    if (MSVC)
        target_compile_options(${target_name} PRIVATE /utf-8 /W4)
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_library(GeoTaskShieldCore STATIC ${GEOTASKSHIELD_CORE_SOURCES})
target_include_directories(GeoTaskShieldCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(GeoTaskShieldCore PUBLIC cxx_std_20)
geotaskshield_apply_common_options(GeoTaskShieldCore)

add_executable(GeoTaskShield "app/console/main.cpp")
target_link_libraries(GeoTaskShield PRIVATE GeoTaskShieldCore)
target_compile_features(GeoTaskShield PRIVATE cxx_std_20)
geotaskshield_apply_common_options(GeoTaskShield)

add_executable(GeoTaskShieldAgentDemo "app/agent_demo/main.cpp")
target_link_libraries(GeoTaskShieldAgentDemo PRIVATE GeoTaskShieldCore)
target_compile_features(GeoTaskShieldAgentDemo PRIVATE cxx_std_20)
geotaskshield_apply_common_options(GeoTaskShieldAgentDemo)

add_executable(GeoTaskShieldBatchDemo "app/batch_demo/main.cpp")
target_link_libraries(GeoTaskShieldBatchDemo PRIVATE GeoTaskShieldCore)
target_compile_features(GeoTaskShieldBatchDemo PRIVATE cxx_std_20)
geotaskshield_apply_common_options(GeoTaskShieldBatchDemo)

if (BUILD_TESTING)
    add_executable(GeoTaskShieldTests "tests/test_core.cpp")
    target_link_libraries(GeoTaskShieldTests PRIVATE GeoTaskShieldCore)
    target_compile_features(GeoTaskShieldTests PRIVATE cxx_std_20)
    geotaskshield_apply_common_options(GeoTaskShieldTests)
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
        "gui/BatchResultsWidget.cpp"
        "gui/MainWindow.cpp"
        "gui/MapCanvas.cpp"
        "gui/MetricBarChart.cpp"
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
    geotaskshield_apply_common_options(GeoTaskShieldGui)
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
        geotaskshield_apply_common_options(GeoTaskShieldGuiSmokeTests)
        add_test(NAME GeoTaskShieldGuiSmokeTests COMMAND GeoTaskShieldGuiSmokeTests)
        set_tests_properties(GeoTaskShieldGuiSmokeTests PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${GEOTASKSHIELD_QT_PLUGIN_DIR}"
            ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>")
    endif()
endif()

install(TARGETS GeoTaskShield GeoTaskShieldAgentDemo GeoTaskShieldBatchDemo
    RUNTIME DESTINATION bin)

if (GEOTASKSHIELD_BUILD_GUI)
    install(TARGETS GeoTaskShieldGui RUNTIME DESTINATION bin)
endif()

install(FILES
    "${CMAKE_SOURCE_DIR}/README.md"
    "${CMAKE_SOURCE_DIR}/HANDOFF.md"
    "${CMAKE_SOURCE_DIR}/CHANGELOG.md"
    "${CMAKE_SOURCE_DIR}/phase2_results.csv"
    "${CMAKE_SOURCE_DIR}/phase5_batch_results.csv"
    "${CMAKE_SOURCE_DIR}/phase5_batch_report.md"
    DESTINATION share/GeoTaskShield)
