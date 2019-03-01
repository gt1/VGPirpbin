VGPirpbin: VGPirpbin.c
	${CC} -ansi -pedantic -W -Wall -O3 -g VGPirpbin.c -o VGPirpbin

clean:

distclean: clean
	rm -f VGPirpbin

#	cc -W -Wall -O3 -ansi -pedantic VGPirpbin.c -o VGPirpbin
