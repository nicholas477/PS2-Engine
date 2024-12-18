#include       "vu1_defs.h"

.syntax new
.name vsmJumpTable
.vu
.init_vf_all
.init_vi_all

--enter
--endenter

    xtop    iBase
    ilw.x   addr, JUMPTABLE(iBase)
    jalr    RETADDR_REG, addr:dummy

    ilw.y   addr, JUMPTABLE(iBase)
    jalr    RETADDR_REG, addr:dummy

    ilw.z   addr, JUMPTABLE(iBase)
    jalr    RETADDR_REG, addr:dummy

    ilw.w   addr, JUMPTABLE(iBase)
    jalr    RETADDR_REG, addr:dummy


    ilw.x   addr, (JUMPTABLE + 1)(iBase)
    jalr    RETADDR_REG, addr:dummy

    ilw.y   addr, (JUMPTABLE + 1)(iBase)
    jalr    RETADDR_REG, addr:dummy

    ilw.z   addr, (JUMPTABLE + 1)(iBase)
    jalr    RETADDR_REG, addr:dummy

    ilw.w   addr, (JUMPTABLE + 1)(iBase)
    jalr    RETADDR_REG, addr:dummy

--exit
--endexit

; Dummy function so VCL doesn't complain about jumps out of this file
dummy:            
	jr RETADDR_REG:dummy