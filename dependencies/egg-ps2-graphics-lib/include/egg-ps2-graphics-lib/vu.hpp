#pragma once

#define VU_FUNCTIONS(name)        \
	void vsm##name##_CodeStart(); \
	void vsm##name##_CodeEnd()

#define mVsmStartAddr(name) ((void*)vsm##name##_CodeStart)

#define mVsmEndAddr(name) ((void*)vsm##name##_CodeEnd)

#define mVsmSize(name) ((u8*)vsm##name##_CodeEnd - (u8*)vsm##name##_CodeStart)