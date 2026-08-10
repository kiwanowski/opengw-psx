
#pragma once

#include <stdint.h>

#include "fixed.hh"

namespace gw {
namespace font {

static constexpr int kFirstChar = 32;
static constexpr int kLastChar = 126;

struct Glyph {
    uint16_t offset;
    uint8_t segments;
};

extern const Vec2 kFontVerts[364];
extern const Glyph kGlyphs[95];

}
}
