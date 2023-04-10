EE_BIN = main.elf
EE_OBJS = main.o input.o engine.o gs.o objects/camera.o objects/teapot.o objects/components/transform_component.o
EE_OBJS:=$(addprefix $(PREFIX), $(EE_OBJS))
EE_LIBS = -ldraw -lgraph -lmath3d -lpacket -ldma -lpad -lc -lstdc++ -lgs

all: $(EE_BIN)
	$(EE_STRIP) --strip-all $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

run: $(EE_BIN)
	ps2client -h $(PS2HOST) execee host:$(EE_BIN)

reset:
	ps2client -h $(PS2HOST) reset