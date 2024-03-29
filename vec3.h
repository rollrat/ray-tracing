#ifndef VEC3_H
#define VEC3_h

#include <cmath>
#include <iostream>

using std::sqrt;

#include <cstdlib>
#include <random>

#include <memory>
#include <vector>

using std::make_shared;
using std::shared_ptr;

// inline double random_double() {
//   // Returns a random real in [0,1).
//   return rand() / (RAND_MAX + 1.0);
// }

inline double random_double() {
  static std::uniform_real_distribution<double> distribution(0.0, 1.0);
  static std::mt19937 generator;
  return distribution(generator);
}

inline double random_double(double min, double max) {
  // Returns a random real in [min,max).
  return min + (max - min) * random_double();
}

class vec3 {
public:
  double e[3];

  vec3() : e{0, 0, 0} {}
  vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

  double x() const { return e[0]; }
  double y() const { return e[1]; }
  double z() const { return e[2]; }

  vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
  double operator[](int i) const { return e[i]; }
  double &operator[](int i) { return e[i]; }

  vec3 &operator=(const vec3 &v) {
    e[0] = v.e[0];
    e[1] = v.e[1];
    e[2] = v.e[2];
    return *this;
  }

  vec3 &operator+=(const vec3 &v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
  }

  vec3 &operator*=(const double t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  vec3 &operator/=(const double t) { return *this *= 1 / t; }

  double length() const { return sqrt(length_squared()); }

  double length_squared() const {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }

  inline static vec3 random() {
    return vec3(random_double(), random_double(), random_double());
  }

  inline static vec3 random(double min, double max) {
    return vec3(random_double(min, max), random_double(min, max),
                random_double(min, max));
  }

  bool near_zero() const {
    // Return true if the vector is close to zero in all dimensions.
    const auto s = 1e-8;
    return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
  }
};

using point3 = vec3;
using color = vec3;

inline std::ostream &operator<<(std::ostream &out, const vec3 &v) {
  return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3 &u, const vec3 &v) {
  return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3 &u, const vec3 &v) {
  return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3 &u, const vec3 &v) {
  return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3 &v) {
  return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3 &v, double t) { return t * v; }

inline vec3 operator/(vec3 v, double t) { return (1 / t) * v; }

inline double dot(const vec3 &u, const vec3 &v) {
  return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3 &u, const vec3 &v) {
  return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
              u.e[2] * v.e[0] - u.e[0] * v.e[2],
              u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(vec3 v) { return v / v.length(); }

void write_color(std::ostream &out, color pixel_color) {
  // Write the translated [0,255] value of each color component.
  out << static_cast<int>(255.999 * pixel_color.x()) << ' '
      << static_cast<int>(255.999 * pixel_color.y()) << ' '
      << static_cast<int>(255.999 * pixel_color.z()) << '\n';
}

class ray {
public:
  ray() {}
  ray(const point3 &origin, const vec3 &direction)
      : orig(origin), dir(direction) {}

  point3 origin() const { return orig; }
  vec3 direction() const { return dir; }

  point3 at(double t) const { return orig + t * dir; }

public:
  point3 orig;
  vec3 dir;
};

class material;
struct hit_record {
  // 어떤 지점에서 물체와 만났는지?
  point3 p;
  // 그 물체와 만난 지점에서의 법선 벡터
  vec3 normal;
  // ray의 t값
  double t;
  shared_ptr<material> mat_ptr;

  // 광선이 겉면을 때리고 바깥으로 나간경우
  bool front_face;

  inline void set_face_normal(const ray &r, const vec3 &outward_normal) {
    // 법선과 광선이 같은 방향이라면 내부임 (내적이 0보다 크면)
    //             다른 방향이라면 외부임 (내적이 0보다 작으면)
    front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

class hittable {
public:
  virtual bool hit(const ray &r, double t_min, double t_max,
                   hit_record &rec) const = 0;
};

class sphere : public hittable {
public:
  sphere() {}
  sphere(point3 cen, double r) : center(cen), radius(r){};
  sphere(point3 cen, double r, shared_ptr<material> m)
      : center(cen), radius(r), mat_ptr(m){};

  virtual bool hit(const ray &r, double t_min, double t_max,
                   hit_record &rec) const override;

public:
  point3 center;
  double radius;
  shared_ptr<material> mat_ptr;
};

bool sphere::hit(const ray &r, double t_min, double t_max,
                 hit_record &rec) const {
  vec3 oc = r.origin() - center;
  auto a = r.direction().length_squared();
  auto half_b = dot(oc, r.direction());
  auto c = oc.length_squared() - radius * radius;

  auto discriminant = half_b * half_b - a * c;
  if (discriminant < 0)
    return false;
  auto sqrtd = sqrt(discriminant);

  // Find the nearest root that lies in the acceptable range.
  // 더 가까운 t값을 찾기 위함임.
  // 구의 경우를 보면 광선이 구를 관통하면 하나 또는 두 개의 점과 접촉하는데,
  // 두 개의 점을 접촉하는 경우 더 가까운 하나만 가져와야함
  auto root = (-half_b - sqrtd) / a;
  if (root < t_min || t_max < root) {
    root = (-half_b + sqrtd) / a;
    if (root < t_min || t_max < root)
      return false;
  }

  rec.t = root;
  rec.p = r.at(rec.t);
  vec3 outward_normal = (rec.p - center) / radius;
  rec.set_face_normal(r, outward_normal);
  rec.mat_ptr = mat_ptr;

  return true;
}

class hittable_list : public hittable {
public:
  hittable_list() {}
  hittable_list(shared_ptr<hittable> object) { add(object); }

  void clear() { objects.clear(); }
  void add(shared_ptr<hittable> object) { objects.push_back(object); }

  virtual bool hit(const ray &r, double t_min, double t_max,
                   hit_record &rec) const override;

public:
  std::vector<shared_ptr<hittable>> objects;
};

// 어떤 물체든지 한 번이라도 부딪힌다면...
// 광선과 가장 가깝게 부딪힌 물체의 record를 가져온다.
bool hittable_list::hit(const ray &r, double t_min, double t_max,
                        hit_record &rec) const {
  hit_record temp_rec;
  bool hit_anything = false;
  auto closest_so_far = t_max;

  for (const auto &object : objects) {
    if (object->hit(r, t_min, closest_so_far, temp_rec)) {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      rec = temp_rec;
    }
  }

  return hit_anything;
}

#include <limits>

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

inline double degrees_to_radians(double degrees) {
  return degrees * pi / 180.0;
}

vec3 random_in_unit_disk() {
  while (true) {
    auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
    if (p.length_squared() >= 1)
      continue;
    return p;
  }
}

class camera {
public:
  camera(point3 lookfrom, point3 lookat, vec3 vup,
         double vfov, // vertical field-of-view in degrees
         double aspect_ratio, double aperture, double focus_dist) {
    auto theta = degrees_to_radians(vfov);
    // theta는 카메라가 보는 위 아래의 각도를 설정한다.
    // theta가 45도 라면 카메라는 정확히 90도 만큼의 이미지를 캡쳐한다.
    auto h = tan(theta / 2);
    auto viewport_height = 2.0 * h;
    auto viewport_width = aspect_ratio * viewport_height;

    // auto focal_length = 1.0;

    // origin = point3(0, 0, 0);
    // horizontal = vec3(viewport_width, 0.0, 0.0);
    // vertical = vec3(0.0, viewport_height, 0.0);
    // lower_left_corner =
    //     origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);

    // 카메라가 보고있는 방향의 반대 방향 벡터
    w = unit_vector(lookfrom - lookat);
    // 업벡터와 w 벡터 두 개는 평면을 만드며, 이 평면의 법선 벡터가 u벡터가 됨
    // u벡터는 업벡터와 w벡터의 교차점을 지난다.
    // 카메라의 오른쪽 방향으로 나아감
    u = unit_vector(cross(vup, w));
    // 카메라의 위쪽 방향으로 나아감
    v = cross(w, u);

    origin = lookfrom;
    horizontal = focus_dist * viewport_width * u;
    vertical = focus_dist * viewport_height * v;
    lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_dist * w;

    lens_radius = aperture / 2;
  }

  // ray get_ray(double u, double v) const {
  //   return ray(origin,
  //              lower_left_corner + u * horizontal + v * vertical - origin);
  // }

  // ray get_ray(double s, double t) const {
  //   return ray(origin,
  //              lower_left_corner + s * horizontal + t * vertical - origin);
  // }

  ray get_ray(double s, double t) const {
    vec3 rd = lens_radius * random_in_unit_disk();
    vec3 offset = u * rd.x() + v * rd.y();

    return ray(origin + offset, lower_left_corner + s * horizontal +
                                    t * vertical - origin - offset);
  }

private:
  point3 origin;
  point3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;
  vec3 u, v, w;
  double lens_radius;
};

inline double clamp(double x, double min, double max) {
  if (x < min)
    return min;
  if (x > max)
    return max;
  return x;
}

void write_color(std::ostream &out, color pixel_color, int samples_per_pixel) {
  auto r = pixel_color.x();
  auto g = pixel_color.y();
  auto b = pixel_color.z();

  // Divide the color by the number of samples.
  auto scale = 1.0 / samples_per_pixel;
  // r *= scale;
  // g *= scale;
  // b *= scale;
  // Divide the color by the number of samples and gamma-correct for gamma=2.0.
  r = sqrt(scale * r);
  g = sqrt(scale * g);
  b = sqrt(scale * b);

  // Write the translated [0,255] value of each color component.
  out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
      << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
      << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}

// 랜덤으로 구 형태로 뻗어나가는 벡터
vec3 random_in_unit_sphere() {
  while (true) {
    auto p = vec3::random(-1, 1);
    if (p.length_squared() >= 1)
      continue;
    return p;
  }
}

vec3 random_unit_vector() { return unit_vector(random_in_unit_sphere()); }

class material {
public:
  virtual bool scatter(const ray &r_in, const hit_record &rec,
                       color &attenuation, ray &scattered) const = 0;
};

vec3 reflect(const vec3 &v, const vec3 &n) {
  // v는 직사광
  // n은 법선
  // 리턴하는 것은 법선 벡터에 대한 반사 벡터임
  return v - 2 * dot(v, n) * n;
}

class lambertian : public material {
public:
  lambertian(const color &a) : albedo(a) {}

  virtual bool scatter(const ray &r_in, const hit_record &rec,
                       color &attenuation, ray &scattered) const override {
    auto scatter_direction = rec.normal + random_unit_vector();

    // Catch degenerate scatter direction
    if (scatter_direction.near_zero())
      scatter_direction = rec.normal;
    scattered = ray(rec.p, scatter_direction);
    attenuation = albedo;
    return true;
  }

public:
  color albedo;
};

class metal : public material {
public:
  metal(const color &a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

  virtual bool scatter(const ray &r_in, const hit_record &rec,
                       color &attenuation, ray &scattered) const override {
    vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
    // scattered = ray(rec.p, reflected);
    scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
    attenuation = albedo;
    return (dot(scattered.direction(), rec.normal) > 0);
  }

public:
  color albedo;
  double fuzz;
};

// 굴절 함수
vec3 refract(const vec3 &uv, const vec3 &n, double etai_over_etat) {
  // uv와 n이 unit vector이기 때문
  // a dot b = |a||b| cos theta
  auto cos_theta = fmin(dot(-uv, n), 1.0);
  // 법선 n과 수직인 벡터
  vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
  // 법선 n과 평행인 벡터
  vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
  // 두 개를 합하면 굴절 벡터가 나온다.
  return r_out_perp + r_out_parallel;
}

// 유전체
class dielectric : public material {
public:
  dielectric(double index_of_refraction) : ir(index_of_refraction) {}

  virtual bool scatter(const ray &r_in, const hit_record &rec,
                       color &attenuation, ray &scattered) const override {
    attenuation = color(1.0, 1.0, 1.0);
    double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

    vec3 unit_direction = unit_vector(r_in.direction());
    double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    vec3 direction;

    // 입사각이 너무 크다면 반사해야함
    if (cannot_refract)
      direction = reflect(unit_direction, rec.normal);
    else
      direction = refract(unit_direction, rec.normal, refraction_ratio);

    scattered = ray(rec.p, direction);
    return true;
  }

public:
  double ir; // Index of Refraction
};

#endif