EE_BIN = main.elf
SRC_FILES = $(shell find $(PREFIX) -name "*.cc")
#EE_OBJS = main.o input.o stats.o engine.o world.o gs.o objects/movement.o objects/camera.o objects/teapot.o objects/components/transform_component.o types.o
#EE_OBJS :=$(addprefix $(PREFIX), $(EE_OBJS))
EE_INCS := -Iinclude $(EE_INCS)
EE_LIBS = -ldraw -lgraph -lmath3d -lpacket -ldma -lpad -lc -lstdc++ -lgs -ldebug
EE_CFLAGS = -O3 -g

OUTPUT_DIR     = build
# IOP_OUTPUT_DIR = irx
VU_OUTPUT_DIR  = vu
VU_PROGS = $(addprefix $(OUTPUT_DIR)/$(VU_OUTPUT_DIR)/, $(patsubst %.vcl, %.o, $(VCL_FILES)))

MKDIR_CMD = mkdir -p

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

EE_OBJS = $(patsubst %.cc, %.o, $(SRC_FILES))

#
# VCL/VU microprograms:
#
VCL_PATH  = $(PREFIX)vu1progs
VCL_FILES = color_triangles_clip_tris.vcl

all: $(EE_BIN) $(VU_PROGS)
	$(EE_STRIP) --strip-all $(EE_BIN)

#
# VU microprograms:
#
$(VU_PROGS): $(OUTPUT_DIR)/$(VU_OUTPUT_DIR)/%.o: $(OUTPUT_DIR)/$(VU_OUTPUT_DIR)/%.vsm
	$(ECHO_ASSEMBLING_VU_PROGS)
	$(QUIET) $(PS2_VU_DVP) $< -o $@

$(OUTPUT_DIR)/$(VU_OUTPUT_DIR)/%.vsm:
	$(MKDIR_CMD) $(dir $@)
	$(PS2_VCLPP) $(VCL_PATH)/$*.vcl $(dir $@)$*.pp.vcl -j
	$(PS2_VCL) -o$@ $(dir $@)$*.pp.vcl

clean:
	rm -f $(EE_BIN) $(EE_OBJS)
	rm -rf $(OUTPUT_DIR)/$(VU_OUTPUT_DIR)

run: $(EE_BIN)
	ps2client -h $(PS2HOST) execee host:$(EE_BIN)

reset:
	ps2client -h $(PS2HOST) reset