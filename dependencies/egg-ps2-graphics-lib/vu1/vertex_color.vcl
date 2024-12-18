#include       "vu1_defs.h"

.syntax new
.name vsmVertexColor
.vu
.init_vf_all
.init_vi_all

--enter
in_vi retaddr(RETADDR_REG)
in_vi iBase(BASE_REG)
--endenter
    ;//////////// --- Load data --- /////////////
    ; Updated dynamically
    ;xtop    iBase

    ilw.w   vertCount,        SCALE(iBase)
    lq      rgba,             RGBA(iBase) ; RGBA mul
                                       ; u32 : R, G, B, A (0-128)

    ilw.z   compsPerPrim,       FOG(iBase)
    ilw.w   destOffset,         FOG(iBase) ; dest address offest (compsPerPrim * vertex count)

    iaddiu  vertexInPtr,      iBase,           VERTEXIN            ; pointer to vertex input data
    iadd    colorInPtr,       vertexInPtr,     vertCount    ; pointer to color input data
    iadd    kickAddress,      vertexInPtr,     destOffset   ; pointer for XGKICK
    iadd    vertexOutPtr,     kickAddress,     compsPerPrim ; pointer to first vert pos out

    ;/////////////// --- Loop --- ///////////////
    iadd vertexCounter, iBase, vertCount ; loop vertCount times
    vertexLoop:

        ;////////// --- Load loop data --- //////////
        lq.xyzw color, 0(colorInPtr) ; load color

        ;////////////// --- Color --- //////////////
        ; Color in the model is from 0-1, we need to convert it to 0-255 fixed point
        loi              255.0
        muli.xyzw        color, color, i
        ftoi0.xyzw       color, color
        
        ; VCL really likes to reorder the lines after this and break compilation
        ; so I added an instruction reordering barrier here
        --barrier

        ;//////////// --- Store data --- ////////////
        sq.xyzw  color,       -1(vertexOutPtr)      ; Color
        ;sq.xyzw  color,        0(vertexOutPtr)      ; XYZ2F
        ;////////////////////////////////////////////

        iaddiu        colorInPtr,        colorInPtr,      1
        iadd          vertexOutPtr,      vertexOutPtr,     compsPerPrim

        iaddi   vertexCounter,  vertexCounter,  -1	; decrement the loop counter 
        ibne    vertexCounter,  iBase,   vertexLoop	; and repeat if needed

    ;//////////////////////////////////////////// 

    RETURN

--exit
--endexit

RETADDR_DUMMY