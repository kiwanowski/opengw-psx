#include "fixed.hh"

namespace gw {


static psyqo::Trig<12> s_trig;

psyqo::Trig<12>& trig() { return s_trig; }


static constexpr Fixed kQuarter = 0.25L;
static constexpr Fixed kC1 = 0.0778873L;
static constexpr Fixed kC2 = 0.0211035L;

static Fixed atanUnitInterval(Fixed z) {
    Fixed az = gw::abs(z);
    return kQuarter * z - z * (az - Fixed(1, 0)) * (kC1 + kC2 * az);
}

Angle atan2(Fixed y, Fixed x) {
    if (x.value == 0 && y.value == 0) return Angle();

    Fixed ax = gw::abs(x);
    Fixed ay = gw::abs(y);

    Fixed result;
    if (ay <= ax) {
        result = atanUnitInterval(y / x);
        if (x.value < 0) result += Fixed(1, 0);
    } else {
        result = Fixed(0.5L) - atanUnitInterval(x / y);
        if (y.value < 0) result += Fixed(1, 0);
    }

    return wrapAngle(Angle(static_cast<int32_t>(result.value >> 2), Angle::RAW));
}

namespace mathutils {


Fixed pointSegmentDistanceSquared(const Vec2& from, const Vec2& to, const Vec2& test) {
    Vec2 seg = to - from;
    Vec2 rel = test - from;

    Fixed lenSq = seg.lengthSquared();
    if (lenSq.value == 0) return rel.lengthSquared();

    Fixed proj = Vec2::dot(seg, rel);

    if (proj.value <= 0) return rel.lengthSquared();
    if (proj >= lenSq) return (test - to).lengthSquared();

    Fixed t = proj / lenSq;
    return (rel - seg * t).lengthSquared();
}

Vec2 closestPointOnLineSegment(const Vec2& from, const Vec2& to, const Vec2& test) {
    Vec2 seg = to - from;
    Fixed lenSq = seg.lengthSquared();
    if (lenSq.value == 0) return from;

    Fixed t = Vec2::dot(seg, test - from) / lenSq;
    if (t.value <= 0) return from;
    if (t >= Fixed(1, 0)) return to;
    return from + seg * t;
}

Fixed pointLineDistance(const Vec2& from, const Vec2& to, const Vec2& test) {
    return distance(closestPointOnLineSegment(from, to, test), test);
}

bool lineCircleIntersects(const Vec2& center, Fixed radius, const Vec2& p1, const Vec2& p2) {
    return pointLineDistance(p1, p2, center) <= radius;
}


static uint32_t s_randState = 0x92d68ca2u;

void seedRandom(uint32_t seed) {
    s_randState = seed ? seed : 0x92d68ca2u;
}

uint32_t random32() {
    uint32_t x = s_randState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    s_randState = x;
    return x;
}

}

}
