; _____     ___ ____     ___ ____
;  ____|   |    ____|   |        | |____|
; |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
;-----------------------------------------------------------------------
; (c) 2020 h4570 Sandro Sobczyński <sandro.sobczynski@gmail.com>
; Licenced under Academic Free License version 2.0
; Review ps2sdk README & LICENSE files for further details.
;
;
;---------------------------------------------------------------
; draw_3D.vcl                                                   |
;---------------------------------------------------------------
; A VU1 microprogram to draw 3D object using XYZ2, RGBAQ and ST|
; This program uses double buffering (xtop)                    |
;                                                              |
; Many thanks to:                                              |
; - Dr Henry Fortuna                                           |
; - Jesper Svennevid, Daniel Collin                            |
; - Guilherme Lampert                                          |
;---------------------------------------------------------------

.macro   matrixMultiplyVertex vertexResult,matrix,vertexIn
  mul            acc,           \matrix[0], \vertexIn[x]
  madd           acc,           \matrix[1], \vertexIn[y]
  madd           acc,           \matrix[2], \vertexIn[z]
  madd           \vertexResult, \matrix[3], vf00[w]
.endm

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

    lq      matrixRow[0],     0(iBase) ; load view-projection matrix
    lq      matrixRow[1],     1(iBase)
    lq      matrixRow[2],     2(iBase)
    lq      matrixRow[3],     3(iBase)

    lq.xyz  scale,            4(iBase) ; load program params
                                     ; float : X, Y, Z - scale vector that we will use to scale the verts after projecting them.
                                     ; float : W - vert count.
    ilw.w   vertCount,        4(iBase)
    lq      primTag,          5(iBase) ; GIF tag - tell GS how many data we will send
    lq      rgba,             6(iBase) ; RGBA
                                       ; u32 : R, G, B, A (0-128)
    iaddiu  vertexData,     iBase,         7           ; pointer to vertex data
    iadd    kickAddress,    vertexData,    vertCount   ; pointer for XGKICK
    iadd    destAddress,    vertexData,    vertCount   ; helper pointer for data inserting
    ;////////////////////////////////////////////

    ;/////////// --- Store tags --- /////////////
    sqi primTag,    (destAddress++) ; prim + tell gs how many data will be
    ;////////////////////////////////////////////

    ;/////////////// --- Loop --- ///////////////
    iadd vertexCounter, iBase, vertCount ; loop vertCount times
    vertexLoop:

        ;////////// --- Load loop data --- //////////
        lq.xyzw vertex, 0(vertexData) ; load xyz
                                     ; float : X, Y, Z
                                     ; any32 : _ = 0


        ;////////////// --- Vertex --- //////////////
        matrixMultiplyVertex vertex, matrixRow, vertex
       
        clipw.xyz	vertex, vertex			; Dr. Fortuna: This instruction checks if the vertex is outside
							; the viewing frustum. If it is, then the appropriate
							; clipping flags are set
        fcand		vi01,   0x003ffff       ; Bitwise AND the clipping flags with 0x3FFFF, this makes
							; sure that we get the clipping judgement for the last three
							; verts (i.e. that make up the triangle we are about to draw)
        iaddiu		iClipBit,   vi01,       0x7FFF      ; Add 0x7FFF. If any of the clipping flags were set this will
							; cause the triangle not to be drawn (any values above 0x8000
							; that are stored in the w component of XYZ2 will set the ADC
							; bit, which tells the GS not to perform a drawing kick on this
							; triangle.
        
        div         q,      vf00[w],    vertex[w]   ; perspective divide (1/vert[w]):
        mul.xyz     vertex, vertex,     q
        mula.xyz    acc,    scale,      vf00[w]     ; scale to GS screen space
        madd.xyz    vertex, vertex,     scale       ; multiply and add the scales -> vert = vert * scale + scale
        ftoi4.xyz   vertex, vertex                  ; convert vertex to 12:4 fixed point format
        
        ;//////////// --- Store data --- ////////////
        sq rgba,            0(destAddress)      ; RGBA
        sq.xyz vertex,      1(destAddress)      ; XYZ2
        isw.w	iClipBit,   1(destAddress)
        ;////////////////////////////////////////////

        iaddiu          vertexData,     vertexData,     1                         
        iaddiu          destAddress,    destAddress,    2

        iaddi   vertexCounter,  vertexCounter,  -1	; decrement the loop counter 
        ibne    vertexCounter,  iBase,   vertexLoop	; and repeat if needed

    ;//////////////////////////////////////////// 

    --barrier

    xgkick kickAddress ; dispatch to the GS rasterizer.

--exit
--endexit
.END