#include "serv.h"
#include "net/proc.h"
#include "net/server.h"

int proc_getrange(NetworkServer *net, Link *link, const Request &req, Response *resp){
    JniContext *jni = resp->jni;
    CHECK_NUM_PARAMS(3);

    //int rpcId = req[2].Int();

    jni->env->CallStaticVoidMethod(jni->class_, jni->handle_, jni->type, jni->id);
    resp->push_back("getrange!");
    return 0;
}

int proc_setrange(NetworkServer *net, Link *link, const Request &req, Response *resp){
    JniContext *jni = resp->jni;
    CHECK_NUM_PARAMS(4);

    //int rpcId = req[2].Int();
    //const Bytes &payload = req[3];

    jni->env->CallStaticVoidMethod(jni->class_, jni->handle_, jni->type, jni->id);
    resp->push_back("setrange!");
    return 0;
}