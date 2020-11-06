KDIR=/lib/modules/$(shell uname -r)/build

SRCS = main.c fixed-point.c eval.c expression.c
obj-m += calc.o
calc-objs += main.o expression.o fixed-point.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS) eval
	make -C $(KDIR) M=$(PWD) modules

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

load: calc.ko
	sudo insmod calc.ko
	sudo chmod 0666 /dev/calc

unload:
	sudo rmmod calc

check: all
	scripts/test.sh

%.o: %.c
	$(CC) -c -o $@ $^

eval: eval.o fixed-point.o
	$(CC) -o $@ $^ -std=gnu11

clean:
	make -C $(KDIR) M=$(PWD) clean
	$(RM) eval
