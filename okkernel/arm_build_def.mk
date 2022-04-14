CROSS_TOOLCHAINS ?= ../../../bm_prebuilt_toolchains
ARM_OPTFLAGS := \
	-std=gnu99 \
	-Os \
	-Wall \
	-Wextra \
	-Werror \
 	-Wno-error=deprecated-declarations \
	-Wno-error=sign-compare \
  -Wno-error=int-to-pointer-cast \
	-Wno-error=unused-variable \
	-Wno-error=unused-but-set-variable

ARM_COMMONFLAGS := \
	-mcpu=arm926ej-s \
	-mfpu=vfp \
	-fno-short-enums \
	-mfloat-abi=softfp \
	-MMD -MP

ARM_ASFLAGS := \
	$(ARM_OPTFLAGS) \
	$(ARM_COMMONFLAGS)

ARM_CFLAGS := $(filter-out -DUSING_FULLNET -DBM_TV_GEN -DDIFF_DUMP -DDUMP_BIN -DLOCAL_MEM_DUMP_BY_STEP -DUSING_FW_DEBUG -DUSING_CMODEL,$(CFLAGS)) \
	$(ARM_COMMONFLAGS) \
	$(ARM_OPTFLAGS) \
	$(CONFIG_FLAGS) \
	-ffunction-sections \
	-fdata-sections \
	-nostdlib

ifeq ($(ARM_DEBUG), 1)
ARM_ASFLAGS += -O0 -g
ARM_CFLAGS += -O0 -g
endif

ARM_LDFLAGS := \
	$(ARM_COMMONFLAGS) \
	-Wl,--no-enum-size-warning \
	-Wl,--check-sections \
	-Wl,--gc-sections \
	-Wl,--unresolved-symbols=report-all

ARM_CROSS_COMPILE := $(CROSS_TOOLCHAINS)/gcc-arm-none-eabi-7-2017-q4-major/bin/arm-none-eabi-
ARM_AS := $(ARM_CROSS_COMPILE)as
ARM_CC := $(ARM_CROSS_COMPILE)gcc
ARM_CXX := $(ARM_CROSS_COMPILE)g++
ARM_AR := $(ARM_CROSS_COMPILE)ar
ARM_LD := $(ARM_CROSS_COMPILE)ld
ARM_OBJCOPY := $(ARM_CROSS_COMPILE)objcopy
ARM_OBJDUMP := $(ARM_CROSS_COMPILE)objdump
