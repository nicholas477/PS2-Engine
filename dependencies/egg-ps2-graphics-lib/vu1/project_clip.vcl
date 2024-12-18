#include       "vu1_mem.h"

.syntax new
.name vsmProjectClip
.vu
.init_vf_all
.init_vi_all

--enter
--endenter
	fcset   0x000000	; VCL wont let us use CLIP without first zeroing
				        ; the clip flags

    ;//////////// --- Load data --- /////////////
    ; Updated dynamically
    xtop    iBase

    lq      matrixRow0,     MATRIXROW0(iBase) ; load view-projection matrix
    lq      matrixRow1,     MATRIXROW1(iBase)
    lq      matrixRow2,     MATRIXROW2(iBase)
    lq      matrixRow3,     MATRIXROW3(iBase)

    lq.xyz  scale,            SCALE(iBase)
    ilw.w   vertCount,        SCALE(iBase)
    lq      primTag,          PRIMTAG(iBase) ; GIF tag - tell GS how many data we will send and what type
    lq      rgba,             RGBA(iBase) ; RGBA mul
                                       ; u32 : R, G, B, A (0-128)

    ;lq      fogSetting,        FOG(iBase) ; x = offset, y = scale
    ilw.z   compsPerPrim,       FOG(iBase)
    ilw.w   destOffset,         FOG(iBase) ; dest address offest (compsPerPrim * vertex count)

    iaddiu  vertexInPtr,      iBase,           VERTEXIN            ; pointer to vertex input data
    iadd    kickAddress,      vertexInPtr,     destOffset   ; pointer for XGKICK
    iadd    vertexOutPtr,     kickAddress,     compsPerPrim ; pointer to first vert pos out
    ;////////////////////////////////////////////

    ;/////////// --- Store tags --- /////////////
    sq primTag,    0(kickAddress) ; prim + tell gs how many data will be
    ;////////////////////////////////////////////

    ;/////////////// --- Loop --- ///////////////
    iadd vertexCounter, iBase, vertCount ; loop vertCount times
    vertexLoop:

        ;////////// --- Load loop data --- //////////
        lq.xyz vertex, 0(vertexInPtr) ; load xyz
                                      ; float : X, Y, Z
                                      ; any32 : _ = 0

        ;////////////// --- Vertex --- //////////////
        mul            acc,           matrixRow0, vertex[x]
        madd           acc,           matrixRow1, vertex[y]
        madd           acc,           matrixRow2, vertex[z]
        madd           vertex,        matrixRow3, vf00[w]

        ; Clipping
        clipw.xyz	vertex, vertex			; Dr. Fortuna: This instruction checks if the vertex is outside
							; the viewing frustum. If it is, then the appropriate
							; clipping flags are set
        fcand		VI01,   0x3FFFF       ; Bitwise AND the clipping flags with 0x3FFFF, this makes
							; sure that we get the clipping judgement for the last three
							; verts (i.e. that make up the triangle we are about to draw)
        iaddiu		adcBit,   VI01,       0x7FFF      ; Add 0x7FFF. If any of the clipping flags were set this will
							; cause the triangle not to be drawn (any values above 0x8000
							; that are stored in the w component of XYZ2 will set the ADC
							; bit, which tells the GS not to perform a drawing kick on this
							; triangle.

        ; Perspective divide
        div         q,      vf00[w],    vertex[w]   ; perspective divide (1/vert[w]):
        mul.xyz     vertex, vertex,     q

        ; Scale to screen space
        mula.xyz    acc,    scale,      vf00[w]     ; scale to GS screen space
        madd.xyz    vertex, vertex,     scale       ; multiply and add the scales -> vert = vert * scale + scale
        ftoi4.xyz   vertex, vertex                  ; convert vertex to 12:4 fixed point format

        ; Add clipping bit
        mfir.w      vertex, adcBit
        
        ;//////////// --- Store data --- ////////////
        sq.xyzw rgba,       -1(vertexOutPtr)      ; Color
        sq.xyzw vertex,      0(vertexOutPtr)      ; XYZ2F
        ;////////////////////////////////////////////

        ; VCL really likes to reorder the lines after this and break compilation
        ; so I added an instruction reordering barrier here
        --barrier

        iaddiu          vertexInPtr,       vertexInPtr,      1
        iadd          vertexOutPtr,      vertexOutPtr,     compsPerPrim

        iaddi   vertexCounter,  vertexCounter,  -1	; decrement the loop counter 
        ibne    vertexCounter,  iBase,   vertexLoop	; and repeat if needed

    ;//////////////////////////////////////////// 


    xgkick kickAddress ; dispatch to the GS rasterizer.

--exit
--endexit