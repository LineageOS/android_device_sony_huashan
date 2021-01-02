# LineageOS Trust HAL
ifeq ($(BOARD_AOSP_BASED),)
PRODUCT_PACKAGES += \
    vendor.lineage.trust@1.0-service
endif

# USB permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml

# USB HIDL interfaces
PRODUCT_PACKAGES += \
    android.hardware.usb@1.0-service.basic
