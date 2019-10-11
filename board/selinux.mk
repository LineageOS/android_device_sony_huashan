# Device sepolicies
BOARD_SEPOLICY_DIRS += \
    $(DEVICE_PATH)/sepolicy

# LineageOS device sepolicies
ifeq ($(BOARD_AOSP_BASED),)
BOARD_SEPOLICY_DIRS += \
    $(DEVICE_PATH)/sepolicy-lineage
endif
