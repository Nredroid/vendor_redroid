/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <log/log.h>
#include <mutex>
#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#include "hwcomposer.h"
#include <EGL/egl.h>

/*****************************************************************************/

struct hwc_context_t {
    hwc_composer_device_1_t device;
    /* our private state goes below here */
};

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static struct hw_module_methods_t hwc_module_methods = {
    .open = hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = HWC_HARDWARE_MODULE_ID,
        .name = "Sample hwcomposer module",
        .author = "The Android Open Source Project",
        .methods = &hwc_module_methods,
    }
};

/*****************************************************************************/

#if 0
static void dump_layer(hwc_layer_1_t const* l) {
    ALOGD("\ttype=%d, flags=%08x, handle=%p, tr=%02x, blend=%04x, {%d,%d,%d,%d}, {%d,%d,%d,%d}",
            l->compositionType, l->flags, l->handle, l->transform, l->blending,
            l->sourceCrop.left,
            l->sourceCrop.top,
            l->sourceCrop.right,
            l->sourceCrop.bottom,
            l->displayFrame.left,
            l->displayFrame.top,
            l->displayFrame.right,
            l->displayFrame.bottom);
}
#endif

static int hwc_prepare(hwc_composer_device_1_t * /*dev*/,
        size_t /*numDisplays*/, hwc_display_contents_1_t** displays) {
    if (displays && (displays[0]->flags & HWC_GEOMETRY_CHANGED)) {
        for (size_t i=0 ; i<displays[0]->numHwLayers ; i++) {
            //dump_layer(&list->hwLayers[i]);
            displays[0]->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
        }
    }
    return 0;
}

static int hwc_set(hwc_composer_device_1_t * /*dev*/,
        size_t /*numDisplays*/, hwc_display_contents_1_t** displays)
{
    //for (size_t i=0 ; i<list->numHwLayers ; i++) {
    //    dump_layer(&list->hwLayers[i]);
    //}

    EGLBoolean success = eglSwapBuffers((EGLDisplay)displays[0]->dpy,
            (EGLSurface)displays[0]->sur);
    if (!success) {
        return HWC_EGL_ERROR;
    }
    return 0;
}

static int hwc_device_close(struct hw_device_t *dev)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
    if (ctx) {
        free(ctx);
    }
    return 0;
}

/*****************************************************************************/

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    bool stream_open = property_get_bool("ro.boot.use_redroid_stream",0);
    int32_t redroid_fps = property_get_int32("ro.boot.redroid_fps",15);
    if (redroid_fps < 1){ redroid_fps = 15; }
    if (stream_open != nullptr){
    ALOGI("hwc_open, streaming enabled");

    }
    int status = -EINVAL;
    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
        struct hwc_context_t *dev;
        dev = (hwc_context_t*)malloc(sizeof(*dev));

        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));
        /* pdev == dev-> device*/
        /* initialize the procs */
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = HWC_DEVICE_API_VERSION_1_0;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = hwc_device_close;
        dev->device.prepare = hwc_prepare;
        dev->device.set = hwc_set;
        dev->device.blank = hwc_blank;
        dev->device.query = hwc_query;
        dev->device.dump = nullptr;
        dev->device.registerProcs = hwc_register_procs;
        dev->device.getDisplayAttributes = hwc_get_display_attributes;
        dev->device.getDisplayConfigs = hwc_get_display_configs;
        *device = &dev->device.common;
        status = 0;
    } else {
    ALOGE("%s called with bad name %s","hwc_open",name);}
    return status;
}
static int hwc_get_display_attributes(struct hwc_composer_device_1* dev,
                                      int disp, uint32_t config __unused,
                                      const uint32_t* attributes, int32_t* values) {
    if (attributes == nullptr || values == nullptr) {
        return -EINVAL;
    }

    uint32_t attr = *attributes;
    if (attr != HWC_DISPLAY_NO_ATTRIBUTE) {
        if (disp != HWC_DISPLAY_PRIMARY) {
            ALOGE("unknown display type %u", disp);
            return -EINVAL;
        }

        int index = 0;
        do {
            int32_t value;
            switch(attr) {
                case HWC_DISPLAY_VSYNC_PERIOD:
                    value = *(int32_t*)((uintptr_t)dev + 244);
                    break;

                case HWC_DISPLAY_WIDTH:
                    value = property_get_int32("ro.boot.redroid_width", 720);
                    break;

                case HWC_DISPLAY_HEIGHT:
                    value = property_get_int32("ro.boot.redroid_height", 1280);
                    break;

                case HWC_DISPLAY_DPI_X:
                case HWC_DISPLAY_DPI_Y:
                    value = property_get_int32("ro.sf.lcd_density", 320) * 1000;
                    break;

                default:
                    ALOGW("unknown display attribute %u", attr);
                    value = -EINVAL;
            }

            values[index] = value;
            attr = attributes[index + 1];
            index++;

        } while (attr != HWC_DISPLAY_NO_ATTRIBUTE);
    }
    return 0;
}

static int hwc_get_display_configs(struct hwc_composer_device_1* dev __unused,
                                   int disp, uint32_t* configs, size_t* numConfigs) {
    if (*numConfigs == 0) {
        return 0;
    }

    if (disp == HWC_DISPLAY_PRIMARY) {
        configs[0] = 0;
        *numConfigs = 1;
        return 0;
    }

    return -EINVAL;
}


static int hwc_blank(struct hwc_composer_device_1* dev, int disp, int blank)
{
    int ret = -EINVAL
    if (blank == 0){
      ret = 0;
    }
    return 0;
}

static int hwc_query(struct hwc_composer_device_1* dev, int what, int* value)
{
    redroid_hwc_device_t* hwc_dev = (redroid_hwc_device_t*)dev;
    std::unique_lock<std::mutex> lock(hwc_dev->mutex);
    switch (what) {
    case HWC_BACKGROUND_LAYER_SUPPORTED:
        // we don't support the background layer yet
        value[0] = 0;
        break;
    case HWC_VSYNC_PERIOD:
        ALOGW("Query for deprecated vsync value, returning %dHz", redroid_fps);
        *value = 1000 * 1000 * 1000 / hwc_dev.vsync_period;
        break;
    case HWC_DISPLAY_TYPES_SUPPORTED:
        *value = HWC_DISPLAY_PRIMARY_BIT | HWC_DISPLAY_EXTERNAL_BIT;
        break;
    default:
        // unsupported query
        return -EINVAL;
    }

    return 0;
}

static void hwc_register_procs(struct hwc_composer_device_1* dev,
                               hwc_procs_t const* procs) {
    redroid_hwc_device_t* hwc_dev = (redroid_hwc_device_t*)dev;
    hwc_dev->procs = procs;

    hwc_dev->display->procs = procs;
}