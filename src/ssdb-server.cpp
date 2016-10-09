/*
Copyright (c) 2012-2015 The SSDB Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
*/
#include "include.h"
#include "version.h"
#include "net/server.h"
#include "ssdb/ssdb.h"
#include "util/app.h"
#include "serv.h"
#include <jni.h>

/*extern "C" {
    #include <dlfcn.h>
}
typedef jint (*jni_createvm_pt)(JavaVM **pvm, void **penv, void *args);
*/

#define APP_NAME "ssdb-server"
#define APP_VERSION SSDB_VERSION

const std::string dot_jar = ".jar";

inline bool ends_with(std::string const & value, std::string const & ending)
{
    return ending.size() <= value.size() && std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

class MyApplication : public Application
{
public:
    SSDB *data_db;
    SSDB *meta_db;
    NetworkServer *net;
    SSDBServer *server;
    bool serve();
    virtual void usage(int argc, char **argv);
    virtual void welcome();
    virtual void run();
private:
    int main_class_offset;
    JNIEnv *jvm_env;
    JavaVM *jvm;
    void destroy(bool success);
    bool init_jvm();
    void destroy_jvm();
};

void MyApplication::welcome(){
	//fprintf(stderr, "%s %s\n", APP_NAME, APP_VERSION);
	//fprintf(stderr, "Copyright (c) 2012-2015 ssdb.io\n");
	//fprintf(stderr, "\n");
}

void MyApplication::usage(int argc, char **argv){
	printf("Usage:\n");
	printf("    %s [-d] /path/to/ssdb.conf [-s start|stop|restart]\n", argv[0]);
	printf("Options:\n");
	printf("    -d    run as daemon\n");
	printf("    -s    option to start|stop|restart the server\n");
	printf("    -h    show this message\n");
}

void MyApplication::run(){
	Options option;
	option.load(*conf);

	std::string data_db_dir = app_args.work_dir + "/data";
	std::string meta_db_dir = app_args.work_dir + "/meta";

	log_info("ssdb-server %s", APP_VERSION);
	log_info("conf_file        : %s", app_args.conf_file.c_str());
	log_info("log_level        : %s", Logger::shared()->level_name().c_str());
	log_info("log_output       : %s", Logger::shared()->output_name().c_str());
	log_info("log_rotate_size  : %" PRId64, Logger::shared()->rotate_size());

	log_info("main_db          : %s", data_db_dir.c_str());
	log_info("meta_db          : %s", meta_db_dir.c_str());
	log_info("cache_size       : %d MB", option.cache_size);
	log_info("block_size       : %d KB", option.block_size);
	log_info("write_buffer     : %d MB", option.write_buffer_size);
	log_info("max_open_files   : %d", option.max_open_files);
	log_info("compaction_speed : %d MB/s", option.compaction_speed);
	log_info("compression      : %s", option.compression.c_str());
	log_info("binlog           : %s", option.binlog? "yes" : "no");
	log_info("binlog_capacity  : %d", option.binlog_capacity);
	log_info("sync_speed       : %d MB/s", conf->get_num("replication.sync_speed"));

    data_db = NULL;
	data_db = SSDB::open(option, data_db_dir);
	if(!data_db){
		log_fatal("could not open data db: %s", data_db_dir.c_str());
		fprintf(stderr, "could not open data db: %s\n", data_db_dir.c_str());
		exit(1);
	}

    meta_db = NULL;
	meta_db = SSDB::open(Options(), meta_db_dir);
	if(!meta_db){
		log_fatal("could not open meta db: %s", meta_db_dir.c_str());
		fprintf(stderr, "could not open meta db: %s\n", meta_db_dir.c_str());
		exit(1);
	}

    destroy(other_args.empty() ? serve() : init_jvm());
}

bool MyApplication::serve(){
    net = NULL;
	net = NetworkServer::init(*conf);

	server = new SSDBServer(data_db, meta_db, *conf, net);
	
	log_info("pidfile: %s, pid: %d", app_args.pidfile.c_str(), (int)getpid());
	log_info("ssdb server started.");
	net->serve();

	delete net;
	delete server;

    return true;
}

void MyApplication::destroy(bool success){
	delete meta_db;
	delete data_db;

	log_info("%s exit.", APP_NAME);

    if (!success)
        exit(1);
}

bool MyApplication::init_jvm() {
    size_t size = other_args.size(), len = size, offset = size;
    JavaVMInitArgs vm_args;
    JavaVMOption *options;
    JNIEnv *env;

    while (offset-- > 0 && !ends_with(other_args[offset], dot_jar))
        len--;

    if (len == size || len < 2) {
        fprintf(stderr, "Required jvm options: -cp app.jar com.example.Main\n");
        return false;
    }
    
    /*
    // from https://github.com/nginx-clojure/nginx-clojure
    void *env;
    void *libVM;
    jni_createvm_pt jvm_creator;

    if (jvm != NULL && jvm_env != NULL) {
        return 0;
    }

    // append RTLD_GLOBAL flag for Alpine Linux on which OpenJDK 7 libjvm.so 
    // can not correctly handle cross symbol links from libjava.so, libverify.so
    if (NULL == (libVM = dlopen(jvm_path, RTLD_LAZY | RTLD_GLOBAL))) {
        fprintf(stderr, "Could not open shared lib :%s,\n %s\n", jvm_path, dlerror());
        return false;
    }

    if (NULL == (jvm_creator = reinterpret_cast<jni_createvm_pt>(dlsym(libVM, "JNI_CreateJavaVM"))) &&
        // for macosx default jvm
        NULL == (jvm_creator = reinterpret_cast<jni_createvm_pt>(dlsym(libVM, "JNI_CreateJavaVM_Impl")))) {
        return false;
    }*/
    
    if (NULL == (options = static_cast<JavaVMOption*>(malloc(len * sizeof(JavaVMOption))))) {
        return false;
    }

    for (size_t i = 0; i < len; i++){
        options[i].extraInfo = NULL;
        options[i].optionString = const_cast<char*>(other_args[i].c_str());
    }

    vm_args.version = JNI_VERSION_1_6;
    vm_args.ignoreUnrecognized = JNI_TRUE;
    vm_args.options = options;
    vm_args.nOptions = len;

    /*if ((*jvm_creator)(&jvm, (void **)&env, (void *)&vm_args) < 0){
        free(options);
        fprintf(stderr, "Could not create java vm.\n");
        return 1;
    }*/

    if (JNI_CreateJavaVM(&jvm, (void **)&env, &vm_args) < 0) {
        free(options);
        fprintf(stderr, "Could not create java vm.\n");
        return false;
    }

    free(options);
    jvm_env = static_cast<JNIEnv*>(env);

    main_class_offset = offset = len;
    auto mc = other_args[offset++].c_str();

    jclass mainClass = jvm_env->FindClass(mc);
    if (mainClass == NULL) {
        fprintf(stderr, "Could not find main class: %s\n", mc);
        destroy_jvm();
        return false;
    }

    jmethodID mainMethod = jvm_env->GetStaticMethodID(mainClass, "main", "([Ljava/lang/String;)V");
    if (mainMethod == NULL) {
        fprintf(stderr, "%s does not have the method: public static void main(String[] args)\n", mc);
        destroy_jvm();
        return false;
    }

    size_t length = size - offset;
    jobjectArray arr = jvm_env->NewObjectArray(length, jvm_env->FindClass("java/lang/String"), NULL);
    if (arr == NULL) {
        fprintf(stderr, "Could not create args for %s.main\n", mc);
        destroy_jvm();
        return false;
    }
    
    for (size_t i = 0; i < length; i++)
        jvm_env->SetObjectArrayElement(arr, i, jvm_env->NewStringUTF(other_args[offset++].c_str()));
    
    jvm_env->CallStaticVoidMethod(mainClass, mainMethod, arr);
    jvm_env->DeleteLocalRef(arr);

    printf("jvm exited.\n");

    return true;
}

void MyApplication::destroy_jvm() {
    jclass systemClass = jvm_env->FindClass("java/lang/System");
    jmethodID exitMethod = jvm_env->GetStaticMethodID(systemClass, "exit", "(I)V");
    jvm_env->CallStaticVoidMethod(systemClass, exitMethod, 0);
    jvm->DestroyJavaVM();
}

int main(int argc, char **argv){
	MyApplication app;
    return app.main(argc, argv);
}
