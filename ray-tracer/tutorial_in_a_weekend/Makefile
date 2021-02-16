CCFLAGS+=-g -Wall -I.
LDFLAGS+=-lm

.PHONY: all
all: test.ppm
	ffmpeg -y -i test.ppm /disk1/www/raytracer/test.png

test.ppm: raytracer
	./raytracer > $@

raytracer: main.o
	echo "[LD] $@"
	g++ ${CCFLAGS} $^ -o $@ ${LDFLAGS}

main.o: main.cpp vec3.hpp hittable_list.hpp hittable.hpp rtweekend.hpp sphere.hpp color.hpp ray.hpp tqdm.hpp tqdm_utils.hpp
	echo "[CC] $@"
	g++ ${CCFLAGS} $< -o $@ -c

ppm.o: ppm.cpp ppm.hpp
	echo "[CC] $@"
	g++ ${CCFLAGS} $< -o $@ -c

.SILENT:

.PHONY: clean
clean:
	-rm *.o raytracer
