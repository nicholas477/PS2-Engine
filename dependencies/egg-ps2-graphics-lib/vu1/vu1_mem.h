#pragma once

#define MATRIXROW0 0
#define MATRIXROW1 (MATRIXROW0 + 1)
#define MATRIXROW2 (MATRIXROW1 + 1)
#define MATRIXROW3 (MATRIXROW2 + 1)

#define SCALE (MATRIXROW3 + 1)

#define PRIMTAG (SCALE + 1)
#define RGBA (PRIMTAG + 1)

#define FOG (RGBA + 1)

// Table of programs for the VU to run on the data
#define JUMPTABLE (FOG + 1)

#define VERTEXIN (JUMPTABLE + 2)
