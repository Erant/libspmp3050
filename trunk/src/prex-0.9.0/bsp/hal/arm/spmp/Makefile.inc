
SRCS:=		arm/arch/locore.S \
		arm/arch/cpufunc.S \
		arm/arch/cpu.c \
		arm/arch/trap.c \
		arm/arch/context.c \
		arm/spmp/interrupt.c \
		arm/spmp/clock.c \
		arm/spmp/machdep.c

ifeq ($(CONFIG_MMU),y)
SRCS+=		arm/arch/mmu.c
endif

ifeq ($(DEBUG),1)
SRCS+=		arm/spmp/diag.c
endif
