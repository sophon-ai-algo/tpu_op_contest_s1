CROSS_TOOLCHAINS ?= ../../../bm_prebuilt_toolchains
ifeq ($(USING_CMODEL)$(HOST_ARCH),0aarch64)
HOST_CFLAGS          = -Wall -Werror -Wno-error=deprecated-declarations $(CONFIG_FLAGS) \
 -ffunction-sections -fdata-sections -ffp-contract=off\
 -fPIC -Wno-unused-function \
 -funwind-tables -rdynamic -fno-short-enums

HOST_CXXFLAGS        += -fexceptions -funwind-tables -rdynamic -fno-short-enums \
 -Wall -Werror -Wno-error=deprecated-declarations $(CONFIG_FLAGS) \
 -ffunction-sections -fdata-sections -fno-strict-aliasing -ffp-contract=off\
 -std=c++11 -Wno-unused-function \
 -fPIC

ifeq ($(PCIE_MODE_ENABLE_CPU), 1)
HOST_CFLAGS += -DPCIE_MODE_ENABLE_CPU
HOST_CXXFLAGS += -DPCIE_MODE_ENABLE_CPU
endif

HOST_CROSS_COMPILE := $(CROSS_TOOLCHAINS)/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
HOST_AS := $(HOST_CROSS_COMPILE)as
HOST_CC := $(HOST_CROSS_COMPILE)gcc
HOST_CXX := $(HOST_CROSS_COMPILE)g++
HOST_AR := $(HOST_CROSS_COMPILE)ar
HOST_LD := $(HOST_CROSS_COMPILE)ld
HOST_OBJCOPY := $(HOST_CROSS_COMPILE)objcopy
HOST_OBJDUMP := $(HOST_CROSS_COMPILE)objdump

else

HOST_CFLAGS          = -Wall -Werror -Wno-error=deprecated-declarations $(CONFIG_FLAGS) \
 -ffunction-sections -fdata-sections \
 -fPIC -Wno-unused-function \
 -funwind-tables -fno-short-enums

HOST_CXXFLAGS        += -funwind-tables -fno-short-enums \
 -Wall -Werror -Wno-error=deprecated-declarations -Wno-error=unused-result $(CONFIG_FLAGS) \
 -ffunction-sections -fdata-sections -fno-strict-aliasing \
 -std=c++11 -Wno-unused-function \
 -fPIC

HOST_CROSS_COMPILE :=
HOST_AS := $(HOST_CROSS_COMPILE)as
HOST_CC := $(HOST_CROSS_COMPILE)gcc
HOST_CXX := $(HOST_CROSS_COMPILE)g++
HOST_AR := $(HOST_CROSS_COMPILE)ar
HOST_LD := $(HOST_CROSS_COMPILE)ld
HOST_OBJCOPY := $(HOST_CROSS_COMPILE)objcopy
HOST_OBJDUMP := $(HOST_CROSS_COMPILE)objdump
endif

HOST_LDFLAGS         += -pthread \
 -Wl,--check-sections \
 -Wl,--gc-sections \
 -Wl,--unresolved-symbols=report-all

ifeq ($(DEBUG),1)
HOST_CFLAGS          += -g -DDEBUG
HOST_CXXFLAGS        += -g -DDEBUG
HOST_LDFLAGS         += -rdynamic
else
HOST_CFLAGS          += -O3
HOST_CXXFLAGS        += -O3
endif
