;//--------------------------------------------------------------------
;// AngleSinCos - Returns the sin and cos of up to 2 angles, which must
;// be contained in the X and Z elements of "angle".  The sin/cos pair
;// will be contained in the X/Y elements of "sincos" for the first
;// angle, and Z/W for the second one.
;// Thanks to Colin Hughes (SCEE) for that one
;//
;// Note: ACC and i registers are modified, and a bunch of temporary
;//       variables are created... Maybe bad for VCL register pressure
;//--------------------------------------------------------------------
   .macro         AngleSinCos sincos, angle
   move.xz        \sincos, \angle                           ; To avoid modifying the original angles...

   mul.w          \sincos, vf00, \sincos[z]                 ; Copy angle from z to w
   add.y          \sincos, vf00, \sincos[x]                 ; Copy angle from x to y

   loi            1.570796                                  ; Phase difference for sin as cos ( PI/2 )
   subi.xz         \sincos, \sincos, i                       ;

   abs            \sincos, \sincos                          ; Mirror cos around zero

   max.xyzw       Vector1111, vf00, vf00[w]                 ; Initialise all 1s

   loi            -0.159155                                 ; Scale so single cycle is range 0 to -1 ( *-1/2PI )
   mulai.xyzw      acc,  \sincos, i                          ;

   loi            12582912.0                                ; Apply bias to remove fractional part
   msubai           acc,  Vector1111, i                       ;
   maddai           acc,  Vector1111, i                       ; Remove bias to leave original int part

   loi            -0.159155                                 ; Apply original number to leave fraction range only
   msubai           acc,  \sincos, i                          ;

   loi            0.5                                       ; Ajust range: -0.5 to +0.5
   msubai           \sincos, Vector1111, i                    ;

   abs            \sincos, \sincos                          ; Clamp: 0 to +0.5

   loi            0.25                                      ; Ajust range: -0.25 to +0.25
   subi            \sincos, \sincos, i                       ;

   mul            anglepower2, \sincos, \sincos             ; a^2

   loi            -76.574959                                ;
   muli            k4angle, \sincos, i                       ; k4 a

   loi            -41.341675                                ;
   muli            k2angle, \sincos, i                       ; k2 a

   loi            81.602226                                 ;
   muli            k3angle, \sincos, i                       ; k3 a

   mul            anglepower4, anglepower2, anglepower2     ; a^4
   mul            k4angle, k4angle, anglepower2             ; k4 a^3
   mula            acc,  k2angle, anglepower2                ; + k2 a^3

   loi            39.710659                                 ; k5 a
   muli            k2angle, \sincos, i                       ;

   mul            anglepower8, anglepower4, anglepower4     ; a^8
   madda           acc,  k4angle, anglepower4                ; + k4 a^7
   madda           acc,  k3angle, anglepower4                ; + k3 a^5
   loi            6.283185                                  ;
   maddai         acc,  \sincos, i                          ; + k1 a
   madd           \sincos, k2angle, anglepower8             ; + k5 a^9
   .endm