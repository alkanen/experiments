CC=g++-10
CCFLAGS+=-std=c++17 -Wall -O3 -I.
LDFLAGS+=-lm -ltbb
HEADERS=camera.hpp \
        color.hpp \
        hittable.hpp \
        hittable_list.hpp \
        material.hpp \
        ray.hpp \
        rtweekend.hpp \
        sphere.hpp \
        tqdm.hpp \
        tqdm_utils.hpp \
        vec3.hpp

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

main.o: main.cpp $(HEADERS)
	echo "[CC] $@"
	${CC} ${CCFLAGS} $< -o $@ -c

ppm.o: ppm.cpp ppm.hpp
	echo "[CC] $@"
	${CC} ${CCFLAGS} $< -o $@ -c

.SILENT:

.PHONY: clean
clean:
	-rm *.o raytracer
