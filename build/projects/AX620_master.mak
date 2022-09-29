SLAVE_PROJECT   := AX620_slave
CASCADAE_TYPE   := MASTER
ifneq ($(noclean),)
FORCE_CLEAN     := FALSE
else
FORCE_CLEAN     := TRUE
endif
ifneq ($(ignoreslave),)
CHECK_SLAVE	:= FALSE
else
CHECK_SLAVE	:= TRUE
endif
