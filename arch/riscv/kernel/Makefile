ASM_SRC		= $(filter-out vmlinux.lds.S,$(sort $(wildcard *.S)))
C_SRC       = $(sort $(wildcard *.c))
CPP_SRC     = $(sort $(wildcard *.cc))
OBJ		    = $(patsubst %.S,%.o,$(ASM_SRC)) $(patsubst %.c,%.o,$(C_SRC)) $(patsubst %.cc,%.o,$(CPP_SRC)) $(patsubst %.S,%.o,$(ASM_SRC))

all:$(OBJ) vmlinux.lds

vmlinux.lds: vmlinux.lds.S
	$(GCC) -E -P ${INCLUDE} -o $@ $^; \
	sed -i '/^typedef/d' vmlinux.lds; \
	sed -i '/^extern const/d' vmlinux.lds

%.o:%.S
	${GCC}  ${CFLAG} -c $<

%.o:%.c
	${GCC}  ${CFLAG} -c $<

%.o:%.cc
	${GXX}  ${CFLAG} -c $<

clean:
	$(shell rm *.o vmlinux.lds 2>/dev/null)
