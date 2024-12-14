
.syntax new
.name vsmVertexColor
.vu
.init_vf_all
.init_vi_all

--enter
--endenter
	fcset   0x000000	; VCL won't let us use CLIP without first zeroing
				        ; the clip flags

    ;//////////// --- Load data --- /////////////
    ; Updated dynamically
    xtop    iBase

    ; lq      matrixRow0,     0(iBase) ; load view-projection matrix
    ; lq      matrixRow1,     1(iBase)
    ; lq      matrixRow2,     2(iBase)
    ; lq      matrixRow3,     3(iBase)

    lq.xyz  scale,            4(iBase) ; load program params
                                     ; float : X, Y, Z - scale vector that we will use to scale the verts after projecting them.
                                     ; float : W - vert count.
    ilw.w   vertCount,        4(iBase)
    ;lq      primTag,          5(iBase) ; GIF tag - tell GS how many data we will send and what type
    lq      rgba,             6(iBase) ; RGBA mul
                                       ; u32 : R, G, B, A (0-128)

    ;lq      fogSetting,        7(iBase) ; x = offset, y = scale
    ilw.z   compsPerPrim,       7(iBase)
    ilw.w   destOffset,         7(iBase) ; dest address offest (compsPerPrim * vertex count)

    iaddiu  vertexInPtr,      iBase,           8            ; pointer to vertex input data
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
        
        ;//////////// --- Store data --- ////////////
        sq.xyzw  color,       -1(vertexOutPtr)      ; Color
        ;sq.xyzw vertex,      0(vertexOutPtr)      ; XYZ2F
        ;////////////////////////////////////////////

        ; VCL really likes to reorder the lines after this and break compilation
        ; so I added an instruction reordering barrier here
        --barrier

        iaddiu        colorInPtr,        colorInPtr,      1
        iadd          vertexOutPtr,      vertexOutPtr,     compsPerPrim

        iaddi   vertexCounter,  vertexCounter,  -1	; decrement the loop counter 
        ibne    vertexCounter,  iBase,   vertexLoop	; and repeat if needed

    ;//////////////////////////////////////////// 


    xgkick kickAddress ; dispatch to the GS rasterizer.

--exit
--endexit