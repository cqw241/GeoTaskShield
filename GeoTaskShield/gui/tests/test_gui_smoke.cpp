#include "gui/BatchResultsWidget.h"
#include "gui/MainWindow.h"

#include <QApplication>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
}

std::string writeTempCsv(const std::string& name, const std::string& content)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / name;
    std::ofstream file(path, std::ios::binary);
    file << content;
    return path.string();
}

} // namespace

int main(int argc, char** argv)
{
    qputenv("QT_QPA_PLATFORM", "offscreen");

    QApplication app(argc, argv);
    gts::MainWindow window;
    require(!window.windowTitle().isEmpty(),
            "MainWindow should expose a user-facing title.");

    window.runSimulation();
    require(window.hasSimulationResult(),
            "MainWindow should run a simulation from default parameters.");

    auto* batchWidget = window.findChild<gts::BatchResultsWidget*>();
    require(batchWidget != nullptr,
            "MainWindow should expose a Batch Results tab.");
    require(batchWidget->hasMarkdownActionsForTesting(),
            "BatchResultsWidget should expose Markdown preview and export entries.");
    require(batchWidget->hasFilteredCsvExportActionForTesting(),
            "BatchResultsWidget should expose a filtered CSV export entry.");

    const std::string csvPath = writeTempCsv(
        "gts_phase7_gui.csv",
        "scenario,workers,tasks,grid_size,k,epsilon,privacy,algorithm,"
        "completed_tasks,total_tasks,completion_rate,average_moving_distance,"
        "total_reward,average_privacy_loss,algorithm_runtime_ms,user_load_stddev,"
        "fairness_index,privacy_utility_ratio,timeout_rate\n"
        "low,1,1,10,1,1,Grid Privacy,Nearest Greedy,"
        "1,1,0.2,1,10,3,0.1,0,0.5,0.05,0\n"
        "high,1,1,10,1,1,Grid Privacy,Nearest Greedy,"
        "1,1,0.9,1,10,1,0.1,0,0.8,0.45,0\n");
    require(batchWidget->loadCsvFile(QString::fromStdString(csvPath)),
            "BatchResultsWidget should load a valid test CSV.");
    require(batchWidget->visibleRowCountForTesting() == 2,
            "BatchResultsWidget should show loaded rows.");
    batchWidget->sortByColumnForTesting("completionRate", Qt::AscendingOrder);
    require(batchWidget->firstScenarioForTesting() == "low",
            "BatchResultsWidget should sort numeric columns numerically.");
    require(batchWidget->chartBarCountForTesting() == 2,
            "BatchResultsWidget should expose chart data after loading.");
    const QString markdown = batchWidget->markdownReportForTesting();
    require(markdown.contains("# GeoTaskShield Batch Results Report"),
            "BatchResultsWidget should generate a Markdown report title.");
    require(markdown.contains("| high | 1 | 1 | Grid Privacy | Nearest Greedy | 90.00% |"),
            "BatchResultsWidget should include loaded rows in the Markdown report.");
    require(markdown.contains("Best privacy-utility ratio"),
            "BatchResultsWidget should summarize the generated Markdown report.");

    const std::string markdownPath = (
        std::filesystem::temp_directory_path() / "gts_phase9_gui_report.md").string();
    require(batchWidget->exportMarkdownForTesting(QString::fromStdString(markdownPath)),
            "BatchResultsWidget should export the generated Markdown report.");
    std::ifstream markdownFile(markdownPath, std::ios::binary);
    const std::string exportedMarkdown((std::istreambuf_iterator<char>(markdownFile)),
                                       std::istreambuf_iterator<char>());
    require(exportedMarkdown.find("# GeoTaskShield Batch Results Report") !=
                std::string::npos,
            "Exported Markdown report should match the generated report.");
    const QString filteredCsv = batchWidget->filteredCsvForTesting();
    require(filteredCsv.contains("scenario,workers,tasks,grid_size,k,epsilon"),
            "BatchResultsWidget should generate a filtered CSV header.");
    require(filteredCsv.contains("high,1,1,10,1,1,Grid Privacy,Nearest Greedy"),
            "BatchResultsWidget should include visible rows in the filtered CSV.");

    const std::string filteredCsvPath =
        (std::filesystem::temp_directory_path() / "gts_phase9_filtered_gui.csv").string();
    require(batchWidget->exportFilteredCsvForTesting(QString::fromStdString(filteredCsvPath)),
            "BatchResultsWidget should export the filtered CSV report.");
    std::ifstream filteredCsvFile(filteredCsvPath, std::ios::binary);
    const std::string exportedCsv((std::istreambuf_iterator<char>(filteredCsvFile)),
                                  std::istreambuf_iterator<char>());
    require(exportedCsv.find("scenario,workers,tasks,grid_size,k,epsilon") !=
                std::string::npos,
            "Exported filtered CSV should contain the batch CSV header.");

    return 0;
}
