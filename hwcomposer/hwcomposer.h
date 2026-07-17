#include <mutex>
#include <thread>

#include <cstdbool>
#include <cstdint>

#include <hardware/hwcomposer.h>

typedef struct redroid_hwc_device {
    /* static data */
    hwc_composer_device_1_t device;

    std::mutex mutex;

    std::thread* hdmi_thread;
    std::thread* event_thread;

    kms::Card* card;

    HWCDisplay* displays[MAX_DISPLAYS];

    kms::Connector* primaryConector;
    kms::Connector* externalConector;

    drmEventContext evctx;

    const hwc_procs_t* cb_procs;
} redroid_hwc_device_t;