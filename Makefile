# Ya-Yu firmware profiles. Dependencies are checked out as Git submodules.
PROFILE ?= seed

LIBDAISY_DIR ?= lib/libDaisy
DAISYSP_DIR ?= lib/DaisySP

# Optional local paths are intentionally not committed.
-include local.mk

ifeq ($(PROFILE),seed)
override TARGET := yayu_seed
override BUILD_DIR := build/seed
APP_TYPE = BOOT_QSPI
else ifeq ($(PROFILE),spotykach)
override TARGET := yayu_spotykach
override BUILD_DIR := build/spotykach
APP_TYPE = BOOT_SRAM
LDSCRIPT = boot/spotykach_sram.lds
BOOT_BIN = ../../bootloader-spotykach-v2.bin
else
$(error Unknown PROFILE "$(PROFILE)". Use seed or spotykach.)
endif

CPP_STANDARD = -std=c++17
C_DEFS = -DTARGET_DAISY -DUSE_DAISYSP_LGPL
C_USR_FLAGS = -ffast-math -funroll-loops
C_INCLUDES = -Isrc -I$(DAISYSP_DIR)/DaisySP-LGPL/Source -I$(DAISYSP_DIR)/Source/Utility

CPP_SOURCES = \
	YaYu_main.cpp \
	src/yayu_engine.cpp \
	src/models/karplus_string.cpp \
	$(DAISYSP_DIR)/DaisySP-LGPL/Source/Effects/reverbsc.cpp \
	$(DAISYSP_DIR)/DaisySP-LGPL/Source/Effects/bitcrush.cpp \
	$(DAISYSP_DIR)/DaisySP-LGPL/Source/Effects/fold.cpp \
	$(DAISYSP_DIR)/DaisySP-LGPL/Source/Filters/tone.cpp

SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

.PHONY: libs seed spotykach flash-seed flash-spotykach

libs:
	$(MAKE) -C $(LIBDAISY_DIR)
	$(MAKE) -C $(DAISYSP_DIR)

seed: libs
	$(MAKE) PROFILE=seed all

spotykach: libs
	$(MAKE) PROFILE=spotykach all

flash-seed: seed
	$(MAKE) PROFILE=seed program-dfu

flash-spotykach: spotykach
	$(MAKE) PROFILE=spotykach program-dfu
