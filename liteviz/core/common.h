#ifndef __LITEVIZ_COMMON_H__
#define __LITEVIZ_COMMON_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <memory>
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <cmath>
#include <map>
#include <array>
#include <unordered_map>

#include <Eigen/Eigen>
#include <Eigen/Core>
#include <Eigen/Dense>

namespace liteviz {

#ifndef RESOURCE_DIR
#define RESOURCE_DIR ""
#endif


#define COLOR_WHITE     vec4f(1.0f, 1.0f, 1.0f, 1.0f)
#define COLOR_RED       vec4f(1.0f, 0.0f, 0.0f, 1.0f)
#define COLOR_GREEN     vec4f(0.0f, 1.0f, 0.0f, 1.0f)
#define COLOR_BLUE      vec4f(0.0f, 0.0f, 1.0f, 1.0f)

#define COLOR_AXIS_X    vec4f(0.819, 0.219, 0.305, 0.5f) 
#define COLOR_AXIS_Y    vec4f(0.454, 0.674, 0.098, 0.5f) 
#define COLOR_AXIS_Z    vec4f(0.207, 0.403, 0.619, 0.5f)


using mat1d = Eigen::Matrix<double, 1, 1>;
using mat2d = Eigen::Matrix<double, 2, 2>;
using mat3d = Eigen::Matrix<double, 3, 3>;
using mat4d = Eigen::Matrix<double, 4, 4>;
using mat5d = Eigen::Matrix<double, 5, 5>;
using mat6d = Eigen::Matrix<double, 6, 6>;

using mat1f = Eigen::Matrix<float, 1, 1>;
using mat2f = Eigen::Matrix<float, 2, 2>;
using mat3f = Eigen::Matrix<float, 3, 3>;
using mat4f = Eigen::Matrix<float, 4, 4>;
using mat5f = Eigen::Matrix<float, 5, 5>;
using mat6f = Eigen::Matrix<float, 6, 6>;

using vec2i = Eigen::Vector2i;
using vec3i = Eigen::Vector3i;
using vec4i = Eigen::Vector4i;

using vec2d = Eigen::Vector2d;
using vec3d = Eigen::Vector3d;
using vec4d = Eigen::Vector4d;
using vec5d = Eigen::Matrix<double, 5, 1>;
using vec6d = Eigen::Matrix<double, 6, 1>;

using vec2f = Eigen::Vector2f;
using vec3f = Eigen::Vector3f;
using vec4f = Eigen::Vector4f;
using vec5f = Eigen::Matrix<float, 5, 1>;
using vec6f = Eigen::Matrix<float, 6, 1>;

using quad = Eigen::Quaternion<double>;
using quaf = Eigen::Quaternion<float>;

} // namespace liteviz

#endif // __LITEVIZ_COMMON_H__