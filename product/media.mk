# Media configurations
ifneq ($(BOARD_AOSP_BASED),)
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/media_codecs_performance_aosp.xml:system/etc/media_codecs_performance.xml
else
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/media_codecs_performance.xml:system/etc/media_codecs_performance.xml
endif
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/media_codecs.xml:system/etc/media_codecs.xml \
    $(LOCAL_PATH)/configs/media_profiles.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles.xml

# Media properties
PRODUCT_PROPERTY_OVERRIDES += \
    media.settings.xml=/vendor/etc/media_profiles.xml
