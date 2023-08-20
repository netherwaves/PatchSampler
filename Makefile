# Project Name
TARGET = PatchSampler
# APP_TYPE = BOOT_QSPI
USE_FATFS = 1

# Sources
CPP_SOURCES = src/main.cpp

# Includes
C_INCLUDES += -Iinclude

# Library Locations
LIBDAISY_DIR = lib/libDaisy
DAISYSP_DIR = lib/DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

