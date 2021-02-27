CC=g++-10
CCFLAGS+=-g -DDEBUG -std=c++17 -Wall -O3 -I.
LDFLAGS+=-lm -ltbb
HEADERS=src/camera.hpp \
        src/color.hpp \
        src/hittable.hpp \
        src/hittable_list.hpp \
        src/material.hpp \
        src/ray.hpp \
        src/rtweekend.hpp \
        src/sphere.hpp \
        src/image_plane.hpp \
        src/vec3.hpp \
        src/greenmeadow.h

.PHONY: all
all: raytracer_scene

/disk1/www/raytracer/test.png: raytracer
	echo ""
	time ./raytracer $@
	echo ""
	echo ""
	echo ""

/disk1/www/raytracer/test_scene.png: raytracer_scene
	time ./raytracer_scene $@

raytracer: main.o
	echo "[LD] $@"
	${CC} ${CCFLAGS} $^ -o $@ ${LDFLAGS}

raytracer_scene: main_scene.o greenmeadow.o
	echo "[LD] $@"
	${CC} ${CCFLAGS} $^ -o $@ ${LDFLAGS}

main.o: src/main.cpp $(HEADERS)
	echo "[CC] $@"
	${CC} ${CCFLAGS} $< -o $@ -c

main_scene.o: src/main_scene.cpp $(HEADERS)
	echo "[CC] $@"
	${CC} ${CCFLAGS} $< -o $@ -c

greenmeadow.o: src/greenmeadow.cpp src/greenmeadow.h
	echo "[CC] $@"
	${CC} ${CCFLAGS} $< -o $@ -c

ppm.o: src/ppm.cpp src/ppm.hpp
	echo "[CC] $@"
	${CC} ${CCFLAGS} $< -o $@ -c

.SILENT:

.PHONY: clean
clean:
	-rm *.o raytracer
