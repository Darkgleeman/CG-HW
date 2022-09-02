#ifndef UTILS_H
#define UTILS_H

// Includes

#include <algorithm>
#include <cassert>
#include <cmath>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <omp.h>

// Definitions

static_assert(sizeof(float) == 4); // TODO: 根据不同 size 的 float 定义不同的 floatAligenInt，一刀切不太好的啊
typedef float Float;

#ifndef NDEBUG
    #define Assert(expression, msg) if (!(expression)) { Error((msg)); assert(false); }
#else
    #define Assert(expression, msg) assert((expression));
#endif

// Usings

#ifndef M_PI
#define M_PI 3.14159265358979323846 /* pi */
#endif
#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923 /* pi/2 */
#endif
#ifndef M_PI_4
#define M_PI_4 0.78539816339744830962 /* pi/4 */
#endif

constexpr Float Pi      = Float(M_PI);
constexpr Float PiInv   = Float((long double)1 / (long double)M_PI);
constexpr Float Pix2    = Float((long double)M_PI * 2);
constexpr Float Pix2Inv = Float((long double)1 / ((long double)M_PI * 2));
constexpr Float Pix4    = Float((long double)M_PI * 4);
constexpr Float Pix4Inv = Float((long double)1 / ((long double)M_PI * 4));
constexpr Float Pi_2    = Float(M_PI_2);
constexpr Float Pi_4    = Float(M_PI_4);
constexpr Float Pi_180  = Float((long double)M_PI / (long double)180);

constexpr Float Infinity      = std::numeric_limits<Float>::infinity();
constexpr Float Epsilon       = std::numeric_limits<Float>::epsilon();
constexpr Float Epsilonx10    = std::numeric_limits<Float>::epsilon() * 10;
constexpr Float Epsilonx100   = std::numeric_limits<Float>::epsilon() * 100;
constexpr Float Epsilonx1000  = std::numeric_limits<Float>::epsilon() * 1000;
constexpr Float Epsilonx10000 = std::numeric_limits<Float>::epsilon() * 10000;

constexpr int IntMax = std::numeric_limits<int>::max();
constexpr int IntMin = std::numeric_limits<int>::min();

using std::make_shared;
using std::shared_ptr;
using std::vector;
using std::string;
using std::pair;

// Utility functions

constexpr Float MachineEpsilon = Epsilon * 0.5;
inline constexpr Float Gamma(int n)
{
    return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
}

inline uint32_t LeftShift3(uint32_t x)
{
    if (x == (1 << 10)) --x;
    x = (x | (x << 16)) & 0b00000011000000000000000011111111;
    x = (x | (x <<  8)) & 0b00000011000000001111000000001111;
    x = (x | (x <<  4)) & 0b00000011000011000011000011000011;
    x = (x | (x <<  2)) & 0b00001001001001001001001001001001;
    return x;
}

inline uint32_t EncodeMorton3(Float x, Float y, Float z)
{
    return (LeftShift3(z) << 2) | (LeftShift3(y) << 1) | (LeftShift3(x));
}

inline int RoundUpToPower2(int num)
{
    int n = num - 1;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16; // 假定输入不会超过 int32
    return n + 1; //! Unsafe
}

inline int GetOrderPower2(int num)
{
    int count = 0;
    while (num > 1)
    {
        num = num >> 1;
        ++count;
    }
    return count;
}

inline bool IsPower2(int num)
{
    return (num & (num - 1)) == 0;
}

void NotImplemented();

inline Float Radians(Float degrees) { return degrees * Pi_180; }

template <typename T>
void Print(const T& value) { std::cout << (value) << std::endl; }

template <typename T>
void Error(const T& value)
{
    #pragma omp critical
    {
        std::cerr << (value) << std::endl;
    }
}

template <typename T1, typename T2>
void Error(const T1& value1, const T2& value2)
{
    #pragma omp critical
    {
        std::cerr << (value1) << "    " << (value2) << std::endl;
    }
}

template <typename T1, typename T2, typename T3>
void Error(const T1& value1, const T2& value2, const T3& value3)
{
    #pragma omp critical
    {
        std::cerr << (value1) << "    " << (value2) << "    " << (value3) << std::endl;
    }
}

template <typename T1, typename T2, typename T3, typename T4>
void Error(const T1& value1, const T2& value2, const T3& value3, const T4& value4)
{
    #pragma omp critical
    {
        std::cerr << (value1) << "    " << (value2) << "    " << (value3) << "    " << (value4) << std::endl;
    }
}

template <typename T>
void Write(const T& value) { std::cout << (value); }

template <typename T>
inline T Lerp(const T& a, const T& b, Float t) { return a + (b - a) * t; }

template <typename T>
inline T Lerp(const T& v00, const T& v01, const T& v10, const T& v11, Float u, Float v)
{
    Float uv = u * v;
    Float u_sub_uv = u - uv;
    return (1 - u_sub_uv - v) * v00 + u_sub_uv * v01 + (v - uv) * v10 + uv * v11;
}

template <typename T>
inline T Lerp(
        const T& v000, const T& v001, const T& v010, const T& v011, 
        const T& v100, const T& v101, const T& v110, const T& v111, 
        Float u, Float v, Float w)
{
    return Lerp(
        Lerp(v000, v001, v010, v011, u, v), 
        Lerp(v100, v101, v110, v111, u, v), 
        w);
}

template <typename T>
inline T Min(const T& a, const T& b) { return std::min(a, b); }

template <typename T>
inline T Min(const T& a, const T& b, const T& c) { return std::min(a, std::min(b, c)); }

template <typename T>
inline T Max(const T& a, const T& b) { return std::max(a, b); }

template <typename T>
inline T Max(const T& a, const T& b, const T& c) { return std::max(a, std::max(b, c)); }

std::string GetFilePath(const string& target, int depth = 5);

#endif