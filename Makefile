# gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello

CC=gcc
CFLAGS=-Wall
SRC=dirfiller.c utility.c ytfs.c
INC=dirfiller.h utility.h ytfs.h

all : CFLAGS += -I/usr/include/python2.7/ -I/usr/include/postgresql/ -L/usr/lib/postgresql/9.3/lib/
all : LDFLAGS += -lpq -lpython2.7 -lssl -lcrypto
all :
	$(CC) $(CFLAGS) $(INC) $(SRC) `pkg-config fuse --cflags --libs` -o ytfs $(LDFLAGS)
	mkdir MyTFS
	mkdir .MyTFS
	echo "./ytfs MyTFS .MyTFS" > ytfs.sh && chmod +x ytfs.sh

clean :
	fusermount -u MyTFS || true
	rm -rf ytfs MyTFS .MyTFS ytfs.sh
