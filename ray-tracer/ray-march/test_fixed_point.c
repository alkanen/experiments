#include <math.h>
#include <stdio.h>

#include "fixed_point.h"

double trad_dist(Coordinate p1, Coordinate p2)
{
  double dx = p1.x - p2.x;
  double dy = p1.y - p2.y;
  double dz = p1.z - p2.z;

  return sqrt(dx*dx + dy*dy + dz*dz);
}

int main(void)
{
  Coordinate p1 = {0xffff, 0xFFFFffffFFFFllu, 1234567890};
  Coordinate p2 = {1234567890, 0xffff, 0xffffFFFFffffllu};

  printf("Distance (mine): %llu\n", (unsigned long long)distance(p1, p2));
  printf("Distance (trad): %f\n", trad_dist(p1, p2));
  printf("Ratio: %f\n", distance(p1, p2)/trad_dist(p1, p2));

  p1 = (Coordinate){0, 0, 0};
  p2 = (Coordinate){10, 10, 10};
  printf("Distance (mine): %llu\n", (unsigned long long)distance(p1, p2));
  printf("Distance (trad): %f\n", trad_dist(p1, p2));
  printf("Ratio: %f\n", distance(p1, p2)/trad_dist(p1, p2));

  p1 = (Coordinate){-10, -10, -10};
  p2 = (Coordinate){10, 10, 10};
  printf("Distance (mine): %llu\n", (unsigned long long)distance(p1, p2));
  printf("Distance (trad): %f\n", trad_dist(p1, p2));
  printf("Ratio: %f\n", distance(p1, p2)/trad_dist(p1, p2));

  p1 = (Coordinate){-10, 10, 10};
  p2 = (Coordinate){10, 10, 10};
  printf("Distance (mine): %llu\n", (unsigned long long)distance(p1, p2));
  printf("Distance (trad): %f\n", trad_dist(p1, p2));
  printf("Ratio: %f\n", distance(p1, p2)/trad_dist(p1, p2));

  p1 = (Coordinate){-10, 10, 10};
  p2 = (Coordinate){-10, 10, 10};
  printf("Distance (mine): %llu\n", (unsigned long long)distance(p1, p2));
  printf("Distance (trad): %f\n", trad_dist(p1, p2));
  printf("Ratio: %f\n", distance(p1, p2)/trad_dist(p1, p2));

  p1 = (Coordinate){10, 10, 10};
  p2 = (Coordinate){10, 10, 10};
  printf("Distance (mine): %llu\n", (unsigned long long)distance(p1, p2));
  printf("Distance (trad): %f\n", trad_dist(p1, p2));
  printf("Ratio: %f\n", distance(p1, p2)/trad_dist(p1, p2));

  p1 = (Coordinate){10, 10, 10};
  p2 = (Coordinate){-10, -10, -10};
  printf("Distance (mine): %llu\n", (unsigned long long)distance(p1, p2));
  printf("Distance (trad): %f\n", trad_dist(p1, p2));
  printf("Ratio: %f\n", distance(p1, p2)/trad_dist(p1, p2));

  p1 = (Coordinate){-10, -10, -10};
  p2 = (Coordinate){-10, -10, -10};
  printf("Distance (mine): %llu\n", (unsigned long long)distance(p1, p2));
  printf("Distance (trad): %f\n", trad_dist(p1, p2));
  printf("Ratio: %f\n", distance(p1, p2)/trad_dist(p1, p2));

  p1 = (Coordinate){-0x7ffffffffffffff, -0x7ffffffffffffff, -0x7ffffffffffffff};
  p2 = (Coordinate){0x7ffffffffffffff, 0x7ffffffffffffff, 0x7ffffffffffffff};
  printf("Distance (mine): %llu\n", (unsigned long long)distance(p1, p2));
  printf("Distance (trad): %f\n", trad_dist(p1, p2));
  printf("Ratio: %f\n", distance(p1, p2)/trad_dist(p1, p2));

  return 0;
}
