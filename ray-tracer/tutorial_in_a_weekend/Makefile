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
        src/vec3.hpp \
        src/aabb.hpp \
        src/bvh.hpp \
        src/moving_sphere.hpp \
        src/perlin.hpp \
        src/texture.hpp \
        src/rtw_stb_image.hpp \
        src/aarect.hpp

HEADERS_SCENE=src/image_plane.hpp \
              src/greenmeadow.h

.PHONY: all
all: raytracer

/disk1/www/raytracer/test_scene.png: raytracer_scene
	time ./raytracer_scene $@

%.png: raytracer
	time ./raytracer $@

raytracer: main.o
	echo "[LD] $@"
	${CC} ${CCFLAGS} $^ -o $@ ${LDFLAGS}

test: test.o
	echo "[LD] $@"
	${CC} ${CCFLAGS} $^ -o $@ ${LDFLAGS}

raytracer_scene: main_scene.o greenmeadow.o
	echo "[LD] $@"
	${CC} ${CCFLAGS} $^ -o $@ ${LDFLAGS}

main.o: src/main.cpp $(HEADERS)
	echo "[CC] $@"
	${CC} ${CCFLAGS} $< -o $@ -c

test.o: src/test.cpp $(HEADERS)
	echo "[CC] $@"
	${CC} ${CCFLAGS} $< -o $@ -c

main_scene.o: src/main_scene.cpp $(HEADERS) $(HEADERS_SCENE)
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
