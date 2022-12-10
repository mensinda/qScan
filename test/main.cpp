#include <iostream>

#include "SaneBackend.hpp"
#include "qscan_log.hpp"

using namespace std;
using namespace qscan::lib;

int main() {
    const logger_t log = logger();

    log->info("QScan backend test playground");

    const SaneBackend backend = SaneBackend([](string x) {return SaneBackend::SaneAuthResult{x, x}; });
    return 0;
}
