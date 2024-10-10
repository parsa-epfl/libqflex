#- * - Mode : makefile - * -
#
#This Makefile example is fairly independent from the main QEMU makefile
#so users can take and adapt it for their build.
#
#Bryan Perdrizat:
#This file is included from the main QEMU(contrib / plugin) Makefile
#and is aimed toward building the libqflex - PLUGIN static library in
#a separated tree.The variable not defined here are therefore defined
#from this parent's Makefile

# ─── Variable ─────────────────────────────────────────────────────────────────

#A simlink or hardlink is required in contrib / plugin / otherwise the next line
#make no sense
LIBQFLEX_VPATH := $(VPATH)/middleware
BUILD_DIR := $(LIBQFLEX_VPATH)/build

#Add your new library directory here
LIBQFLEX_NAME :=
LIBQFLEX_NAME += trace
#LIBQFLEX_NAME += timing
#NAME become->lib + {NAME } +.so
LIBQFLEX_LIBNAME 	:= $(addsuffix $(SO_SUFFIX),$(addprefix lib,$(LIBQFLEX_NAME)))
LIBQFLEX_FULL_PATH 	:= $(addprefix $(BUILD_DIR)/,$(LIBQFLEX_LIBNAME))

#All the *.c files and all the.o files
SRC_FILES := $(foreach dir,$(LIBQFLEX_NAME),$(wildcard $(LIBQFLEX_VPATH)/$(dir)/*.c))
OBJ_FILES := $(patsubst $(LIBQFLEX_VPATH)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

# ─── Build Rules ──────────────────────────────────────────────────────────────

# Entrypoint, will build all library defined
custom_all: $(LIBQFLEX_FULL_PATH)

# Build all library by *filter*ing the subdirectory, otherwise all object would
# used to build the lib
$(BUILD_DIR)/lib%.so: $(filter $(BUILD_DIR)%, $(OBJ_FILES))
	$(CC) -shared -o $@ $^ $(LDLIBS)

# Build all files but create their builds directories first (|)
$(OBJ_FILES): $(SRC_FILES) | mkdir
	$(CC) $(CFLAGS) $(PLUGIN_CFLAGS) -c -o $@ $<

# ──────────────────────────────────────────────────────────────────────────────

custom_clean:
	rm -rf $(BUILD_DIR)
mkdir:
	mkdir -p $(dir $(OBJ_FILES))

var:
	$(info shell pwd $(shell pwd))
	$(info $$LIBQFLEX_FULL_PATH ${LIBQFLEX_FULL_PATH})
	$(info $$LIBQFLEX_VPATH ${LIBQFLEX_VPATH})
	$(info $$LIBQFLEX_LIBNAME ${LIBQFLEX_LIBNAME})
	$(info $$BUILD_DIR ${BUILD_DIR})
	$(info $$SRC ${SRC_FILES})
	$(info $$OBJ ${OBJ_FILES})

	$(info $$SRC_PATH ${SRC_PATH})

.PHONY: custom_all custom_clean var mkdir
