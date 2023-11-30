EE_SRC_DIR = src/
#EE_OBJS_DIR = obj/
EE_LIB_DIR = lib/

EE_INCS += -I./include -I$(EE_SRC_DIR)

EE_LIB = $(EE_LIB_DIR)libegg.a
EE_OBJS = src/egg.o src/filesystem.o src/asset.o
EE_OPTFLAGS = -O3 -std=gnu++20
EE_WARNFLAGS = -Wall -Wno-unused-function -Wno-unused-variable -Wno-strict-aliasing -Wno-conversion-null -Wno-unused-but-set-variable

all: $(EE_OBJS_DIR) $(EE_LIB_DIR) $(EE_LIB)

$(EE_OBJS_DIR):
	mkdir -p $(EE_OBJS_DIR)

$(EE_LIB_DIR):
	mkdir -p $(EE_LIB_DIR)

$(EE_OBJS_DIR)%.o : $(EE_SRC_DIR)%.c
	$(EE_C_COMPILE) -DIOAPI_NO_64 -c $< -o $@

install: all
	mkdir -p $(DESTDIR)$(PS2SDK)/ports/include/egg/
	mkdir -p $(DESTDIR)$(PS2SDK)/ports/lib
	cp -f $(EE_LIB) $(DESTDIR)$(PS2SDK)/ports/lib
	cp -f include/egg/*.hpp $(DESTDIR)$(PS2SDK)/ports/include/egg/

clean:
	rm -f -r $(EE_OBJS_DIR) $(EE_LIB_DIR)
	rm -rf ./src/*.o

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal