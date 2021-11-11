ASM_SRC		= $(sort $(wildcard *.S))
C_SRC     = $(sort $(wildcard *.c))
CPP_SRC   = $(sort $(wildcard *.cc))
OBJ		    = $(patsubst %.S,%.o,$(ASM_SRC)) $(patsubst %.c,%.o,$(C_SRC)) $(patsubst %.cc,%.o,$(CPP_SRC))

all:$(OBJ)


%.o:%.S
	${GCC}  ${CFLAG} -c $<

%.o:%.c
	${GCC}  ${CFLAG} -c $<

%.o:%.cc
	${GXX}  ${CFLAG} -c $<

clean:
	$(shell rm *.o 2>/dev/null)
