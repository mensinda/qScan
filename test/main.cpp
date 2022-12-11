#include <iostream>

#include "SaneBackend.hpp"
#include "qscan_log.hpp"

using namespace std;
using namespace qscan::lib;

int main(int argc, char *argv[]) {
    const logger_t log = logger();

    log->info("QScan backend test playground");

    SaneBackend backend = SaneBackend([](string x) { return SaneBackend::SaneAuthResult{x, x}; });

    vector<unique_ptr<SaneDevice>> devices = backend.find_devices();

    SaneDevice *dev = nullptr;
    if (argc > 0) {
        for (unique_ptr<SaneDevice> &d : devices) {
            if (d->getName() == string{argv[1]}) {
                dev = d.get();
            }
        }
    }

    if (!dev) {
        return 0;
    }

    dev->connect();

    return 0;
}
