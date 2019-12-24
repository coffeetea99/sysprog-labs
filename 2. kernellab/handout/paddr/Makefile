KDIR = /lib/modules/$(shell uname -r)/build

obj-m := dbfs_paddr.o

all : 
	$(MAKE) -C $(KDIR) M=$(PWD) modules;
	gcc -o va2pa va2pa.c;
	sudo insmod dbfs_paddr.ko

clean : 
	$(MAKE) -C $(KDIR) M=$(PWD) clean;
	rm va2pa;
	sudo rmmod dbfs_paddr.ko
