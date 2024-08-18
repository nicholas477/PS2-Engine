EE_SRC_DIR = src/
EE_LIB_DIR = lib/
EE_OBJS_DIR = obj/

EE_INCS += -I./include -I$(PS2SDK)/ports/include -I$(EE_SRC_DIR)

EE_LIB = $(EE_LIB_DIR)libegg-ps2-graphics-lib.a
EE_OBJS = src/egg-ps2-graphics-lib.o src/mesh.o src/vu_programs.o src/types.o
EE_OPTFLAGS = -std=gnu++20 -g
EE_WARNFLAGS = -Wall -Wno-unused-function -Wno-unused-variable -Wno-strict-aliasing -Wno-conversion-null -Wno-unused-but-set-variable -Werror=return-type

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB)

$(EE_OBJS_DIR):
	mkdir -p $(EE_OBJS_DIR)

$(EE_LIB_DIR):
	mkdir -p $(EE_LIB_DIR)

$(EE_OBJS_DIR)%.o : $(EE_SRC_DIR)%.c
	$(EE_C_COMPILE) -DIOAPI_NO_64 -c $< -o $@

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)
	rm -rf ./src/*.o

include Makefile.vuprogs

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal