#!/bin/sh

SSDB_SERVER=target/bin/ssdb-server

SSDB_JNI_VERSION=1.0.0-SNAPSHOT
SSDB_JNI_JAR=target/ssdb-jni-$SSDB_JNI_VERSION.jar

[ ! -e $SSDB_SERVER ] && echo "$SSDB_SERVER not built." && exit 0
[ ! -e $SSDB_JNI_JAR ] && echo "$SSDB_JNI_JAR not built." && exit 0

mkdir -p target/data/ssdb

./$SSDB_SERVER jni-test.conf -Dssdb.readers=4 -Djava.class.path=$PWD/$SSDB_JNI_JAR ssdb.Jni $@
