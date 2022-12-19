obj-m+=myKvStore.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC)  myKvTest1.c -o  myKvTest1
	$(CC)  myKvTest2.c -o  myKvTest2
	$(CC)  myKvTest3.c -o  myKvTest3
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm  myKvTest1 myKvTest2 myKvTest3

