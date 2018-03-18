# Lights
TARGET_PROVIDES_LIBLIGHT := true

# Lineage Hardware
ifeq ($(BOARD_AOSP_BASED),)
BOARD_HARDWARE_CLASS += \
    $(DEVICE_PATH)/lineagehw
endif
