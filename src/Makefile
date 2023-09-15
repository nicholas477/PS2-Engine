EE_BIN = main.elf
ISO_TGT = $(OUTPUT_DIR)/test.iso
SRC_FILES = $(shell find $(PREFIX) -name "*.cc" -o -name "*.cpp" -o -name "*.c")
EE_OBJS = $(patsubst %.cc, build/%.o, $(SRC_FILES))
EE_OBJS := $(patsubst %.c, build/%.o, $(EE_OBJS))
EE_OBJS := $(patsubst %.cpp, build/%.o, $(EE_OBJS))
EE_INCS := -I$(PS2SDK)/ports/include -Iinclude $(EE_INCS)
EE_LDFLAGS  += -L$(PS2SDK)/ports/lib
EE_LIBS = -ldraw -lgraph -lmath3d -lpacket -ldma -lpad -lc -lstdc++ -lgs -ldebug -leedebug -lpacket2 -laudsrv -lcdvd -lxcdvd -lps2gl -lps2stuff
EE_OPTFLAGS = -O3 -g -std=gnu++17
EE_CFLAGS += -DNO_VU0_VECTORS -DNO_ASM
EE_CXXFLAGS += -DNO_VU0_VECTORS -DNO_ASM
EE_WARNFLAGS = -Wall -Wno-unused-function -Wno-unused-variable -Wno-strict-aliasing -Wno-conversion-null 
LINKFLAGS	= -O3 -Wl,-zmax-page-size=128

OUTPUT_DIR     = build
ISO_FOLDER_DIR = build/iso
VU_OUTPUT_DIR  = vu
VU_PROGS = $(addprefix $(OUTPUT_DIR)/$(VU_OUTPUT_DIR)/, $(patsubst %.vcl, %.o, $(VCL_FILES)))

#
# Vector Unit assembler:
#
PS2_VU_DVP = dvp-as

#
# openvcl = Vector Command Line preprocessor.
# (get it at: https://github.com/jsvennevid/openvcl)
#
# vclpp = Vector assembly preprocessor
# (get it at: https://github.com/glampert/vclpp)
PS2_VCL   = vcl
PS2_VCLPP = vclpp

#
# VCL/VU microprograms:
#
VCL_PATH  = $(PREFIX)vu1progs
VCL_FILES =

EE_OBJS := $(VU_PROGS) $(EE_OBJS)

include src/Makefile.iso

all: $(ISO_TGT)

#
# VU microprograms:
#
$(VU_PROGS): $(OUTPUT_DIR)/$(VU_OUTPUT_DIR)/%.o: $(OUTPUT_DIR)/$(VU_OUTPUT_DIR)/%.vsm
	$(PS2_VU_DVP) $< -o $@

$(OUTPUT_DIR)/$(VU_OUTPUT_DIR)/%.vsm:
	$(MKDIR) -p $(dir $@)
	$(PS2_VCLPP) $(VCL_PATH)/$*.vcl $(dir $@)$*.pp.vcl -j
	$(PS2_VCL) -o$@ $(dir $@)$*.pp.vcl

clean:
	rm -f $(EE_BIN)
	rm -rf $(OUTPUT_DIR)
	rm -rf $(ISO_TGT)

run: $(EE_BIN)
	ps2client -h $(PS2HOST) execee host:$(ISO_FOLDER_DIR)/$(EE_BIN)

reset:
	ps2client -h $(PS2HOST) reset