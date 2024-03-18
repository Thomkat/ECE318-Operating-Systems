all:
	gcc -g -std=c11 -D_DEFAULT_SOURCE -Wall bbfs.c log.c our_lib.c \
		`pkg-config fuse --cflags --libs` \
		`pkg-config libssl libcrypto --cflags --libs` -o bbfs

clean:
	rm -rf ./bbfs .real_storage

	
