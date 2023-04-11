EE_BIN = main.elf
SRCS := $(wildcard *.cc)
EE_OBJS = main.o input.o stats.o engine.o world.o gs.o objects/movement.o objects/camera.o objects/teapot.o objects/components/transform_component.o types.o
EE_OBJS :=$(addprefix $(PREFIX), $(EE_OBJS))
EE_INCS := -Iinclude $(EE_INCS)
EE_LIBS = -ldraw -lgraph -lmath3d -lpacket -ldma -lpad -lc -lstdc++ -lgs -ldebug
EE_CFLAGS = -O3 -g

all: $(EE_BIN)
	$(EE_STRIP) --strip-all $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

run: $(EE_BIN)
	ps2client -h $(PS2HOST) execee host:$(EE_BIN)

reset:
	ps2client -h $(PS2HOST) reset