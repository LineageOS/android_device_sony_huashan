# Browser package
PRODUCT_PACKAGES += \
    Jelly

# Launcher packages
ifneq ($(BOARD_AOSP_BASED),)
PRODUCT_PACKAGES += \
    TrebuchetQuickStepGo
endif

# Music packages
PRODUCT_PACKAGES += \
    Eleven

# Recorder packages
PRODUCT_PACKAGES += \
    Recorder
