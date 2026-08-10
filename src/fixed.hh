
#pragma once

#include <stdint.h>

#include "psyqo/fixed-point.hh"
#include "psyqo/trigonometry.hh"

namespace gw {

using Fixed = psyqo::FixedPoint<12>;
using Angle = psyqo::Angle;

static constexpr long double kPi = 3.14159265358979323846L;

consteval Angle rad(long double radians) { return Angle(radians / kPi); }

static constexpr int32_t kTurnRaw = 2 << 10;


constexpr uint32_t isqrt64(uint64_t n) {
    if (n == 0) return 0;

    int rounds = 32;
    while ((n >> 62) == 0) {
        n <<= 2;
        rounds--;
    }

    uint64_t rem = 0, root = 0;
    for (int i = 0; i < rounds; i++) {
        root <<= 1;
        rem = (rem << 2) | (n >> 62);
        n <<= 2;
        if (root < rem) {
            rem -= root | 1;
            root |= 2;
        }
    }
    return static_cast<uint32_t>(root >> 1);
}

inline Fixed sqrt(Fixed v) {
    if (v.value <= 0) return Fixed();
    return Fixed(static_cast<int32_t>(isqrt64(static_cast<uint64_t>(v.value) << 12)), Fixed::RAW);
}

inline Fixed abs(Fixed v) { return v.value < 0 ? -v : v; }

inline Fixed approxLength(Fixed x, Fixed y) {
    static constexpr Fixed kAlpha = 0.96043387L;
    static constexpr Fixed kBeta = 0.39782473L;

    Fixed ax = gw::abs(x);
    Fixed ay = gw::abs(y);
    Fixed hi = ax.value > ay.value ? ax : ay;
    Fixed lo = ax.value > ay.value ? ay : ax;
    return kAlpha * hi + kBeta * lo;
}

inline Fixed min(Fixed a, Fixed b) { return a.value < b.value ? a : b; }
inline Fixed max(Fixed a, Fixed b) { return a.value > b.value ? a : b; }

inline Angle wrapAngle(Angle a) {
    int32_t t = a.value % kTurnRaw;
    if (t < 0) t += kTurnRaw;
    return Angle(t, Angle::RAW);
}

inline Angle scaleAngle(Angle a, Fixed factor) {
    return Angle(static_cast<int32_t>((static_cast<int64_t>(a.value) * factor.value) >> 12), Angle::RAW);
}

inline Angle diffAngles(Angle a, Angle b) {
    int32_t d = wrapAngle(a).value - wrapAngle(b).value;
    if (d >= kTurnRaw / 2) {
        d -= kTurnRaw;
    } else if (d <= -kTurnRaw / 2) {
        d += kTurnRaw;
    }
    return Angle(d, Angle::RAW);
}


psyqo::Trig<12>& trig();

inline Fixed sin(Angle a) { return trig().sin(a); }
inline Fixed cos(Angle a) { return trig().cos(a); }

Angle atan2(Fixed y, Fixed x);


struct Vec2 {
    Fixed x, y;

    constexpr Vec2() = default;
    constexpr Vec2(Fixed x_, Fixed y_) : x(x_), y(y_) {}

    constexpr Vec2 operator+(const Vec2& p) const { return Vec2(x + p.x, y + p.y); }
    constexpr Vec2 operator-(const Vec2& p) const { return Vec2(x - p.x, y - p.y); }
    constexpr Vec2 operator-() const { return Vec2(-x, -y); }
    constexpr Vec2 operator*(Fixed f) const { return Vec2(x * f, y * f); }
    constexpr Vec2 operator*(const Vec2& p) const { return Vec2(x * p.x, y * p.y); }
    constexpr Vec2 operator/(Fixed f) const { return Vec2(x / f, y / f); }

    constexpr Vec2& operator+=(const Vec2& p) { x += p.x; y += p.y; return *this; }
    constexpr Vec2& operator-=(const Vec2& p) { x -= p.x; y -= p.y; return *this; }
    constexpr Vec2& operator*=(Fixed f) { x = x * f; y = y * f; return *this; }
    constexpr Vec2& operator*=(const Vec2& p) { x = x * p.x; y = y * p.y; return *this; }

    constexpr bool operator==(const Vec2& p) const { return x == p.x && y == p.y; }

    static constexpr Fixed dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }

    constexpr Fixed lengthSquared() const { return x * x + y * y; }

    Fixed length() const { return gw::sqrt(lengthSquared()); }

    Vec2 normalized() const {
        Fixed len = length();
        if (len.value == 0) return Vec2();
        return Vec2(x / len, y / len);
    }
};


namespace mathutils {

inline Fixed distanceSquared(const Vec2& a, const Vec2& b) { return (a - b).lengthSquared(); }
inline Fixed distance(const Vec2& a, const Vec2& b) { return (a - b).length(); }

inline Angle angleTo(const Vec2& from, const Vec2& to) {
    Vec2 d = to - from;
    return gw::atan2(d.y, d.x);
}

inline Vec2 rotate(const Vec2& p, Angle a) {
    Fixed c = gw::cos(a);
    Fixed s = gw::sin(a);
    return Vec2(p.x * c - p.y * s, p.x * s + p.y * c);
}

inline Vec2 fromAngle(Angle a, Fixed distance) {
    return Vec2(gw::cos(a) * distance, gw::sin(a) * distance);
}

inline Vec2 clamp(const Vec2& v, Fixed maxLen) {
    Fixed lenSq = v.lengthSquared();
    if (lenSq <= maxLen * maxLen) return v;
    Fixed len = gw::sqrt(lenSq);
    if (len.value == 0) return v;
    return v * (maxLen / len);
}

Fixed pointSegmentDistanceSquared(const Vec2& from, const Vec2& to, const Vec2& test);

Vec2 closestPointOnLineSegment(const Vec2& from, const Vec2& to, const Vec2& test);
Fixed pointLineDistance(const Vec2& from, const Vec2& to, const Vec2& test);
bool lineCircleIntersects(const Vec2& center, Fixed radius, const Vec2& p1, const Vec2& p2);


void seedRandom(uint32_t seed);
uint32_t random32();

inline Fixed frand() { return Fixed(static_cast<int32_t>(random32() & 0xfff), Fixed::RAW); }

inline int randFromTo(int from, int to) {
    int range = to - from + 1;
    if (range <= 0) return from;
    return from + static_cast<int>(random32() % static_cast<uint32_t>(range));
}

inline Angle randomAngle() {
    return Angle(static_cast<int32_t>(random32() % static_cast<uint32_t>(kTurnRaw)), Angle::RAW);
}

}

}
