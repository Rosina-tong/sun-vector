PY = /usr/lib/python3.4

FL = sun_vector.so

all: $(FL)

$(FL): solar_pos.o spa.o
	cc -g -shared -Wl,-soname,sun_vector.so.1 -o $(FL) solar_pos.o spa.o -lc

solar_pos.o: solar_pos.c
	cc -fPIC -g -c -Wall solar_pos.c

spa.o: spa.c
	cc -fPIC -g -c -Wall spa.c

clean:
	rm -rf *.o *.so 
