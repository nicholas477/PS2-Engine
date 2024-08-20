EE_BIN = build/iso/MAIN.00
ISO_TGT = $(OUTPUT_DIR)/test.iso
SRC_FILES = $(shell find $(PREFIX) -name "*.cc" -o -name "*.cpp" -o -name "*.c")
EE_OBJS = $(patsubst %.cc, build/%.o, $(SRC_FILES))
EE_OBJS := $(patsubst %.c, build/%.o, $(EE_OBJS))
EE_OBJS := $(patsubst %.cpp, build/%.o, $(EE_OBJS))
EE_INCS := -I$(PS2SDK)/ports/include -Iinclude -Idependencies/egg-ps2-graphics-lib/include $(EE_INCS)
EE_LDFLAGS  += -L$(PS2SDK)/ports/lib -Ldependencies/egg-ps2-graphics-lib/lib
EE_LIBS = -legg-ps2-graphics-lib -legg -ldraw -lgraph -lmath3d -ldma -lpad -lc -lstdc++ -lgs -ldebug -leedebug -laudsrv -lcdvd -lxcdvd -lkbd -lpacket2 
EE_OPTFLAGS = -g -std=gnu++20
EE_CFLAGS += -DNO_VU0_VECTORS -DNO_ASM -g
EE_CXXFLAGS += -DNO_VU0_VECTORS -DNO_ASM -frtti -g
EE_WARNFLAGS = -Wall -Wno-unused-function -Wno-unused-variable -Wno-strict-aliasing -Wno-conversion-null -Werror=return-type
LINKFLAGS	= -g -Wl,-zmax-page-size=128

OUTPUT_DIR     = build
ISO_FOLDER_DIR = build/iso

include src/Makefile.iso

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