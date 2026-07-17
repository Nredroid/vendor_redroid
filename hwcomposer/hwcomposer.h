#include <mutex>
#include <thread>

#include <cstdbool>
#include <cstdint>

#include <hardware/hwcomposer.h>

typedef struct redroid_hwc_device {
    hwc_composer_device_1_t device;
    hwc_procs_t const*       procs;
    pthread_t                vsync_thread;
    bool                     stop_thread;
    int32_t                  vsync_period;
    std::mutex               hwc_mutex;
    bool                     vsync_enabled;
} redroid_hwc_device_t;