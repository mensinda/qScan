#include "SaneDevice.hpp"
#include "util/ValueResetter.hpp"
#include "SaneException.hpp"
#include "SaneImage.hpp"
#include "enum2str.hpp"
#include "qscan_log.hpp"

#include <fstream>

namespace qscan::lib {

SaneDevice::SaneDevice(const SANE_Device *device)
    : name(device->name), vendor(device->vendor), model(device->model), type(device->type), options(&rawOptions) {}

SaneDevice::~SaneDevice() {
    if (handle) {
        logger()->info("[Sane] Closing device {}", name);
        sane_cancel(handle);
        sane_close(handle);
    }
}

void SaneDevice::connect() {
    logger_t log = logger();

    log->info("[Sane] Connecting to {}", name);

    SANE_Status status = sane_open(name.c_str(), &handle);
    if (status != SANE_STATUS_GOOD) {
        handle = nullptr;
        throw SaneException(status, "Failed to connect to " + name);
    }

    // Always do blocking IO
    sane_set_io_mode(handle, SANE_FALSE);
    sane_cancel(handle);

    reload_options();

    log->info("[Sane] Connected to {}", name);
}

void SaneDevice::reload_options() {
    if (disableOptionReload) {
        return;
    }

    logger_t log = logger();

    // Reload options on subsequent runs
    if (!rawOptions.empty()) {
        log->info("[Sane] Reloading {} options:", rawOptions.size());
        for (SaneOption &opt : rawOptions) {
            opt.reload();
        }

        options.refreshFilter();
        return;
    }

    // Construct the options the first time
    SANE_Int    nopts  = 0;
    SANE_Status status = sane_control_option(handle, 0, SANE_ACTION_GET_VALUE, &nopts, nullptr);

    if (status != SANE_STATUS_GOOD) {
        throw SaneException(status, "Failed to get the number of options");
    }

    log->info("[Sane] Loading {} options:", nopts);

    for (SANE_Int i = 1; i < nopts; ++i) {
        const SANE_Option_Descriptor *opt_desc = sane_get_option_descriptor(handle, i);
        if (opt_desc->type == SANE_TYPE_GROUP) {
            continue;
        }

        rawOptions.emplace_back(this, i);
    }
    options.refreshFilter();
}

constexpr size_t BUFFER_SIZE = 65536;

class DeviceCanceler {
    SANE_Handle handle;
    size_t     *expected;
    size_t     *read;

  public:
    DeviceCanceler(SANE_Handle _handle, size_t *_expected, size_t *_read)
        : handle(_handle), expected(_expected), read(_read) {}
    ~DeviceCanceler() {
        sane_cancel(handle);
        *expected = 0;
        *read     = 0;
    }

    DeviceCanceler(const DeviceCanceler &) = delete;
    DeviceCanceler(DeviceCanceler &&)      = delete;

    DeviceCanceler &operator=(const DeviceCanceler &) = delete;
    DeviceCanceler &operator=(DeviceCanceler &&)      = delete;
};

SaneImage SaneDevice::scan() {
    SaneImage       image;
    SANE_Status     status;
    SANE_Parameters parameters;

    DeviceCanceler canceler{handle, &expectedBytes, &readBytes};

    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(BUFFER_SIZE);

    expectedBytes = 0;
    readBytes     = 0;

    while (true) {
        status = sane_start(handle);
        if (status != SANE_STATUS_GOOD) {
            throw SaneException(status, "Failed to start scan");
        }

        status = sane_get_parameters(handle, &parameters);
        if (status != SANE_STATUS_GOOD) {
            throw SaneException(status, "Failed to get parameters");
        }

        logger()->debug("Sane parameters:");
        logger()->debug("  format:          {}", enum2str::toStr(parameters.format));
        logger()->debug("  last_frame:      {}", parameters.last_frame);
        logger()->debug("  bytes_per_line:  {}", parameters.bytes_per_line);
        logger()->debug("  pixels_per_line: {}", parameters.pixels_per_line);
        logger()->debug("  lines:           {}", parameters.lines);
        logger()->debug("  depth:           {}", parameters.depth);

        image.setup(parameters);
        if (expectedBytes == 0 && parameters.lines > 0) {
            expectedBytes = parameters.lines * parameters.bytes_per_line;
        }

        // Read the image data
        while (true) {
            SANE_Int numRead = 0;
            status           = sane_read(handle, buffer.get(), BUFFER_SIZE, &numRead);
            if (status == SANE_STATUS_EOF) {
                break;
            } else if (status != SANE_STATUS_GOOD) {
                throw SaneException(status, "Failed to read data");
            }
            image.addRawData({buffer.get(), (size_t)numRead}, parameters.format);
            readBytes += numRead;
        }

        if (parameters.last_frame == SANE_FALSE && parameters.format != SANE_FRAME_GRAY &&
            parameters.format != SANE_FRAME_RGB) {
            continue;
        }
        break;
    }

    return image;
}

double SaneDevice::scanProgress() const {
    if (expectedBytes == 0) {
        return 0.0;
    }
    return std::min(1.0, (double)readBytes / (double)expectedBytes);
}

SaneDevice::snapshot_t SaneDevice::optionsSnapshot() const {
    std::unordered_map<std::string, SaneOption::value_t> snapshot{};
    snapshot.reserve(rawOptions.size());
    for (const auto &opt : rawOptions) {
        snapshot[opt.getName()] = opt.getValue();
    }
    return snapshot;
}

void SaneDevice::applyOptionSnapshot(const snapshot_t &_snapshot) {
    ValueResetter resetter{&disableOptionReload, true};
    for (auto &opt : rawOptions) {
        auto iter = _snapshot.find(opt.getName());
        if (iter == _snapshot.end()) {
            continue;
        }
        // We will reload at the end anyway
        (void)opt.setValue(iter->second);
    }
    resetter.resetNow();
    reload_options();
}

void SaneDevice::cancelScan() {
    sane_cancel(handle);
}

} // namespace qscan::lib
