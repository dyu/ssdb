#include "serv.h"
#include "net/proc.h"
#include "net/server.h"

int proc_getrange(NetworkServer *net, Link *link, const Request &req, Response *resp){
    JniContext *jni = resp->jni;
    int rpcId;
    if (req.size() < 4 || 0 >= (rpcId = req[2].Int())) {
        resp->push_back("Invalid request");
        return 0;
    }

    //printf("rpc: %d\n", rpcId);

    jni->env->CallStaticVoidMethod(jni->class_, jni->handle_, jni->type, jni->id);
    resp->push_back("[0]");
    return 0;
}

int proc_setrange(NetworkServer *net, Link *link, const Request &req, Response *resp){
    JniContext *jni = resp->jni;
    int rpcId;
    if (req.size() < 4 || 0 >= (rpcId = req[2].Int())) {
        resp->push_back("Invalid request");
        return 0;
    }

    //const Bytes &payload = req[3];

    //printf("rpc: %d\n", rpcId);

    jni->env->CallStaticVoidMethod(jni->class_, jni->handle_, jni->type, jni->id);
    resp->push_back("[0]");
    return 0;
}