#include <stdio.h>
#include <math.h>

class Vector;
class Sphere;

class Vector {
public:
  Vector(float x, float y, float z)
  {
    this->x = x;
    this->y = y;
    this->z = z;
  }
  Vector(const Vector &orig)
  {
    this->x = orig.x;
    this->y = orig.y;
    this->z = orig.z;
  }
  Vector operator=(const Vector &orig)
  {
    return Vector(orig.x, orig.y, orig.z);
  }

  const char *c_str(void)
  {
    static char temp_buf[1024];
    sprintf(temp_buf, "{x: %f, y: %f, z: %f}", this->x, this->y, this->z);
    return temp_buf;
  }

  // Returns the scalar product of two vectors
  float dot(const Vector &other)
  {
    return (
      this->x * other.x +
      this->y * other.y +
      this->z * other.z
    );
  }

  float norm()
  {
    float scale_factor = sqrt(this->dot(*this));
    this->x /= scale_factor;
    this->y /= scale_factor;
    this->z /= scale_factor;

    return scale_factor;
  }

  Vector operator+(Vector &other)
  {
    return Vector(
        other.x + this->x,
        other.y + this->y,
        other.z + this->z
    );
  }

  Vector operator-(Vector &other)
  {
    return Vector(
        other.x - this->x,
        other.y - this->y,
        other.z - this->z
    );
  }
  
  Vector operator*(float scale)
  {
    return Vector(this->x * scale, this->y * scale, this->z * scale);
  }

  Vector operator/(float scale)
  {
    return Vector(this->x / scale, this->y / scale, this->z / scale);
  }

  friend Vector operator*(Vector &vec, float scale);

private:
  float x, y, z;
};

Vector operator*(Vector &vec, float scale)
{
  return Vector(scale * vec.x, scale * vec.y, scale * vec.z);
}


class Sphere {
public:
  Sphere(
    const Vector &position,
    const Vector &colour,
    float radius, // Diameter?
    float b, // Glare thingie, higher -> smaller glares
    float specularity
  ) : pos(position), col(colour)
  {
    this->pos = position;
    this->col = colour;
    this->radius = radius;
    this->b = b;
    this->specularity = specularity;
  }

  bool intersects(
    Vector &orig,
    Vector &direction,
    float &output,
    float cutoff=0.01
  )
  {
    orig = orig - this->pos;
    output = -orig.dot(direction);
    output -= sqrt(
       this->radius * this->radius - orig.dot(orig) + output * output
    );

    if(output > cutoff)
      return true;

    return false;
  }

private:
  Vector pos;
  Vector col;
  float radius;
  float b;
  float specularity;
};

class Light {
public:
  Light(
    const Vector &position,
    const Vector &colour,
    float a
  );
};

Vector trace(const Vector &origo, const Vector &direction, int steps)
{
  
}

int main(void)
{
  Sphere spheres[] = {
    Sphere(Vector(-12.73, -12.73, 50), Vector(1, 0, 0), 8, 50, 0.3),
    Sphere(Vector(-12.73, 12.73, 50), Vector(0, 1, 0), 8, 50, 0.3),
    Sphere(Vector(12.73, 12.73, 50), Vector(0, 0, 1), 8, 50, 0.3),
    Sphere(Vector(12.73, -12.73, 50), Vector(1, 1, 0), 8, 50, 0.3),
    Sphere(Vector(0, 0, 50), Vector(1, 1, 1), 10, 50, 0.6)
  };

  float tmp;
  Vector orig(0, 0, 0);
  /*
  Vector directions[] = {
    Vector(
      1024 / 2 - 0 % 1024,
      1024 / 2 - 0 / 1024,
      1024
    ),
    Vector(
      1024 / 2 - 512 % 1024,
      1024 / 2 - 512 / 1024,
      1024
    ),
    Vector(
      1024 / 2 - (1024*1024 / 2) % 1024,
      1024 / 2 - (1024*1024 / 2) / 1024,
      1024
    ),
    Vector(
      1024 / 2 + 0 % 1024,
      1024 / 2 + 0 / 1024,
      1024
    ),
    Vector(
      1024 / 2 + 512 % 1024,
      1024 / 2 + 512 / 1024,
      1024
    ),
    Vector(
      1024 / 2 + (1024*1024 / 2) % 1024,
      1024 / 2 + (1024*1024 / 2) / 1024,
      1024
    ),
    Vector(
      -12.73,
      -12.73+8,
      50
    ),
  };

  for(unsigned int i=0; i < sizeof(directions)/sizeof(*directions); i++) {
    Vector direction = directions[i];
  */

  int S = 1024;
  for(int i = S*S; i--; ) {
    Vector direction = Vector(
      S/2 - i%S,
      S/2 - i/S,
      S
    );
    direction.norm();

    bool does = spheres[0].intersects(orig, direction, tmp);
    if(does) {
      printf("%s - ", direction.c_str());
      printf("%f\n", tmp);
    }
  }
}
