CC=g++-10
CCFLAGS+=-std=c++17 -Wall -O3 -I.
LDFLAGS+=-lm -ltbb
HEADERS=src/camera.hpp \
        src/color.hpp \
        src/hittable.hpp \
        src/hittable_list.hpp \
        src/material.hpp \
        src/ray.hpp \
        src/rtweekend.hpp \
        src/sphere.hpp \
        src/vec3.hpp

.PHONY: all
all: test.ppm
	ffmpeg -y -i test.ppm /disk1/www/raytracer/test.png

test.ppm: raytracer
	echo ""
	time ./raytracer
	echo ""
	echo ""
	echo ""

raytracer: main.o
	echo "[LD] $@"
	${CC} ${CCFLAGS} $^ -o $@ ${LDFLAGS}

main.o: src/main.cpp $(HEADERS)
	echo "[CC] $@"
	${CC} ${CCFLAGS} $< -o $@ -c

ppm.o: src/ppm.cpp src/ppm.hpp
	echo "[CC] $@"
	${CC} ${CCFLAGS} $< -o $@ -c

.SILENT:

.PHONY: clean
clean:
	-rm *.o raytracer
