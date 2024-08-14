#pragma once

#ifndef VU_FUNCTIONS
#define VU_FUNCTIONS(name)        \
	void vsm##name##_CodeStart(); \
	void vsm##name##_CodeEnd()
#endif

#ifndef mVsmStartAddr
#define mVsmStartAddr(name) ((void*)vsm##name##_CodeStart)
#endif

#ifndef mVsmEndAddr
#define mVsmEndAddr(name) ((void*)vsm##name##_CodeEnd)
#endif

#ifndef mVsmSize
#define mVsmSize(name) ((u8*)vsm##name##_CodeEnd - (u8*)vsm##name##_CodeStart)
#endif