#include "models.hh"

namespace gw {
namespace models {



static const Vec2 kPlayerVerts[] = {
    {Fixed(0.0L), Fixed(-1.0L)},   {Fixed(1.0L), Fixed(-0.15L)},
    {Fixed(0.5L), Fixed(0.7L)},    {Fixed(0.72L), Fixed(0.02L)},
    {Fixed(0.0L), Fixed(-0.4L)},   {Fixed(-0.72L), Fixed(0.02L)},
    {Fixed(-0.5L), Fixed(0.7L)},   {Fixed(-1.0L), Fixed(-0.15L)},
};

static const Edge kPlayerEdges[] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 0},
};

const Model player = {kPlayerVerts, kPlayerEdges, 8, 8};


static const Vec2 kMissileVerts[] = {
    {Fixed(0.0L), Fixed(7.5L)},
    {Fixed(2.0L), Fixed(-5.2L)},
    {Fixed(-2.0L), Fixed(-5.2L)},
};

static const Edge kMissileEdges[] = {{0, 1}, {1, 2}, {2, 0}};

const Model missile = {kMissileVerts, kMissileEdges, 3, 3};


static const Vec2 kGruntVerts[] = {
    {Fixed(0.0L), Fixed(1.0L)},
    {Fixed(1.0L), Fixed(0.0L)},
    {Fixed(0.0L), Fixed(-1.0L)},
    {Fixed(-1.0L), Fixed(0.0L)},
};

static const Edge kGruntEdges[] = {{0, 1}, {1, 2}, {2, 3}, {3, 0}};

const Model grunt = {kGruntVerts, kGruntEdges, 4, 4};


static const Vec2 kWandererVerts[] = {
    {Fixed(0.0L), Fixed(0.0L)},   {Fixed(0.0L), Fixed(1.0L)},
    {Fixed(1.0L), Fixed(1.0L)},   {Fixed(1.0L), Fixed(0.0L)},
    {Fixed(1.0L), Fixed(-1.0L)},  {Fixed(0.0L), Fixed(-1.0L)},
    {Fixed(-1.0L), Fixed(-1.0L)}, {Fixed(-1.0L), Fixed(0.0L)},
    {Fixed(-1.0L), Fixed(1.0L)},
};

static const Vec2 kWandererFlippedVerts[] = {
    {Fixed(0.0L), Fixed(0.0L)},   {Fixed(1.0L), Fixed(0.0L)},
    {Fixed(1.0L), Fixed(1.0L)},   {Fixed(0.0L), Fixed(1.0L)},
    {Fixed(-1.0L), Fixed(1.0L)},  {Fixed(-1.0L), Fixed(0.0L)},
    {Fixed(-1.0L), Fixed(-1.0L)}, {Fixed(0.0L), Fixed(-1.0L)},
    {Fixed(1.0L), Fixed(-1.0L)},
};

static const Edge kWandererEdges[] = {
    {0, 1}, {1, 2}, {2, 0}, {0, 3}, {3, 4}, {4, 0},
    {0, 5}, {5, 6}, {6, 0}, {0, 7}, {7, 8}, {8, 0},
};

const Model wanderer = {kWandererVerts, kWandererEdges, 9, 12};
const Model wandererFlipped = {kWandererFlippedVerts, kWandererEdges, 9, 12};


static const Vec2 kWeaverVerts[] = {
    {Fixed(0.0L), Fixed(1.0L)},   {Fixed(1.0L), Fixed(0.0L)},
    {Fixed(0.0L), Fixed(-1.0L)},  {Fixed(-1.0L), Fixed(0.0L)},
    {Fixed(-1.0L), Fixed(1.0L)},  {Fixed(1.0L), Fixed(1.0L)},
    {Fixed(1.0L), Fixed(-1.0L)},  {Fixed(-1.0L), Fixed(-1.0L)},
};

static const Edge kWeaverEdges[] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4},
};

const Model weaver = {kWeaverVerts, kWeaverEdges, 8, 8};


static const Vec2 kSpinnerVerts[] = {
    {Fixed(0.0L), Fixed(0.0L)},
    {Fixed(-1.0L), Fixed(1.0L)},
    {Fixed(1.0L), Fixed(1.0L)},
    {Fixed(1.0L), Fixed(-1.0L)},
    {Fixed(-1.0L), Fixed(-1.0L)},
};

static const Edge kSpinnerEdges[] = {
    {1, 0}, {0, 3}, {4, 0}, {0, 2}, {2, 3}, {3, 4}, {4, 1}, {1, 2},
};

const Model spinner = {kSpinnerVerts, kSpinnerEdges, 5, 8};


static const Vec2 kMayflyVerts[] = {
    {Fixed(-0.25L), Fixed(1.25L)}, {Fixed(0.25L), Fixed(1.25L)},
    {Fixed(1.2L), Fixed(-0.5L)},   {Fixed(1.0L), Fixed(-0.9L)},
    {Fixed(-1.0L), Fixed(-0.9L)},  {Fixed(-1.2L), Fixed(-0.5L)},
};

static const Edge kMayflyEdges[] = {{0, 3}, {1, 4}, {2, 5}};

const Model mayfly = {kMayflyVerts, kMayflyEdges, 6, 3};


static const Vec2 kProtonVerts[] = {
    {Fixed(1.2L), Fixed(0.0L)},        {Fixed(0.84853L), Fixed(0.84853L)},
    {Fixed(0.0L), Fixed(1.2L)},        {Fixed(-0.84853L), Fixed(0.84853L)},
    {Fixed(-1.2L), Fixed(0.0L)},       {Fixed(-0.84853L), Fixed(-0.84853L)},
    {Fixed(0.0L), Fixed(-1.2L)},       {Fixed(0.84853L), Fixed(-0.84853L)},
};

static const Edge kProtonEdges[] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 0},
};

const Model proton = {kProtonVerts, kProtonEdges, 8, 8};


static const Vec2 kSnakeHeadVerts[] = {
    {Fixed(0.0L), Fixed(1.53L)},      {Fixed(0.4L), Fixed(1.404L)},
    {Fixed(0.71L), Fixed(1.0575L)},   {Fixed(0.9L), Fixed(0.558L)},
    {Fixed(0.9L), Fixed(0.135L)},     {Fixed(0.68L), Fixed(-0.288L)},
    {Fixed(0.46L), Fixed(-0.513L)},   {Fixed(0.0L), Fixed(0.0L)},
    {Fixed(-0.46L), Fixed(-0.513L)},  {Fixed(-0.68L), Fixed(-0.288L)},
    {Fixed(-0.9L), Fixed(0.135L)},    {Fixed(-0.9L), Fixed(0.558L)},
    {Fixed(-0.71L), Fixed(1.0575L)},  {Fixed(-0.4L), Fixed(1.404L)},
};

static const Edge kSnakeHeadEdges[] = {
    {0, 1},   {1, 2},   {2, 3},   {3, 4},  {4, 5},   {5, 6},  {6, 7},
    {7, 8},   {8, 9},   {9, 10},  {10, 11}, {11, 12}, {12, 13}, {13, 0},
};

const Model snakeHead = {kSnakeHeadVerts, kSnakeHeadEdges, 14, 14};

static const Vec2 kSnakeSegVerts[] = {
    {Fixed(0.0L), Fixed(0.9L)},
    {Fixed(0.6L), Fixed(-0.6L)},
    {Fixed(-0.6L), Fixed(-0.6L)},
};

static const Edge kSnakeSegEdges[] = {{0, 1}, {1, 2}, {2, 0}};

const Model snakeSegment = {kSnakeSegVerts, kSnakeSegEdges, 3, 3};


static const Vec2 kRepulsorVerts[] = {
    {Fixed(8.0L), Fixed(-1.0L)},    {Fixed(8.0L), Fixed(8.0L)},
    {Fixed(11.5L), Fixed(12.0L)},   {Fixed(15.0L), Fixed(8.0L)},
    {Fixed(15.0L), Fixed(-5.0L)},   {Fixed(5.0L), Fixed(-14.0L)},
    {Fixed(-5.0L), Fixed(-14.0L)},  {Fixed(-15.0L), Fixed(-5.0L)},
    {Fixed(-15.0L), Fixed(8.0L)},   {Fixed(-11.5L), Fixed(12.0L)},
    {Fixed(-8.0L), Fixed(8.0L)},    {Fixed(-8.0L), Fixed(-1.0L)},
};

static const Edge kRepulsorEdges[] = {
    {0, 1}, {1, 2}, {2, 3},  {3, 4},   {4, 5},   {5, 6},
    {6, 7}, {7, 8}, {8, 9},  {9, 10},  {10, 11}, {11, 0},
};

const Model repulsor = {kRepulsorVerts, kRepulsorEdges, 12, 12};

static const Vec2 kRepulsorShieldVerts[] = {
    {Fixed(-12.0L), Fixed(20.0L)},
    {Fixed(12.0L), Fixed(20.0L)},
};

static const Edge kRepulsorShieldEdges[] = {{0, 1}};

const Model repulsorShield = {kRepulsorShieldVerts, kRepulsorShieldEdges, 2, 1};


static const Vec2 kShieldVerts[] = {
    {Fixed(0.0L), Fixed(11.5L)},   {Fixed(8.1L), Fixed(8.1L)},
    {Fixed(11.5L), Fixed(0.0L)},   {Fixed(8.1L), Fixed(-8.1L)},
    {Fixed(0.0L), Fixed(-11.5L)},  {Fixed(-8.1L), Fixed(-8.1L)},
    {Fixed(-11.5L), Fixed(0.0L)},  {Fixed(-8.1L), Fixed(8.1L)},
    {Fixed(0.0L), Fixed(4.0L)},    {Fixed(0.0L), Fixed(-4.0L)},
    {Fixed(-4.0L), Fixed(0.0L)},   {Fixed(4.0L), Fixed(0.0L)},
};

static const Edge kShieldEdges[] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 0}, {8, 9}, {10, 11},
};

const Model shieldSymbol = {kShieldVerts, kShieldEdges, 12, 10};

}
}
