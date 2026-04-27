#include "gui/MainWindow.h"

#include <QApplication>

#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message)
{
    if (!condition) {
        throw std::runtime_error(message);
    }
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

    return 0;
}
