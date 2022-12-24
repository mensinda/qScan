#include "config/QScanConfig.hpp"
#include "widgets/MainWindow.hpp"
#include "SaneException.hpp"
#include "qscan_log.hpp"

#include <Magick++.h>

#include <QApplication>

using namespace std;
using namespace qscan::gui;
using namespace qscan::lib;
using namespace Magick;

int main(int argc, char *argv[]) {
    logger()->set_level(spdlog::level::debug);

    InitializeMagick(*argv);

    QApplication a(argc, argv);

    QScanConfig cfg{};
    SaneBackend backend{[](const string &) { return SaneBackend::SaneAuthResult{}; }};

    MainWindow w(nullptr, backend, cfg);
    w.show();
    return a.exec();
}
