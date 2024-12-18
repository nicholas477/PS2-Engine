#include       "vu1_defs.h"

.syntax new
.name vsmxgkick
.vu
.init_vf_all
.init_vi_all

--enter
in_vi iBase(BASE_REG)
--endenter

    ilw.w   destOffset,       FOG(iBase) ; dest address offest (compsPerPrim * vertex count)
    iaddiu  vertexInPtr,      iBase,           VERTEXIN     ; pointer to vertex input data
    iadd    kickAddress,      vertexInPtr,     destOffset   ; pointer for XGKICK

    --barrier
    
    xgkick  kickAddress

--exit
--endexit