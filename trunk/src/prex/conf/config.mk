#
# Automatically generated file. Don't edit
#
LOADER_BASE=0x00010000
KERNEL_BASE=0x80080000
DRIVER_BASE=AUTODETECT
SYSPAGE_BASE=0x80000000
BOOTIMG_BASE=0x80012000
CFLAGS+=-mcpu=arm926ej-s
CONFIG_HZ=1000
CONFIG_TIME_SLICE=50
CONFIG_OPEN_MAX=16
CONFIG_BUF_CACHE=32
CONFIG_FS_THREADS=4
CONFIG_DIAG_SERIAL=y

CONFIG_CMDBOX=y
CONFIG_CMD_CAT=y
CONFIG_CMD_CLEAR=y
CONFIG_CMD_CP=y
CONFIG_CMD_DATE=y
CONFIG_CMD_DMESG=y
CONFIG_CMD_ECHO=y
CONFIG_CMD_FREE=y
CONFIG_CMD_HEAD=y
CONFIG_CMD_HOSTNAME=y
CONFIG_CMD_KILL=y
CONFIG_CMD_LS=y
CONFIG_CMD_MKDIR=y
CONFIG_CMD_MOUNT=y
CONFIG_CMD_MV=y
CONFIG_CMD_NICE=y
CONFIG_CMD_PS=y
CONFIG_CMD_PWD=y
CONFIG_CMD_REBOOT=y
CONFIG_CMD_RM=y
CONFIG_CMD_RMDIR=y
CONFIG_CMD_SH=y
CONFIG_CMD_SHUTDOWN=y
CONFIG_CMD_SLEEP=y
CONFIG_CMD_SYNC=y
CONFIG_CMD_TEST=y
CONFIG_CMD_TOUCH=y
CONFIG_CMD_UMOUNT=y
CONFIG_CMD_UNAME=y

DRIVER+= $(SRCDIR)/dev/dev.ko
BOOTTASKS+= $(SRCDIR)/usr/server/boot/boot
BOOTTASKS+= $(SRCDIR)/usr/server/proc/proc
BOOTTASKS+= $(SRCDIR)/usr/server/fs/fs
BOOTTASKS+= $(SRCDIR)/usr/server/exec/exec
BOOTFILES+= $(SRCDIR)/usr/bin/init/init
BOOTFILES+= $(SRCDIR)/usr/bin/cmdbox/cmdbox
BOOTFILES+= $(SRCDIR)/doc/LICENSE
BOOTFILES+= $(SRCDIR)/usr/sample/hello/hello

SRCDIR=/home/niels/src/libspmp3050/src/prex
ARCH=arm
PLATFORM=spmp
PROFILE=spmp
CROSS_COMPILE=/usr/local/arm/bin/arm-elf-

CONFIG_CFLAGS+= -fno-stack-protector
