# Power package
PRODUCT_PACKAGES += \
    android.hardware.power@1.0-impl \
    power.qcom

# QCOM-perf properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.extension_library=libqti-perfd-client.so
