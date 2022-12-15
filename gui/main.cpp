#include "config/QScanConfig.hpp"
#include "widgets/MainWindow.hpp"
#include "SaneException.hpp"
#include "qscan_log.hpp"

#include <QApplication>

using namespace std;
using namespace qscan::gui;
using namespace qscan::lib;

int main(int argc, char *argv[]) {
    logger()->set_level(spdlog::level::debug);

    QApplication a(argc, argv);

    QScanConfig cfg{};
    SaneBackend backend{[](const string &) { return SaneBackend::SaneAuthResult{}; }};

    MainWindow w(nullptr, backend, cfg);
    w.show();
    return a.exec();
}
