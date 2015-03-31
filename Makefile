all: bin2png png2bin

clean:
	@rm -rf *.o bin2png png2bin

CFLAGS = -std=c99 -Wall -Wextra
LDFLAGS = -lpng

common.o: common.h
	clang -o $@ -c common.c $(CFLAGS)

imgify.o: imgify.h
	clang -o $@ -c imgify.c $(CFLAGS)

bin2png: common.o imgify.o
	clang -o $@ bin2png.c $^ $(CFLAGS) $(LDFLAGS)

png2bin: common.o imgify.o
	clang -o $@ png2bin.c $^ $(CFLAGS) $(LDFLAGS)