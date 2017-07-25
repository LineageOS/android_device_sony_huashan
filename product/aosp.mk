# AOSP specific products
ifneq ($(BOARD_AOSP_BASED),)

# AOSP packages
PRODUCT_PACKAGES += \
    Launcher3 \
    messaging \
    SoundRecorder \
    SpeechRecorder \
    Terminal \
    WallpaperPicker

# AOSP properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.setupwizard.rotation_locked=true \
    ro.storage_manager.enabled=true

endif
