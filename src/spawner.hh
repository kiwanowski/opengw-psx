
#pragma once

#include <stdint.h>

#include "entity.hh"
#include "fixed.hh"

namespace gw {

class Enemies;

static constexpr int kMaxSpawnIndex = 40;

static constexpr int kMaxWaves = 8;

class Spawner {
  public:
    void init();

    void run(Enemies& enemies, bool playerActive, bool bombing);

    int spawnIndex() const;

    Fixed spawnProgress() const;

  private:
    enum WaveType : uint8_t { WAVE_UNUSED = 0, WAVE_SWARM, WAVE_RUSH };

    struct Wave {
        WaveType type = WAVE_UNUSED;
        Entity::Type entityType = Entity::TYPE_UNDEF;
        int16_t spawnCount = 0;
        int16_t timer = 0;
    };

    void spawnEntities(Enemies& enemies, Entity::Type type, int numWanted);

    void runTrickle(Enemies& enemies, int index);

    void startWave(Enemies& enemies, int index);

    void runWaves(Enemies& enemies);

    void newWave(WaveType type, Entity::Type entityType, int spawnCount);
    int activeWaveCount() const;

    uint32_t m_spawnFrames = 0;

    int16_t m_spawnCheckTimer = 0;
    int16_t m_waveStartTimer = 0;
    int16_t m_spawnWaitTimer = 0;

    uint8_t m_corner = 0;

    Wave m_waves[kMaxWaves];
};

}
