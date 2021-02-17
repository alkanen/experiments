CCFLAGS+=-g -Wall -I.
LDFLAGS+=-lm
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
	./raytracer > $@

raytracer: main.o
	echo "[LD] $@"
	g++ ${CCFLAGS} $^ -o $@ ${LDFLAGS}

main.o: main.cpp $(HEADERS)
	echo "[CC] $@"
	g++ ${CCFLAGS} $< -o $@ -c

ppm.o: ppm.cpp ppm.hpp
	echo "[CC] $@"
	g++ ${CCFLAGS} $< -o $@ -c

.SILENT:

.PHONY: clean
clean:
	-rm *.o raytracer
