
.syntax new
.name vsmVertexColorRenderer
.vu
.init_vf_all
.init_vi_all

--enter
--endenter

    ;/////////////////////////////////////////////

	fcset   0x000000	; VCL won't let us use CLIP without first zeroing
				     ; the clip flags
    

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
    lq      primTag,          5(iBase) ; GIF tag - tell GS how many data we will send
    lq      rgba,             6(iBase) ; RGBA
                                       ; u32 : R, G, B, A (0-128)

    lq      fogSetting,        7(iBase) ; x = offset, y = scale

    iaddiu  vertexData,     iBase,         8            ; pointer to vertex data
    iadd    colorData,      vertexData,    vertCount   ; pointer to color data
    iadd    kickAddress,    colorData,     vertCount   ; pointer for XGKICK
    iadd    destAddress,    colorData,     vertCount   ; helper pointer for data inserting
    ;////////////////////////////////////////////

    ;/////////// --- Store tags --- /////////////
    sqi primTag,    (destAddress++) ; prim + tell gs how many data will be
    ;////////////////////////////////////////////

    ;/////////////// --- Loop --- ///////////////
    iadd vertexCounter, iBase, vertCount ; loop vertCount times
    vertexLoop:

        ;////////// --- Load loop data --- //////////
        lq.xyz vertex, 0(vertexData) ; load xyz
                                     ; float : X, Y, Z
                                     ; any32 : _ = 0

        lq.xyzw       color,     0(colorData)
        ;move.w        color,     vf00[w]

        ;////////////// --- Color --- //////////////
        ; Color in the model is from 0-1, we need to convert it to 0-255 fixed point
        loi              255.0
        muli.xyzw        color, color, i
        ftoi0.xyzw       color, color


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

        ; Fog mask (why is this like this? strange...)
        iaddiu fogMask, vi00, 0x7fff
        iaddiu fogMask, fogMask, 1
        iand   adcBit, fogMask, adcBit


        ; Perspective divide
        div         q,      vf00[w],    vertex[w]   ; perspective divide (1/vert[w]):
        mul.xyz     vertex, vertex,     q



        ; Fog
        muly.w  fog, vertex, fogSetting    ; multiply the vertex's z by the fog scale (fogSetting[y])
        addx.w  fog, fog,    fogSetting    ; add the fog start offset (fogSetting[x])

        ; Clamp fog from 0-255
        loi 255.0
        minii.w fog, fog,    i
        maxx.w  fog, fog,    vf00

        ; Convert the fog to an integer, move it to an integer register (fogColor)
        ftoi4.w fog, fog
        mtir    fogColor, fog[w]

        ; Combine clipping + fog settings
        ior     adcBit, adcBit, fogColor



        ; Scale to screen space
        mula.xyz    acc,    scale,      vf00[w]     ; scale to GS screen space
        madd.xyz    vertex, vertex,     scale       ; multiply and add the scales -> vert = vert * scale + scale
        ftoi4.xyz   vertex, vertex                  ; convert vertex to 12:4 fixed point format

        ; Add clipping bit
        mfir.w      vertex, adcBit

        
        ;//////////// --- Store data --- ////////////
        sq.xyzw color,       0(destAddress)
        sq.xyzw vertex,      1(destAddress)      ; XYZ2
        ;////////////////////////////////////////////

        iaddiu          vertexData,     vertexData,     1
        iaddiu          colorData,      colorData,      1
        iaddiu          destAddress,    destAddress,    2

        iaddi   vertexCounter,  vertexCounter,  -1	; decrement the loop counter 
        ibne    vertexCounter,  iBase,   vertexLoop	; and repeat if needed

    ;//////////////////////////////////////////// 

    --barrier

    xgkick kickAddress ; dispatch to the GS rasterizer.

--exit
--endexit