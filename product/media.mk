# Media configurations
ifneq ($(BOARD_AOSP_BASED),)
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/media_codecs_aosp.xml:system/etc/media_codecs.xml \
    $(LOCAL_PATH)/configs/media_codecs_performance_aosp.xml:system/etc/media_codecs_performance.xml \
    $(LOCAL_PATH)/configs/media_profiles_aosp.xml:system/etc/media_profiles.xml
else
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/media_codecs.xml:system/etc/media_codecs.xml \
    $(LOCAL_PATH)/configs/media_codecs_performance.xml:system/etc/media_codecs_performance.xml \
    $(LOCAL_PATH)/configs/media_profiles.xml:system/etc/media_profiles.xml
endif
