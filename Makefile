KDIR=/lib/modules/$(shell uname -r)/build

obj-m += calc.o
calc-objs += main.o expression.o
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

eval: eval.c fixed-point.h
	$(CC) -o $@ $< -std=gnu11

clean:
	make -C $(KDIR) M=$(PWD) clean
	$(RM) eval
