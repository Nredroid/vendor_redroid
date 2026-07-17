typedef struct hwc_composer_device_1 {
    /**
     * Common methods of the hardware composer device.  This *must* be the first member of
     * hwc_composer_device_1 as users of this structure will cast a hw_device_t to
     * hwc_composer_device_1 pointer in contexts where it's known the hw_device_t references a
     * hwc_composer_device_1.
     */
    struct hw_device_t common;

    /*
     * (*prepare)() is called for each frame before composition and is used by
     * SurfaceFlinger to determine what composition steps the HWC can handle.
     *
     * (*prepare)() can be called more than once, the last call prevails.
     *
     * The HWC responds by setting the compositionType field in each layer to
     * either HWC_FRAMEBUFFER, HWC_OVERLAY, or HWC_CURSOR_OVERLAY. For the
     * HWC_FRAMEBUFFER type, composition for the layer is handled by
     * SurfaceFlinger with OpenGL ES. For the latter two overlay types,
     * the HWC will have to handle the layer's composition. compositionType
     * and hints are preserved between (*prepare)() calles unless the
     * HWC_GEOMETRY_CHANGED flag is set.
     *
     * (*prepare)() is called with HWC_GEOMETRY_CHANGED to indicate that the
     * list's geometry has changed, that is, when more than just the buffer's
     * handles have been updated. Typically this happens (but is not limited to)
     * when a window is added, removed, resized or moved. In this case
     * compositionType and hints are reset to their default value.
     *
     * For HWC 1.0, numDisplays will always be one, and displays[0] will be
     * non-NULL.
     *
     * For HWC 1.1, numDisplays will always be HWC_NUM_PHYSICAL_DISPLAY_TYPES.
     * Entries for unsupported or disabled/disconnected display types will be
     * NULL.
     *
     * In HWC 1.3, numDisplays may be up to HWC_NUM_DISPLAY_TYPES. The extra
     * entries correspond to enabled virtual displays, and will be non-NULL.
     *
     * returns: 0 on success. An negative error code on error. If an error is
     * returned, SurfaceFlinger will assume that none of the layer will be
     * handled by the HWC.
     */
    int (*prepare)(struct hwc_composer_device_1 *dev,
                    size_t numDisplays, hwc_display_contents_1_t** displays);

    /*
     * (*set)() is used in place of eglSwapBuffers(), and assumes the same
     * functionality, except it also commits the work list atomically with
     * the actual eglSwapBuffers().
     *
     * The layer lists are guaranteed to be the same as the ones returned from
     * the last call to (*prepare)().
     *
     * When this call returns the caller assumes that the displays will be
     * updated in the near future with the content of their work lists, without
     * artifacts during the transition from the previous frame.
     *
     * A display with zero layers indicates that the entire composition has
     * been handled by SurfaceFlinger with OpenGL ES. In this case, (*set)()
     * behaves just like eglSwapBuffers().
     *
     * For HWC 1.0, numDisplays will always be one, and displays[0] will be
     * non-NULL.
     *
     * For HWC 1.1, numDisplays will always be HWC_NUM_PHYSICAL_DISPLAY_TYPES.
     * Entries for unsupported or disabled/disconnected display types will be
     * NULL.
     *
     * In HWC 1.3, numDisplays may be up to HWC_NUM_DISPLAY_TYPES. The extra
     * entries correspond to enabled virtual displays, and will be non-NULL.
     *
     * IMPORTANT NOTE: There is an implicit layer containing opaque black
     * pixels behind all the layers in the list. It is the responsibility of
     * the hwcomposer module to make sure black pixels are output (or blended
     * from).
     *
     * IMPORTANT NOTE: In the event of an error this call *MUST* still cause
     * any fences returned in the previous call to set to eventually become
     * signaled.  The caller may have already issued wait commands on these
     * fences, and having set return without causing those fences to signal
     * will likely result in a deadlock.
     *
     * returns: 0 on success. A negative error code on error:
     *    HWC_EGL_ERROR: eglGetError() will provide the proper error code (only
     *        allowed prior to HWComposer 1.1)
     *    Another code for non EGL errors.
     */
    int (*set)(struct hwc_composer_device_1 *dev,
                size_t numDisplays, hwc_display_contents_1_t** displays);

    /*
     * eventControl(..., event, enabled)
     * Enables or disables h/w composer events for a display.
     *
     * eventControl can be called from any thread and takes effect
     * immediately.
     *
     *  Supported events are:
     *      HWC_EVENT_VSYNC
     *
     * returns -EINVAL if the "event" parameter is not one of the value above
     * or if the "enabled" parameter is not 0 or 1.
     */
    int (*eventControl)(struct hwc_composer_device_1* dev, int disp,
            int event, int enabled);

    union {
        /*
         * For HWC 1.3 and earlier, the blank() interface is used.
         *
         * blank(..., blank)
         * Blanks or unblanks a display's screen.
         *
         * Turns the screen off when blank is nonzero, on when blank is zero.
         * Multiple sequential calls with the same blank value must be
         * supported.
         * The screen state transition must be be complete when the function
         * returns.
         *
         * returns 0 on success, negative on error.
         */
        int (*blank)(struct hwc_composer_device_1* dev, int disp, int blank);

        /*
         * For HWC 1.4 and above, setPowerMode() will be used in place of
         * blank().
         *
         * setPowerMode(..., mode)
         * Sets the display screen's power state.
         *
         * Refer to the documentation of the HWC_POWER_MODE_* constants
         * for information about each power mode.
         *
         * The functionality is similar to the blank() command in previous
         * versions of HWC, but with support for more power states.
         *
         * The display driver is expected to retain and restore the low power
         * state of the display while entering and exiting from suspend.
         *
         * Multiple sequential calls with the same mode value must be supported.
         *
         * The screen state transition must be be complete when the function
         * returns.
         *
         * returns 0 on success, negative on error.
         */
        int (*setPowerMode)(struct hwc_composer_device_1* dev, int disp,
                int mode);
    };

    /*
     * Used to retrieve information about the h/w composer
     *
     * Returns 0 on success or -errno on error.
     */
    int (*query)(struct hwc_composer_device_1* dev, int what, int* value);

    /*
     * (*registerProcs)() registers callbacks that the h/w composer HAL can
     * later use. It will be called immediately after the composer device is
     * opened with non-NULL procs. It is FORBIDDEN to call any of the callbacks
     * from within registerProcs(). registerProcs() must save the hwc_procs_t
     * pointer which is needed when calling a registered callback.
     */
    void (*registerProcs)(struct hwc_composer_device_1* dev,
            hwc_procs_t const* procs);

    /*
     * This field is OPTIONAL and can be NULL.
     *
     * If non NULL it will be called by SurfaceFlinger on dumpsys
     */
    void (*dump)(struct hwc_composer_device_1* dev, char *buff, int buff_len);

    /*
     * (*getDisplayConfigs)() returns handles for the configurations available
     * on the connected display. These handles must remain valid as long as the
     * display is connected.
     *
     * Configuration handles are written to configs. The number of entries
     * allocated by the caller is passed in *numConfigs; getDisplayConfigs must
     * not try to write more than this number of config handles. On return, the
     * total number of configurations available for the display is returned in
     * *numConfigs. If *numConfigs is zero on entry, then configs may be NULL.
     *
     * Hardware composers implementing HWC_DEVICE_API_VERSION_1_3 or prior
     * shall choose one configuration to activate and report it as the first
     * entry in the returned list. Reporting the inactive configurations is not
     * required.
     *
     * HWC_DEVICE_API_VERSION_1_4 and later provide configuration management
     * through SurfaceFlinger, and hardware composers implementing these APIs
     * must also provide getActiveConfig and setActiveConfig. Hardware composers
     * implementing these API versions may choose not to activate any
     * configuration, leaving configuration selection to higher levels of the
     * framework.
     *
     * Returns 0 on success or a negative error code on error. If disp is a
     * hotpluggable display type and no display is connected, an error shall be
     * returned.
     *
     * This field is REQUIRED for HWC_DEVICE_API_VERSION_1_1 and later.
     * It shall be NULL for previous versions.
     */
    int (*getDisplayConfigs)(struct hwc_composer_device_1* dev, int disp,
            uint32_t* configs, size_t* numConfigs);

    /*
     * (*getDisplayAttributes)() returns attributes for a specific config of a
     * connected display. The config parameter is one of the config handles
     * returned by getDisplayConfigs.
     *
     * The list of attributes to return is provided in the attributes
     * parameter, terminated by HWC_DISPLAY_NO_ATTRIBUTE. The value for each
     * requested attribute is written in order to the values array. The
     * HWC_DISPLAY_NO_ATTRIBUTE attribute does not have a value, so the values
     * array will have one less value than the attributes array.
     *
     * This field is REQUIRED for HWC_DEVICE_API_VERSION_1_1 and later.
     * It shall be NULL for previous versions.
     *
     * If disp is a hotpluggable display type and no display is connected,
     * or if config is not a valid configuration for the display, a negative
     * error code shall be returned.
     */
    int (*getDisplayAttributes)(struct hwc_composer_device_1* dev, int disp,
            uint32_t config, const uint32_t* attributes, int32_t* values);

    /*
     * (*getActiveConfig)() returns the index of the configuration that is
     * currently active on the connected display. The index is relative to
     * the list of configuration handles returned by getDisplayConfigs. If there
     * is no active configuration, HWC_ERROR shall be returned.
     *
     * Returns the configuration index on success or HWC_ERROR on error.
     *
     * This field is REQUIRED for HWC_DEVICE_API_VERSION_1_4 and later.
     * It shall be NULL for previous versions.
     */
    int (*getActiveConfig)(struct hwc_composer_device_1* dev, int disp);

    /*
     * (*setActiveConfig)() instructs the hardware composer to switch to the
     * display configuration at the given index in the list of configuration
     * handles returned by getDisplayConfigs.
     *
     * If this function returns without error, any subsequent calls to
     * getActiveConfig shall return the index set by this function until one
     * of the following occurs:
     *   1) Another successful call of this function
     *   2) The display is disconnected
     *
     * Returns 0 on success or a negative error code on error. If disp is a
     * hotpluggable display type and no display is connected, or if index is
     * outside of the range of hardware configurations returned by
     * getDisplayConfigs, an error shall be returned.
     *
     * This field is REQUIRED for HWC_DEVICE_API_VERSION_1_4 and later.
     * It shall be NULL for previous versions.
     */
    int (*setActiveConfig)(struct hwc_composer_device_1* dev, int disp,
            int index);
    /*
     * Asynchronously update the location of the cursor layer.
     *
     * Within the standard prepare()/set() composition loop, the client
     * (surfaceflinger) can request that a given layer uses dedicated cursor
     * composition hardware by specifiying the HWC_IS_CURSOR_LAYER flag. Only
     * one layer per display can have this flag set. If the layer is suitable
     * for the platform's cursor hardware, hwcomposer will return from prepare()
     * a composition type of HWC_CURSOR_OVERLAY for that layer. This indicates
     * not only that the client is not responsible for compositing that layer,
     * but also that the client can continue to update the position of that layer
     * after a call to set(). This can reduce the visible latency of mouse
     * movement to visible, on-screen cursor updates. Calls to
     * setCursorPositionAsync() may be made from a different thread doing the
     * prepare()/set() composition loop, but care must be taken to not interleave
     * calls of setCursorPositionAsync() between calls of set()/prepare().
     *
     * Notes:
     * - Only one layer per display can be specified as a cursor layer with
     *   HWC_IS_CURSOR_LAYER.
     * - hwcomposer will only return one layer per display as HWC_CURSOR_OVERLAY
     * - This returns 0 on success or -errno on error.
     * - This field is optional for HWC_DEVICE_API_VERSION_1_4 and later. It
     *   should be null for previous versions.
     */
    int (*setCursorPositionAsync)(struct hwc_composer_device_1 *dev, int disp, int x_pos, int y_pos);

    /*
     * Reserved for future use. Must be NULL.
     */
    void* reserved_proc[1];

} hwc_composer_device_1_t;