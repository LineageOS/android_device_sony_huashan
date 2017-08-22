# Camera permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml

# Camera HIDL interfaces
PRODUCT_PACKAGES += \
    android.hardware.camera.provider@2.4-impl \
    camera.device@3.2-impl
