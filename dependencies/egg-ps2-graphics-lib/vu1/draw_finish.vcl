
; _____        ____   ___
;   |     \/   ____| |___|
;   |     |   |   \  |   |
;---------------------------------------------------------------
; Copyright 2022, tyra - https://github.com/h4570/tyra
; Licensed under Apache License 2.0
; Sandro Sobczyński <sandro.sobczynski@gmail.com>
;
;---------------------------------------------------------------
; Needed for synchronization with draw_wait_finish()
;---------------------------------------------------------------

.syntax new
.name vsmDrawFinish
.vu
.init_vf_all
.init_vi_all

--enter
--endenter

    xtop    buffer
    iaddiu  kickAddress,    buffer, 10
    xgkick  kickAddress

--exit
--endexit