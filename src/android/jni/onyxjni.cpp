#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <android/log.h>
#include <jni.h>

#define LOG_TAG "com.mcntech.com"
static JavaVM *java_vm;

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	java_vm = vm;
}
