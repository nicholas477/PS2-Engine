/*     Copyright (C) 2000,2001,2002  Sony Computer Entertainment America

            This file is subject to the terms and conditions of the GNU Lesser
       General Public License Version 2.1. See the file "COPYING" in the
       main directory of this archive for more details.                             */

     #include       "vertex_color_renderer_mem_linear.h"

     .include       "db_in_db_out.i"
     .include       "math.i"
     .include       "lighting.i"
     .include       "clip_cull.i"
     .include       "geometry.i"
     .include       "io.i"
     .include       "general.i"

	 .include		"teapot_math.i"

kInputQPerV         .equ           4
kOutputQPerV        .equ           3

     .init_vf_all
     .init_vi_all

     .name          vsmVertexColorRenderer

     --enter
     --endenter

     ; ------------------------ initialization ---------------------------------

     load_vert_xfrm vert_xform

     --cont

     ; -------------------- transform & texture loop ---------------------------

main_loop_lid:

     init_constants
     init_clip_cnst

     init_io_loop
     init_out_buf

     set_strip_adcs

     ; const_color = material emissive + global ambient
     get_cnst_color const_color

     init_bfc_strip

xform_loop_lid:          --LoopCS 1,3

     ; xform/clip vertex

     load_vert      vert

	; wind
	move.w			vert, vf00
    lq.xyzw        	time, kTime(vi00)
    ; add.xyzw		time, time, vert[y]
	move.xyzw		sincos, vf00
	AngleSinCos		sincos, time
	loi 32.0
	muli			sincos, sincos, i
    add.y       	vert, vert, time[x]

	--cont



     xform_vert     xformed_vert, vert_xform, vert
     vert_to_gs     gs_vert, xformed_vert

     load_strip_adc strip_adc
     bfc_strip_vert xformed_vert, strip_adc
     clip_vert      xformed_vert
     fcand          vi01, 0x003ffff
     iand           vi01, vi01, do_clipping
     set_adc_fbs    gs_vert, strip_adc

     store_xyzf     gs_vert

     ; constant color

     ; const_color = material emissive + global ambient
     load_pvcolor vert_color
     loi 255.0
     addi.w vert_color, vf00, i
     muli.xyz vert_color, vert_color, i
     store_rgba vert_color

     ;store_rgb const_color

     ; texture coords

     load_stq       tex_stq
     xform_tex_stq  tex_stq, tex_stq, q ; q is from normalize_3
     store_stq      tex_stq

     next_io
     loop_io        xform_loop_lid


     ; -------------------- done! -------------------------------

done_lid:

     ; clamp and convert to fixed-point
     ; reads vert_color
     finish_colors

     ; ---------------- kick packet to GS -----------------------

     kick_to_gs

     --cont

     b    main_loop_lid

.END ; for gasp
