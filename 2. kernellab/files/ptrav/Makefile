KDIR = /lib/modules/$(shell uname -r)/build

obj-m := dbfs_ptrav.o

all : 
	$(MAKE) -C $(KDIR) M=$(PWD) modules;
	gcc -o rss rss.c;
	gcc -static test.c -o test;
	sudo insmod dbfs_ptrav.ko

clean : 
	$(MAKE) -C $(KDIR) M=$(PWD) clean;
	rm rss test;
	sudo rmmod dbfs_ptrav.ko
