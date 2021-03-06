##############################################################
#
#                   DO NOT EDIT THIS FILE!
#
##############################################################

# If the tool is built out of the kit, PIN_ROOT must be specified in the make invocation and point to the kit root.
MIN_SUPPORTED_REV:=62732
MAX_SUPPORTED_REV:=76887  #(pin 3.1)
#MAX_SUPPORTED_REV:=81201 #(pin 3.2 not yet supported)

ifdef PIN_ROOT
AVAIL_REV := $(shell pin -version | awk '/Rev/{print $$3}')
IS_CORRECT_VER := $(shell [ $(AVAIL_REV) -ge $(MIN_SUPPORTED_REV) ] && [ $(AVAIL_REV) -le $(MAX_SUPPORTED_REV)  ] && echo 1)
ifneq ($(IS_CORRECT_VER), 1)
$(info Pin Rev ${AVAIL_REV} )
$(error Required Pin Rev between ${MIN_SUPPORTED_REV} and ${MAX_SUPPORTED_REV}.)
endif
CONFIG_ROOT := $(PIN_ROOT)/source/tools/Config
else # PIN_ROOT not defined, so may be compiling from inside Pin kit
CONFIG_ROOT := ../Config
endif

CONFIG_EXISTS := $(shell [ -d ${CONFIG_ROOT} ] && echo 1 )
ifneq (${CONFIG_EXISTS}, 1)
$(info PIN_ROOT not set to point to the Pin directory.)
$(info Install Pin and set PIN_ROOT to point to it.)
$(error Required Pin Rev between ${MIN_SUPPORTED_REV} and ${MAX_SUPPORTED_REV}.)
endif

include $(CONFIG_ROOT)/makefile.config
include rules.makefile
include $(TOOLS_ROOT)/Config/makefile.default.rules

##############################################################
#
#                   DO NOT EDIT THIS FILE!
#
##############################################################
