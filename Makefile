# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

PS2HOST?=10.0.6.50

PREFIX=src/

include src/Makefile

include $(PS2SDK)/samples/Makefile.pref
include Makefile.eeglobal_cpp
