EE_BIN = build/iso/MAIN.00
ISO_TGT = $(OUTPUT_DIR)/test.iso
SRC_FILES = $(shell find $(PREFIX) -name "*.cc" -o -name "*.cpp" -o -name "*.c")
#SRC_FILES = src/main.cc
EE_OBJS = $(patsubst %.cc, build/%.o, $(SRC_FILES))
EE_OBJS := $(patsubst %.c, build/%.o, $(EE_OBJS))
EE_OBJS := $(patsubst %.cpp, build/%.o, $(EE_OBJS))
EE_INCS := -I$(PS2SDK)/ports/include -Iinclude -Ivu1 $(EE_INCS)
EE_LDFLAGS  += -L$(PS2SDK)/ports/lib
#EE_LIBS = -ldebug -lpatches -lxcdvd 
EE_LIBS = -ldebug -lpatches -lxcdvd -lnetman -lps2gl -lps2stuff -legg -lps2ip -ldraw -lgraph -lmath3d -ldma -lpad -lc -lstdc++ -lgs -ldebug -leedebug -laudsrv -lcdvd -lxcdvd -lkbd
EE_OPTFLAGS = -O3 -g -std=gnu++20
EE_CFLAGS += -DNO_VU0_VECTORS -DNO_ASM
EE_CXXFLAGS += -DNO_VU0_VECTORS -DNO_ASM -frtti
EE_WARNFLAGS = -Wall -Wno-unused-function -Wno-unused-variable -Wno-strict-aliasing -Wno-conversion-null 
LINKFLAGS	= -O3 -g -Wl,-zmax-page-size=128

OUTPUT_DIR     = build
ISO_FOLDER_DIR = build/iso
VU_OUTPUT_DIR  = vu
VU_PROGS = $(addprefix $(OUTPUT_DIR)/$(VU_OUTPUT_DIR)/, $(patsubst %.vcl, %.o, $(VCL_FILES)))

PS2GL_DIR = dependencies/ps2gl
EE_INCS += -I$(PS2GL_DIR)/vu1

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
VCL_PATH  = $(PREFIX)
VCL_FILES = renderer/ps2gl_renderers/vuprogs/general_pv_diff.vcl

include src/Makefile.iso
include src/Makefile.vuprogs

VUPROGS: $(addsuffix .vo, $(addprefix vu1/, $(RENDERERS)))
EE_OBJS := $(VUPROGS) $(EE_OBJS)

.PHONY: compile cleancompile
compile: $(EE_BIN)

cleancompile:
	rm -f $(EE_BIN)
	rm -rf $(OUTPUT_DIR)

clean: cleaniso cleancompile

realclean: clean
	rm -f  $(VSM_SOURCES)

all: $(ISO_TGT)

run: $(EE_BIN)
	cd $(dir $(EE_BIN))
	ps2client -h $(PS2HOST) execee host:$(notdir $(EE_BIN))

reset:
	ps2client -h $(PS2HOST) reset