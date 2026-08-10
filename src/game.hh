
#pragma once

#include "controls.hh"
#include "player.hh"
#include "render.hh"

namespace gw {

class Game {
  public:
    void init(Player* player);

    void run(const Controls& controls);
    void draw(Renderer& r);

    int score() const;

  private:
    void runCollisions();

    Player* m_player = nullptr;
};

}
