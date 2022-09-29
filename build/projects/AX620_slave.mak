SUPPORT_RAMDISK := TRUE
CASCADAE_TYPE   := SLAVE
ifneq ($(noclean),)
FORCE_CLEAN     := FALSE
else
FORCE_CLEAN     := TRUE
endif
