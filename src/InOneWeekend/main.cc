#include "rtweekend.h"

#include "camera.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "vector"

int main()
{
    // Build sphere list on host using a std::vector for convenience,
    // then copy to a raw array for the GPU.
    std::vector<Sphere> sphere_vec;

    // Ground
    sphere_vec.push_back(Sphere(point3(0, -1000, 0), 1000,
                                Material::make_lambertian(color(0.5, 0.5, 0.5))));

    // Random small spheres
    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            auto choose_mat = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
            if ((center - point3(4, 0.2, 0)).length() > 0.9)
            {
                if (choose_mat < 0.8)
                {
                    auto albedo = color::random() * color::random();
                    sphere_vec.push_back(Sphere(center, 0.2,
                                                Material::make_lambertian(albedo)));
                }
                else if (choose_mat < 0.95)
                {
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_vec.push_back(Sphere(center, 0.2,
                                                Material::make_metal(albedo, fuzz)));
                }
                else
                {
                    sphere_vec.push_back(Sphere(center, 0.2,
                                                Material::make_dielectric(1.5)));
                }
            }
        }
    }

    // Three large spheres
    sphere_vec.push_back(Sphere(point3(0, 1, 0), 1.0,
                                Material::make_dielectric(1.5)));
    sphere_vec.push_back(Sphere(point3(-4, 1, 0), 1.0,
                                Material::make_lambertian(color(0.4, 0.2, 0.1))));
    sphere_vec.push_back(Sphere(point3(4, 1, 0), 1.0,
                                Material::make_metal(color(0.7, 0.6, 0.5), 0.0)));

    int n = (int)sphere_vec.size();
    Sphere *spheres = new Sphere[n];
    for (int i = 0; i < n; i++)
        spheres[i] = sphere_vec[i];

    hittable_list world(spheres, n);

    camera cam;
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width = 1200;
    cam.samples_per_pixel = 10;
    cam.max_depth = 20;
    cam.vfov = 20;
    cam.lookfrom = point3(13, 2, 3);
    cam.lookat = point3(0, 0, 0);
    cam.vup = vec3(0, 1, 0);
    cam.defocus_angle = 0.6;
    cam.focus_dist = 10.0;

    cam.render(world);

    delete[] spheres;
}
