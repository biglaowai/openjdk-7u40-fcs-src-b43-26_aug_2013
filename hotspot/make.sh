#!/bin/bash
# build openjdk script
export LANG=C

# need oracle jdk 1.6
export ALT_BOOTDIR="/usr/java/jdk1.6.0_45"
export ALT_JDK_IMPORT_PATH="/usr/java/jdk1.6.0_45"

export ANT_HOME="/usr/share/ant"

export PATH="/usr/lib:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/lib/i386-linux-gnu:/usr/lib/gcc/i686-linux-gnu/4.8"

#the env variable can not effect, so you need set it manual
export DISABLE_HOTSPOT_OS_VERSION_CHECK=ok

export HOTSPOT_BUILD_JOBS=5
export ALT_OUTPUTDIR="../build/hotspot_debug"

cd make
make DEBUG_BINARIES=true jvmg jvmg1 2>&1|tee ../build/hotspot_build_debug.log 
