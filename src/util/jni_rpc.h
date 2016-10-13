#ifndef UTIL_JNI_RPC_H_
#define UTIL_JNI_RPC_H_

#include <jni.h>

struct JniContext {
    int id;
    int type;
    JNIEnv *env;
    jclass class_;
    jmethodID handle_;
    char* buf;
};

#endif
