
#pragma once

#include "bomb.hh"
#include "enemies.hh"
#include "fixed.hh"
#include "grid.hh"
#include "particles.hh"
#include "pointdisplay.hh"
#include "render.hh"
#include "spawner.hh"

namespace gw {

class Player;

struct Camera {
    Vec2 currentPos;
    Vec2 targetPos;
    Fixed currentZoom = Fixed(46, 0);
    Fixed targetZoom = Fixed(46, 0);

    static constexpr Fixed kZoomedIn = 46.0L;
    static constexpr Fixed kZoomedOut = 72.0L;

    void center();

    void follow(const Vec2& pos);

    void run();
};

struct World {
    Grid grid;
    Renderer renderer;
    Camera camera;
    Enemies enemies;
    Spawner spawner;
    Particles particles;
    Bomb bomb;
    PointDisplays pointDisplays;

    Player* player1 = nullptr;

    Fixed brightness = Fixed(1, 0);
};

extern World g_world;

}
