#include "serv.h"
#include "net/proc.h"
#include "net/server.h"

inline void
write16(char* buf, uint16_t value)
{
    *(uint16_t*)(buf) = value;
}

inline void
write32(char* buf, uint32_t value)
{
    *(uint32_t*)(buf) = value;
}

inline void
write64(char* buf, uint64_t value)
{
    *(uint64_t*)(buf) = value;
}

inline uint16_t read16(char* buf)
{
    return *(uint16_t*)(buf);
}

static char* write(char* buf, const Bytes& header, const Bytes& payload)
{
    auto headerSize = header.size(),
        payloadSize = payload.size();
    
    write16(buf, headerSize);
    buf += 2;
    write16(buf, payloadSize);
    buf += 2;

    memcpy(buf, header.data(), headerSize);
    buf += headerSize;
    memcpy(buf, payload.data(), payloadSize);
    buf += payloadSize;

    return buf;
}

static int handle_rpc(NetworkServer *net, Link *link, const Request &req, Response *resp)
{
    JniContext *jni = resp->jni;
    if (req.size() < 4) {
        resp->push_back("Invalid request");
        return 0;
    }

    //printf("key: %.*s\n", req[1].size(), req[1].data());

    char *buf = write(jni->buf, req[2], req[3]);

    jni->env->CallStaticVoidMethod(jni->class_, jni->handle_, jni->type, jni->id);

    auto size = read16(buf);
    resp->push_back(std::string(buf + 2, size));
    return 0;
}

int proc_getrange(NetworkServer *net, Link *link, const Request &req, Response *resp){
    return handle_rpc(net, link, req, resp);
}

int proc_setrange(NetworkServer *net, Link *link, const Request &req, Response *resp){
    return handle_rpc(net, link, req, resp);
}