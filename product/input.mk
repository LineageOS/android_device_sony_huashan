# Keys layouts
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/cyttsp-i2c.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/cyttsp-i2c.kl \
    $(LOCAL_PATH)/configs/gpio-keys.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/gpio-keys.kl \
    $(LOCAL_PATH)/configs/keypad_8960.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/keypad_8960.kl \
    $(LOCAL_PATH)/configs/keypad_8960_liquid.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/keypad_8960_liquid.kl \
    $(LOCAL_PATH)/configs/msm8960-snd-card_Button_Jack.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/msm8960-snd-card_Button_Jack.kl \
    $(LOCAL_PATH)/configs/synaptics_rmi4_i2c.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/synaptics_rmi4_i2c.kl

# Touchscreen configurations
PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/configs/clearpad.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/clearpad.idc \
    $(LOCAL_PATH)/configs/cyttsp4_mt.idc:$(TARGET_COPY_OUT_VENDOR)/usr/idc/cyttsp4_mt.idc
