EE_BIN = main.elf
SRC_FILES = $(shell find $(PREFIX) -name "*.cc" -o -name "*.cpp" -o -name "*.c")
EE_OBJS = $(patsubst %.cc, build/%.o, $(SRC_FILES))
EE_OBJS := $(patsubst %.c, build/%.o, $(EE_OBJS))
EE_OBJS := $(patsubst %.cpp, build/%.o, $(EE_OBJS))
EE_INCS := -Iinclude $(EE_INCS)
EE_LIBS = -ldraw -lgraph -lmath3d -lpacket -ldma -lpad -lc -lstdc++ -lgs -ldebug -leedebug -lpacket2
EE_OPTFLAGS = -O3 -g -std=gnu++17
EE_WARNFLAGS = -Wall -Wno-unused-function -Wno-unused-variable
LINKFLAGS	= -O3 -Wl,-zmax-page-size=128

OUTPUT_DIR     = build
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
VCL_FILES = color_triangles_clip_tris.vcl color_triangles_clip_verts.vcl draw_finish.vcl

EE_OBJS := $(VU_PROGS) $(EE_OBJS)

all: $(EE_BIN)

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

run: $(EE_BIN)
	ps2client -h $(PS2HOST) execee host:$(EE_BIN)

reset:
	ps2client -h $(PS2HOST) reset