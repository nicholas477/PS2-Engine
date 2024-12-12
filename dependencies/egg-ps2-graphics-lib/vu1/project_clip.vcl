
.syntax new
.name vsmProjectClip
.vu
.init_vf_all
.init_vi_all

--enter
--endenter
    ;//////////// --- Load data --- /////////////
    ; Updated dynamically
    xtop    iBase

    lq      matrixRow0,     0(iBase) ; load view-projection matrix
    lq      matrixRow1,     1(iBase)
    lq      matrixRow2,     2(iBase)
    lq      matrixRow3,     3(iBase)

    lq.xyz  scale,            4(iBase) ; load program params
                                     ; float : X, Y, Z - scale vector that we will use to scale the verts after projecting them.
                                     ; float : W - vert count.
    ilw.w   vertCount,        4(iBase)
    lq      primTag,          5(iBase) ; GIF tag - tell GS how many data we will send and what type
    lq      rgba,             6(iBase) ; RGBA mul
                                       ; u32 : R, G, B, A (0-128)

    ;lq      fogSetting,        7(iBase) ; x = offset, y = scale
    ilw.z   compsPerPrim,       7(iBase)
    ilw.w   destOffset,         7(iBase) ; dest address offest (compsPerPrim * vertex count)

    iaddiu  vertexInPtr,      iBase,           8            ; pointer to vertex input data
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

        ; Perspective divide
        div         q,      vf00[w],    vertex[w]   ; perspective divide (1/vert[w]):
        mul.xyz     vertex, vertex,     q

        ; Scale to screen space
        mula.xyz    acc,    scale,      vf00[w]     ; scale to GS screen space
        madd.xyz    vertex, vertex,     scale       ; multiply and add the scales -> vert = vert * scale + scale
        ftoi4.xyz   vertex, vertex                  ; convert vertex to 12:4 fixed point format
        
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