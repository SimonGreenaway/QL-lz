CC=qgcc qdos-gcc
COPTS=-Wall -O3 -fomit-frame-pointer

OBJS=lz.o
DEPS=lz.h

%.o: %.c $(DEPS)
	$(CC) $(COPTS) -c -o $@ $< $(CFLAGS)

all:	lz

lz:	lz.o
	$(CC) -o lz lz.o  -L/usr/local/share/qdos/lib -lm

clean:
	rm -f $(OBJS)
	rm -fR flp1

git:	clean
	git add .
	git commit
	git push

flp1:
	mkdir flp1

deploy:  lz flp1
	cp lz flp1
	#cp BOOT_flp1 flp1/BOOT
	#cp env.bin flp1/env_bin

run:    deploy
	/home/simon/emulators/ql/emulators/sQLux/sqlux ./sqlux --romdir=/home/simon/emulators/ql/emulators/sQLux/roms --kbd=UK --speed=0.75 --ramsize=896 --sound 8 --win_size 2x --device flp1,./flp1,/home/simon/code/ql/QL-sprites/flp1,qdos-like --romport=TK236.rom --fixaspect=2 --filter=1

runfast:    deploy
	/home/simon/emulators/ql/emulators/sQLux/sqlux ./sqlux --romdir=/home/simon/emulators/ql/emulators/sQLux/roms --kbd=UK --speed=0 --ramsize=896 --sound 8 --win_size 2x --device flp1,./flp1,/home/simon/code/ql/QL-sprites/flp1,qdos-like --romport=TK236.rom --fixaspect=2 --filter=1 
