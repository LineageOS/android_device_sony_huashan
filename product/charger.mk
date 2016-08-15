# Healthd packages
ifneq ($(BOARD_AOSP_BASED),)
PRODUCT_PACKAGES += \
    charger_res_images
endif

# Healthd properties
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    ro.enable_boot_charger_mode=1
