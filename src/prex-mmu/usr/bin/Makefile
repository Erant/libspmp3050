include $(SRCDIR)/mk/own.mk
SUBDIR=		init

ifeq ($(CONFIG_CMDBOX),y)
# Pack commands in cmdbox
SUBDIR+=	cmdbox
else
# Build all commands
SUBDIR-$(CONFIG_CMD_CAL)+=	cal
SUBDIR-$(CONFIG_CMD_CAT)+=	cat
SUBDIR-$(CONFIG_CMD_CLEAR)+=	clear
SUBDIR-$(CONFIG_CMD_CP)+=	cp
SUBDIR-$(CONFIG_CMD_DATE)+=	date
SUBDIR-$(CONFIG_CMD_DMESG)+=	dmesg
SUBDIR-$(CONFIG_CMD_ECHO)+=	echo
SUBDIR-$(CONFIG_CMD_FREE)+=	free
SUBDIR-$(CONFIG_CMD_HEAD)+=	head
SUBDIR-$(CONFIG_CMD_HOSTNAME)+=	hostname
SUBDIR-$(CONFIG_CMD_KILL)+=	kill
SUBDIR-$(CONFIG_CMD_LS)+=	ls
SUBDIR-$(CONFIG_CMD_MKDIR)+=	mkdir
SUBDIR-$(CONFIG_CMD_MOUNT)+=	mount
SUBDIR-$(CONFIG_CMD_MV)+=	mv
SUBDIR-$(CONFIG_CMD_NICE)+=	nice
SUBDIR-$(CONFIG_CMD_PS)+=	ps
SUBDIR-$(CONFIG_CMD_PWD)+=	pwd
SUBDIR-$(CONFIG_CMD_REBOOT)+=	reboot
SUBDIR-$(CONFIG_CMD_RM)+=	rm
SUBDIR-$(CONFIG_CMD_RMDIR)+=	rmdir
SUBDIR-$(CONFIG_CMD_SHUTDOWN)+=	shutdown
SUBDIR-$(CONFIG_CMD_SLEEP)+=	sleep
SUBDIR-$(CONFIG_CMD_SYNC)+=	sync
SUBDIR-$(CONFIG_CMD_TEST)+=	test
SUBDIR-$(CONFIG_CMD_TOUCH)+=	touch
SUBDIR-$(CONFIG_CMD_UMOUNT)+=	umount
SUBDIR-$(CONFIG_CMD_UNAME)+=	uname
SUBDIR-$(CONFIG_CMD_SH)+=	sh

SUBDIR+= $(SUBDIR-y)
endif

include $(SRCDIR)/mk/subdir.mk
