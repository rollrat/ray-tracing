#include "vec3.h"
#include <iostream>

// int main() {
//   const int image_width = 256;
//   const int image_height = 256;

//   vec3 a;

//   std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

//   for (int j = image_height - 1; j >= 0; --j) {
//     std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
//     for (int i = 0; i < image_width; ++i) {
//       color pixel_color(double(i) / (image_width - 1),
//                         double(j) / (image_height - 1), 0.25);
//       write_color(std::cout, pixel_color);
//     }
//   }
// }

// ray가 구에 부딧히는지에 대한 여부
// bool hit_sphere(const point3 &center, double radius, const ray &r) {
//   vec3 oc = r.origin() - center;
//   auto a = dot(r.direction(), r.direction());
//   auto b = 2.0 * dot(oc, r.direction());
//   auto c = dot(oc, oc) - radius * radius;
//   auto discriminant = b * b - 4 * a * c;
//   return (discriminant > 0);
// }

// color ray_color(const ray &r) {
//   if (hit_sphere(point3(0, 0, -1), 0.5, r))
//     return color(1, 0, 0);

//   vec3 unit_direction = unit_vector(r.direction());
//   auto t = 0.5 * (unit_direction.y() + 1.0);
//   return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
// }

// 구와 부딪히는 곳에 대한 t값
// ray 선은 r.center + t*r.direction임
// 따라서 t값을 알면 어떤 점에서 부딪혔는지 알 수 있다.
double hit_sphere(const point3 &center, double radius, const ray &r) {
  vec3 oc = r.origin() - center;
  //   auto a = dot(r.direction(), r.direction());
  //   auto b = 2.0 * dot(oc, r.direction());
  //   auto c = dot(oc, oc) - radius * radius;
  //   auto discriminant = b * b - 4 * a * c;
  //   if (discriminant < 0) {
  //     return -1.0;
  //   } else {
  //     return (-b - sqrt(discriminant)) / (2.0 * a);
  //   }

  // b가 2h이므로 2를 곱하지 않는 다면 ...
  auto a = r.direction().length_squared();
  auto half_b = dot(oc, r.direction());
  auto c = oc.length_squared() - radius * radius;
  auto discriminant = half_b * half_b - a * c;

  if (discriminant < 0) {
    return -1.0;
  } else {
    return (-half_b - sqrt(discriminant)) / a;
  }
}

// color ray_color(const ray &r) {
//   auto t = hit_sphere(point3(0, 0, -1), 0.5, r);
//   if (t > 0.0) {
//     // 구의 중심 점이 (0, 0, -1)임
//     // z가 음수 인곳이 카메라보다 앞 쪽에 있는 곳
//     /*
//                     +y
//                     |
//                     |        /
//                     |
//                     |    /
//                     |
//                     |/
//        -   -   -   /-------------- +x
//                  /  |
//                /
//              /      |
//             +z
//     */
//     // 구와 부딪히는 곳의 법선 벡터
//     // 구와 부딪히는 곳과 수직인 벡터
//     vec3 N = unit_vector(r.at(t) - vec3(0, 0, -1));
//     return 0.5 * color(N.x() + 1, N.y() + 1, N.z() + 1);
//   }
//   vec3 unit_direction = unit_vector(r.direction());
//   t = 0.5 * (unit_direction.y() + 1.0);
//   return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
// }

color ray_color_diffuse(const ray &r, const hittable &world, int depth) {
  hit_record rec;

  if (depth <= 0)
    // 광선이 너무 많이 부딪힌다면 그냥 검은색을 내보내버림
    return color(0, 0, 0);

  if (world.hit(r, 0.001, infinity, rec)) {
    // return 0.5 * (rec.normal + color(1, 1, 1));
    // 법선 벡터와 약간 차이나는 벡터임
    point3 target = rec.p + rec.normal + random_unit_vector();
    // 이 광선이 다른 곳에서 부딪혀 올 수 있음
    // 광선은 빛의 반대 방향임
    // 광선을 쏴서 그 광선이 가는 방향을 역추적하면 빛의 색이 나옴
    // 광선이 하나 부딪힐 때마다 0.5를 곱함으로써 최초의 빛의 영향을 작게
    // 만든다.
    return 0.5 *
           ray_color_diffuse(ray(rec.p, target - rec.p), world, depth - 1);
  }
  vec3 unit_direction = unit_vector(r.direction());
  auto t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

color ray_color_material(const ray &r, const hittable &world, int depth) {
  hit_record rec;

  // If we've exceeded the ray bounce limit, no more light is gathered.
  if (depth <= 0)
    return color(0, 0, 0);

  if (world.hit(r, 0.001, infinity, rec)) {
    ray scattered;
    color attenuation;
    if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
      return attenuation * ray_color_material(scattered, world, depth - 1);
    return color(0, 0, 0);
  }

  vec3 unit_direction = unit_vector(r.direction());
  auto t = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

int main() {

  // Image

  const auto aspect_ratio = 16.0 / 9.0;
  const int image_width = 600;
  const int image_height = static_cast<int>(image_width / aspect_ratio);
  const int samples_per_pixel = 100;
  const int max_depth = 50;

  // World

  hittable_list world;
  // 카메라 앞에 있는 구
  //   world.add(make_shared<sphere>(point3(0, 0, -1), 0.5));
  // 카메라 앞이지만 100 만큼 밑에 있는 존나 큰 구
  //   world.add(make_shared<sphere>(point3(0, -100.5, -1), 100));

  auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
  // auto material_center = make_shared<lambertian>(color(0.7, 0.3, 0.3));
  // auto material_left = make_shared<metal>(color(0.8, 0.8, 0.8));
  // auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2));
  // auto material_left = make_shared<metal>(color(0.8, 0.8, 0.8), 0.3);
  auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 1.0);
  // auto material_center = make_shared<dielectric>(1.5);
  auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
  auto material_left = make_shared<dielectric>(1.5);

  world.add(
      make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
  world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
  world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
  world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

  // Camera

  // camera cam(90.0, aspect_ratio);
  // camera cam(point3(-2, 2, 1), point3(0, 0, -1), vec3(0, 1, 0), 90,
  //            aspect_ratio);
  camera cam(point3(-2, 2, 1), point3(0, 0, -1), vec3(0, 1, 0), 20,
             aspect_ratio);
  //   auto viewport_height = 2.0;
  //   auto viewport_width = aspect_ratio * viewport_height;
  //   auto focal_length = 1.0;

  //   auto origin = point3(0, 0, 0);
  //   auto horizontal = vec3(viewport_width, 0, 0);
  //   auto vertical = vec3(0, viewport_height, 0);
  //   auto lower_left_corner =
  //       origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);

  // Render
  std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

  for (int j = image_height - 1; j >= 0; --j) {
    std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
    color colors[image_width];
#pragma omp parallel for
    for (int i = 0; i < image_width; ++i) {
      //   auto u = double(i) / (image_width - 1);
      //   auto v = double(j) / (image_height - 1);
      //   ray r(origin, lower_left_corner + u * horizontal + v * vertical -
      //   origin); color pixel_color = ray_color(r, world);
      //   write_color(std::cout, pixel_color);
      color pixel_color(0, 0, 0);
      for (int s = 0; s < samples_per_pixel; ++s) {
        auto u = (i + random_double()) / (image_width - 1);
        auto v = (j + random_double()) / (image_height - 1);
        ray r = cam.get_ray(u, v);
        pixel_color += ray_color_material(r, world, max_depth);
      }
      colors[i] = pixel_color;
    }

    for (int i = 0; i < image_width; ++i) {
      write_color(std::cout, colors[i], samples_per_pixel);
    }
  }

  std::cerr << "\nDone.\n";
}