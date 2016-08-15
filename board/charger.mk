# Healthd
BOARD_CHARGER_ENABLE_SUSPEND := true
BOARD_CHARGER_DISABLE_INIT_BLANK := true
BACKLIGHT_PATH := /sys/devices/i2c-10/10-0040/leds/lcd-backlight1/brightness
SECONDARY_BACKLIGHT_PATH := /sys/devices/i2c-10/10-0040/leds/lcd-backlight2/brightness
RED_LED_PATH := /sys/devices/i2c-10/10-0047/leds/LED1_R/brightness
GREEN_LED_PATH := /sys/devices/i2c-10/10-0047/leds/LED1_G/brightness
BLUE_LED_PATH := /sys/devices/i2c-10/10-0047/leds/LED1_B/brightness

# Healthd library extension
ifneq ($(BOARD_AOSP_BASED),)
BOARD_HAL_STATIC_LIBRARIES := libhealthd.board
BOARD_HEALTHD_CUSTOM_CHARGER_RES := $(DEVICE_PATH)/charger/images
endif
