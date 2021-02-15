#include <stdint.h>

#include "fixed_point.h"

Distance int_sqrt(Distance s)
{
  Distance guess = s >> 1;

  // Sanity check
  if (!guess)
    return s;

  Distance next_guess = (guess + s / guess) >> 1;

  while (next_guess < guess) {
    guess = next_guess;
    next_guess = (guess + s / guess ) >> 1;
  }

  return guess;
}

Distance distance(Coordinate p1, Coordinate p2)
{
  Distance dx = p1.x - p2.x;
  Distance dy = p1.y - p2.y;
  Distance dz = p1.z - p2.z;
  if(p1.x < p2.x)
    dx = p2.x - p1.x;
  if(p1.y < p2.y)
    dy = p2.y - p1.y;
  if(p1.z < p2.z)
    dz = p2.z - p1.z;

  unsigned shift = 0;

  // Downshift values until it's certain that the sum of their squares can fit in
  // an unsigned 64 bit integer.
  while((dx >= 0x7fffffff) | (dy >= 0x7fffffff) | (dz >= 0x7fffffff)) {
    dx >>= 1;
    dy >>= 1;
    dz >>= 1;

    shift++;
  }

  Distance dist = int_sqrt(dx*dx + dy*dy + dz*dz);

  return dist << shift;
}
