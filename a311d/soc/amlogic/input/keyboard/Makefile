#
# Makefile for the keyboard device drivers.
# Support OpenHammony HDF Input
#

# Each configuration option enables a list of files.

ifeq ($(CONFIG_DRIVERS_HDF_INPUT), y)
HDF_ROOT_DIR = -I$(srctree)/drivers/hdf
ccflags-$(CONFIG_DRIVERS_HDF_INPUT) += $(HDF_ROOT_DIR)/framework/model/input/driver \
             $(HDF_ROOT_DIR)/framework/include/core \
             $(HDF_ROOT_DIR)/framework/core/common/include/host \
             $(HDF_ROOT_DIR)/framework/include/utils \
             $(HDF_ROOT_DIR)/framework/include/osal \
             $(HDF_ROOT_DIR)/framework/ability/sbuf/include \
             $(HDF_ROOT_DIR)/khdf/osal/include \
             $(HDF_ROOT_DIR)/evdev
endif

obj-y	+= gpio_keypad.o
