#include       "vu1_defs.h"

.syntax new
.name vsmJumpTable
.vu
.init_vi BASE_REG, RETADDR_REG

--enter
--endenter

    xtop    BASE_REG

    ilw.x   addr, JUMPTABLE(BASE_REG)
    jalr    RETADDR_REG, addr:dummy

    ilw.y   addr, JUMPTABLE(BASE_REG)
    jalr    RETADDR_REG, addr:dummy

    ilw.z   addr, JUMPTABLE(BASE_REG)
    jalr    RETADDR_REG, addr:dummy

    ilw.w   addr, JUMPTABLE(BASE_REG)
    jalr    RETADDR_REG, addr:dummy

    ilw.x   addr, (JUMPTABLE + 1)(BASE_REG)
    jalr    RETADDR_REG, addr:dummy

    ilw.y   addr, (JUMPTABLE + 1)(BASE_REG)
    jalr    RETADDR_REG, addr:dummy

    ilw.z   addr, (JUMPTABLE + 1)(BASE_REG)
    jalr    RETADDR_REG, addr:dummy

    ilw.w   addr, (JUMPTABLE + 1)(BASE_REG)
    jalr    RETADDR_REG, addr:dummy

--exit
--endexit

; Dummy function so VCL doesnt complain about jumps out of this file
dummy:            
	jr RETADDR_REG:dummy