# FM packages
PRODUCT_PACKAGES += \
    init.qcom.fm.sh

# FM properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vendor.fm.use_audio_session=true \
    vendor.hw.fm.internal_antenna=true
