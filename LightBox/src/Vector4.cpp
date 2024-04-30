#include "Vector4.h"


#include "Vector3.h"

namespace LightBox {
    Vector4::Vector4(const Vector3& v, float w)
        : x(v.x), y(v.y), z(v.z), w(w) {}
    std::ostream& operator<<(std::ostream& os, const Vector4& vec)
    {
        std::cout << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z << " w: " << vec.w;

        return os;
    }
}