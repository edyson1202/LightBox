#include "Vector3.h"

namespace LightBox {
    std::ostream& operator<<(std::ostream& os, const Vector3& vec)
    {
        std::cout << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z << "\n";

        return os;
    }
    Vector3 Vector3::Reflect(const Vector3& v, const Vector3& normal) {
        return v - 2 * Vector3::Dot(v, normal) * normal;
    }
    Vector3 Vector3::Refract(const Vector3& v, const Vector3& n, float etai_over_etat, float cos_theta)
    {
        //float cos_theta = fmin(Vector3::Dot(-v, n), 1.f);
        Vector3 refracted_x = etai_over_etat * (v + cos_theta * n);
        Vector3 refracted_y = -sqrt(fabs(1.f - refracted_x.GetLengthSq())) * n;
        return refracted_x + refracted_y;
    }

    Vector3 Vector3::Mix(const Vector3& u, const Vector3& v, float t)
    {
        return (1.f - t) * u + t * v;
    }
}
