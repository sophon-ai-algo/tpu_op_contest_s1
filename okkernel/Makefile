USING_CMODEL     ?= 0
HOST_ARCH        ?= x86
DEBUG            ?= 0
ENABLE_ICACHE    ?= 0
REL_TOP          ?= .

include ./arm_build_def.mk
include ./host_build_def.mk

ifeq ($(ENABLE_ICACHE), 0)
FW_LIB = firmware.a
else
FW_LIB = firmware_en_icache.a
endif


ifeq ($(USING_CMODEL),1)
	CONFIG_FLAGS    += -DUSING_CMODEL
	OKK_LIB_PATH += -Wl,-rpath=./lib/cmodel/ -L./lib/cmodel/
	OKK_LIB_PATH += -Wl,-rpath=$(REL_TOP)/lib/bmnn/cmodel/ -L$(REL_TOP)/lib/bmnn/cmodel/
	OKK_LIB_PATH += $(EXTRA_CMODEL_LIBS)
	output_dir=./build/cmodel/
else ifeq ($(HOST_ARCH), x86)
	OKK_LIB_PATH += -Wl,-rpath=./lib/pcie -L./lib/pcie
	OKK_LIB_PATH += -Wl,-rpath=$(REL_TOP)/lib/bmnn/pcie -L$(REL_TOP)/lib/bmnn/pcie
	OKK_LIB_PATH += $(EXTRA_PCIE_LIBS)
	output_dir=./build/pcie/
else ifeq ($(HOST_ARCH), aarch64)
	OKK_LIB_PATH += -Wl,-rpath=./lib/soc -L./lib/soc
	OKK_LIB_PATH += -Wl,-rpath=$(REL_TOP)/lib/bmnn/soc -L$(REL_TOP)/lib/bmnn/soc
	OKK_LIB_PATH += $(EXTRA_AARCH64_LIBS)
	output_dir=./build/soc/
endif

OKK_INC_PATH += -I./include
OKK_INC_PATH += -I$(REL_TOP)/include/bmlib
OKK_INC_PATH += $(EXTRA_INC_PATH)

OKK_LINK := bm168x_ddr.lds


CPP_ARGS = $(HOST_CXXFLAGS) $(HOST_LDFLAGS) $(OKK_INC_PATH) $(OKK_LIB_PATH)

ifeq ($(USING_CMODEL),1)
        CPP_ARGS += -lbmlib
	C_ARGS = $(HOST_CFLAGS) $(HOST_LDFLAGS) $(OKK_INC_PATH) $(OKK_LIB_PATH)
	dev_cc = $(HOST_CC)
else
	CPP_ARGS += -lbmlib
	C_ARGS = $(ARM_CFLAGS) $(ARM_LDFLAGS) $(OKK_INC_PATH) $(OKK_LIB_PATH)
	dev_cc = $(ARM_CC)
endif

show:
	@echo "Set CROSS_TOOLCHAINS environment first by"
	@echo "    export CROSS_TOOLCHAINS=path/to/your/toolchains"
	@echo "  or"
	@echo "    source ./prepare_toolchains.sh"
	@echo "Please compile okkernel by:"
	@echo "    make okk"
	@echo "  or"
	@echo "    make okk USING_CMODEL=1"

src_dir := .

create_dir:
	@mkdir -p $(output_dir)

cpp_src   = $(wildcard $(src_dir)/host/*.cpp)
cpp_exe1  = $(patsubst $(src_dir)/host/%.cpp, %, $(cpp_src))
cpp_exe   = $(addprefix $(output_dir), $(cpp_exe1))
cpp_objs  = $(addsuffix .o, $(cpp_exe))
$(warning, $(cpp_exe))

cpp_dep   = $(patsubst %.o, %.d, $(cpp_objs)))

$(cpp_objs): $(output_dir)%.o:$(src_dir)/host/%.cpp
	@$(HOST_CXX) $(CPP_ARGS) -MMD -c $< -o $@

-include $(cpp_dep)

c_src   = $(wildcard $(src_dir)/device/*.c)
c_exe1  = $(patsubst $(src_dir)/device/%.c, %, $(c_src))
c_exe   = $(addprefix $(output_dir), $(c_exe1))
c_objs  = $(addsuffix .o, $(c_exe))

c_dep   = $(patsubst %.o, %.d, $(cpp_objs)))

$(c_objs): $(output_dir)%.o:$(src_dir)/device/%.c
	@echo "CC $^"
	@$(dev_cc) $(C_ARGS) -MMD -c $< -o $@

-include $(c_dep)

ifeq ($(USING_CMODEL),1)

$(cpp_exe): %:%.o $(c_objs)
	@echo "LD $@"
	@$(HOST_CXX) -o $@ $^ \
	-Wl,--start-group \
	$(CPP_ARGS)  \
	-lm -ldl \
	-Wl,--end-group

okk: create_dir $(cpp_exe)

else
$(cpp_exe): %:%.o
	@echo "LD $@"
	@$(HOST_CXX) -o $@ $^ \
	-Wl,--start-group \
	$(CPP_ARGS) \
	-lm -ldl \
	-Wl,--end-group

firmware: create_dir $(c_objs)
	@echo "LD $@"
	@$(ARM_AR) -rc $(output_dir)/okkernel_firmware.a  $(c_objs)
	@$(ARM_CC) -o $(output_dir)/$@ $(ARM_LDFLAGS) -T$(OKK_LINK) $(OKK_LIB_PATH) \
	-Wl,--start-group \
	-lc -lm $(c_objs) -l:$(FW_LIB) \
	-Wl,--end-group
	@$(ARM_OBJCOPY) -O binary -R *.slow* $(output_dir)/$@ \
	$(output_dir)/$@_tcm.bin
	@$(ARM_OBJCOPY) -O binary -j *.slow* $(output_dir)/$@ \
	$(output_dir)/$@_ddr.bin

okk: create_dir $(cpp_exe) firmware
endif

clean:
	@rm -rf build
