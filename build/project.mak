AX_PRJ_LIST += AX620_demo
AX_PRJ_LIST += AX620_nand

AX_PRJ_LIST += AX620U_demo

AX_PRJ_LIST += AX620U_nand

AX_PRJ_LIST += AX620U_38board


ifneq ($(MAKECMDGOALS),plist)
ifneq ($(MAKECMDGOALS),scan)

ifneq ($(project),)
PROJECT := $(project)
else ifneq ($(p),)
PROJECT := $(p)
else
$(error project(p) is not asigned, example: p=AX620_evb or call "make plist" to show all projects)
endif

ifeq ($(words $(subst $(PROJECT),,$(AX_PRJ_LIST))), $(words $(AX_PRJ_LIST)))
$(error project "$(PROJECT)" does not exist!!!)
endif

CHIP_NAME := CHIP_$(shell echo  ${PROJECT} | awk '{match($$0, /(^AX[0-9]+)[A-Za-z]?_/,a);print a[1]}')

endif # ifeq ($(MAKECMDGOALS),scan)
endif # ifeq ($(MAKECMDGOALS),list)

define list_projects
	@echo -e "\e[34;1m -------project list------\033[0m"
	@for prj in $(AX_PRJ_LIST); do \
		echo -e "\e[34;1m  p=$$prj\033[0m";\
	done
endef

ifeq ($(MAKECMDGOALS),plist)
plist:
	$(call list_projects)
endif
