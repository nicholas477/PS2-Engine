.syntax new
.name vsmxgkick
.vu
.init_vf_all
.init_vi_all

--enter
--endenter

    xtop    iBase
    ilw.w   destOffset,       7(iBase) ; dest address offest (compsPerPrim * vertex count)
    iaddiu  vertexInPtr,      iBase,           8            ; pointer to vertex input data
    iadd    kickAddress,      vertexInPtr,     destOffset   ; pointer for XGKICK

    --barrier
    
    xgkick  kickAddress

--exit
--endexit