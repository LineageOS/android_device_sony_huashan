# Sensors configurations
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/sap.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sap.conf \
    $(LOCAL_PATH)/configs/sensors.conf:$(TARGET_COPY_OUT_VENDOR)/etc/sensors.conf

# Sensors packages
PRODUCT_PACKAGES += \
    sensors.msm8960
