#include <stdio.h>
#include "NativeAdd.h"
#include "myAdd.h"


JNIEXPORT jint JNICALL Java_NativeAdd_add
(JNIEnv *env, jobject obj, jint x, jint y)
{
	return myAdd(x, y);
	//return x + y;
}