#include "spawner.hh"

#include "enemies.hh"
#include "player.hh"
#include "world.hh"

namespace gw {

static constexpr uint32_t kFramesPerIndex = 1250;

static constexpr Fixed kSpawnMargin = 15.0L;

static constexpr Fixed kWaveMargin = 2.0L;

static constexpr Fixed kRushRadius = 40.0L;
static constexpr Fixed kRushRadiusBlackHole = 80.0L;

static constexpr int kSwarmPeriod = 10;
static constexpr int kWaveStartPeriod = 20;

static constexpr int kSpawnCheckPeriod = 100;
static constexpr int kFranticIndex = 10;

int Spawner::spawnIndex() const {
    uint32_t index = m_spawnFrames / kFramesPerIndex;
    return int(index > kMaxSpawnIndex ? kMaxSpawnIndex : index);
}

void Spawner::init() {
    m_spawnFrames = 0;
    m_spawnCheckTimer = 0;
    m_waveStartTimer = 0;
    m_spawnWaitTimer = 50;
    m_corner = 0;
    for (auto& w : m_waves) w.type = WAVE_UNUSED;
}

Fixed Spawner::spawnProgress() const {
    uint32_t raw = (m_spawnFrames * 4096u) / (kFramesPerIndex * kMaxSpawnIndex);
    Fixed p(int32_t(raw), Fixed::RAW);
    return p > Fixed(1, 0) ? Fixed(1, 0) : p;
}

int Spawner::activeWaveCount() const {
    int n = 0;
    for (const auto& w : m_waves) {
        if (w.type != WAVE_UNUSED) n++;
    }
    return n;
}

void Spawner::newWave(WaveType type, Entity::Type entityType, int spawnCount) {
    if (spawnCount <= 0) return;

    for (int i = 0; i < kMaxWaves; i++) {
        if (m_waves[i].type != WAVE_UNUSED) continue;
        m_waves[i].type = type;
        m_waves[i].entityType = entityType;
        m_waves[i].spawnCount = int16_t(spawnCount);
        m_waves[i].timer = 0;
        return;
    }
}

void Spawner::spawnEntities(Enemies& enemies, Entity::Type type, int numWanted) {
    if (enemies.countActiveOfType(type) >= numWanted) return;

    Entity* enemy = enemies.getUnusedEnemyOfType(type);
    if (!enemy) return;

    const Fixed left = kSpawnMargin;
    const Fixed bottom = kSpawnMargin;
    const Fixed right = Fixed(Grid::extentX() - 1, 0) - kSpawnMargin;
    const Fixed top = Fixed(Grid::extentY() - 1, 0) - kSpawnMargin;

    Vec2 spawnPoint(left + mathutils::frand() * (right - left),
                    bottom + mathutils::frand() * (top - bottom));

    Vec2 hitPoint;
    if (g_world.grid.hitTest(spawnPoint, enemy->getRadius(), &hitPoint)) spawnPoint = hitPoint;

    enemy->setPos(spawnPoint);
    enemy->setWaveId(Entity::kNoWave);
    enemy->setState(Entity::STATE_SPAWN_TRANSITION);
}

void Spawner::runTrickle(Enemies& enemies, int index) {
    if (index <= 1) {
        spawnEntities(enemies, Entity::TYPE_WANDERER, 2);
    } else if (index < 3) {
        spawnEntities(enemies, Entity::TYPE_WANDERER, 4);
    }

    spawnEntities(enemies, Entity::TYPE_GRUNT, (index <= 1) ? 2 : 4);

    if (index >= 1) {
        spawnEntities(enemies, Entity::TYPE_SPINNER, 2);
        spawnEntities(enemies, Entity::TYPE_WEAVER, 2);
    }

    if (index >= 2 && mathutils::randFromTo(0, 99) < 4) {
        spawnEntities(enemies, Entity::TYPE_BLACKHOLE, 1);
    }
}

static int originalPoolSize(Entity::Type type) {
    switch (type) {
    case Entity::TYPE_GRUNT: return 200;
    case Entity::TYPE_WANDERER: return 100;
    case Entity::TYPE_WEAVER: return 200;
    case Entity::TYPE_SPINNER: return 100;
    case Entity::TYPE_TINYSPINNER: return 100;
    case Entity::TYPE_MAYFLY: return 400;
    case Entity::TYPE_PROTON: return 200;
    case Entity::TYPE_SNAKE: return 50;
    case Entity::TYPE_BLACKHOLE: return 8;
    case Entity::TYPE_REPULSOR: return 4;
    default: return 1;
    }
}

void Spawner::startWave(Enemies& enemies, int index) {
    Fixed progress = spawnProgress();

    auto sized = [&](Entity::Type type, int floor, bool halve) {
        int n = int((Fixed(originalPoolSize(type), 0) * progress).value + 4095) >> 12;

        if (halve) n /= 2;

        if (n < floor) n = floor;

        const int pool = enemies.poolSize(type);
        if (n > pool) n = pool;
        if (n < 1) n = 1;
        return n;
    };

    switch (mathutils::randFromTo(0, 12)) {
    case 0:
        newWave(WAVE_SWARM, Entity::TYPE_GRUNT, sized(Entity::TYPE_GRUNT, 20, false));
        break;
    case 1:
        newWave(WAVE_SWARM, Entity::TYPE_WEAVER, sized(Entity::TYPE_WEAVER, 20, false));
        break;
    case 2:
        if (index > 4) newWave(WAVE_SWARM, Entity::TYPE_SNAKE, sized(Entity::TYPE_SNAKE, 8, false));
        break;
    case 3:
        newWave(WAVE_SWARM, Entity::TYPE_SPINNER, sized(Entity::TYPE_SPINNER, 20, false));
        break;
    case 4:
        if (index > 4) {
            newWave(WAVE_SWARM, Entity::TYPE_BLACKHOLE, sized(Entity::TYPE_BLACKHOLE, 4, false));
        }
        break;
    case 5:
        if (index > 8) newWave(WAVE_SWARM, Entity::TYPE_MAYFLY, sized(Entity::TYPE_MAYFLY, 50, false));
        break;

    case 6:
        newWave(WAVE_RUSH, Entity::TYPE_GRUNT, sized(Entity::TYPE_GRUNT, 20, true));
        break;
    case 7:
        newWave(WAVE_RUSH, Entity::TYPE_WEAVER, sized(Entity::TYPE_WEAVER, 20, true));
        break;
    case 8:
        if (index > 4) newWave(WAVE_RUSH, Entity::TYPE_SNAKE, sized(Entity::TYPE_SNAKE, 8, true));
        break;
    case 9:
        newWave(WAVE_RUSH, Entity::TYPE_SPINNER, sized(Entity::TYPE_SPINNER, 20, true));
        break;
    case 10:
        break;
    case 11:
        if (index > 4) {
            newWave(WAVE_RUSH, Entity::TYPE_REPULSOR, sized(Entity::TYPE_REPULSOR, 1, true));
        }
        break;
    default:
        break;
    }
}

void Spawner::runWaves(Enemies& enemies) {
    const Fixed left = kWaveMargin;
    const Fixed bottom = kWaveMargin;
    const Fixed right = Fixed(Grid::extentX() - 1, 0) - kWaveMargin;
    const Fixed top = Fixed(Grid::extentY() - 1, 0) - kWaveMargin;

    for (int i = 0; i < kMaxWaves; i++) {
        Wave& wave = m_waves[i];
        if (wave.type == WAVE_UNUSED) continue;

        const uint8_t waveId = uint8_t(i);
        const bool isBlackHole = (wave.entityType == Entity::TYPE_BLACKHOLE);

        if (wave.type == WAVE_RUSH && wave.spawnCount > 0) {
            if (!g_world.player1) continue;
            Vec2 playerPos = g_world.player1->getPos();

            while (wave.spawnCount-- > 0) {
                Entity* enemy = enemies.getUnusedEnemyOfType(wave.entityType);
                if (!enemy) break;

                Fixed radius = isBlackHole ? kRushRadiusBlackHole : kRushRadius;
                Vec2 spawnPoint = playerPos + mathutils::fromAngle(mathutils::randomAngle(), radius);
                spawnPoint.x += mathutils::frand() * Fixed(4, 0) - Fixed(2, 0);
                spawnPoint.y += mathutils::frand() * Fixed(4, 0) - Fixed(2, 0);

                Vec2 hitPoint;
                if (g_world.grid.hitTest(spawnPoint, enemy->getRadius(), &hitPoint)) {
                    spawnPoint = hitPoint;
                }

                enemy->setPos(spawnPoint);
                enemy->setWaveId(waveId);
                enemy->setState(Entity::STATE_SPAWN_TRANSITION);
            }
            wave.spawnCount = 0;

        } else if (wave.type == WAVE_SWARM && wave.spawnCount > 0) {
            if (wave.timer == 0) {
                for (int n = 0; n < 4 && wave.spawnCount > 0; n++) {
                    Entity* enemy = enemies.getUnusedEnemyOfType(wave.entityType);
                    if (!enemy) break;
                    wave.spawnCount--;

                    Fixed jx = mathutils::frand() * Fixed(10, 0) - Fixed(5, 0);
                    Fixed jy = mathutils::frand() * Fixed(10, 0) - Fixed(5, 0);

                    Vec2 spawnPoint;
                    switch (m_corner & 3) {
                    case 0: spawnPoint = Vec2(left + jx, top + jy); break;
                    case 1: spawnPoint = Vec2(right + jx, top + jy); break;
                    case 2: spawnPoint = Vec2(right + jx, bottom + jy); break;
                    default: spawnPoint = Vec2(left + jx, bottom + jy); break;
                    }
                    ++m_corner;

                    Fixed radius = isBlackHole ? Fixed(20, 0) : enemy->getRadius();
                    Fixed lo = radius;
                    Fixed hiX = Fixed(Grid::extentX(), 0) - radius - Fixed(1, 0);
                    Fixed hiY = Fixed(Grid::extentY(), 0) - radius - Fixed(1, 0);

                    if (spawnPoint.x < lo) spawnPoint.x = lo;
                    else if (spawnPoint.x > hiX) spawnPoint.x = hiX;
                    if (spawnPoint.y < lo) spawnPoint.y = lo;
                    else if (spawnPoint.y > hiY) spawnPoint.y = hiY;

                    Vec2 hitPoint;
                    if (g_world.grid.hitTest(spawnPoint, enemy->getRadius(), &hitPoint)) {
                        spawnPoint = hitPoint;
                    }

                    enemy->setPos(spawnPoint);
                    enemy->setWaveId(waveId);
                    enemy->setState(Entity::STATE_SPAWN_TRANSITION);
                }
            }

            if (++wave.timer > kSwarmPeriod) wave.timer = 0;
        }

        if (wave.spawnCount <= 0 && !enemies.anyAliveFromWave(waveId)) {
            wave.type = WAVE_UNUSED;
        }
    }
}

void Spawner::run(Enemies& enemies, bool playerActive, bool bombing) {
    if (spawnIndex() < kMaxSpawnIndex) ++m_spawnFrames;

    if (m_spawnWaitTimer > 0) --m_spawnWaitTimer;

    if (!playerActive) {
        m_spawnCheckTimer = 0;
        m_waveStartTimer = 0;
        return;
    }

    if (m_spawnWaitTimer > 0 || bombing) {
        m_spawnCheckTimer = 0;
        return;
    }

    const int index = spawnIndex();

    if (++m_spawnCheckTimer > kSpawnCheckPeriod || index > kFranticIndex) {
        m_spawnCheckTimer = 0;
        runTrickle(enemies, index);
    }

    int wavesAllowed = 0;
    if (index > 1) wavesAllowed = 1;
    if (index > 12) wavesAllowed = 2;
    if (index > 20) wavesAllowed = kMaxWaves;

    if (++m_waveStartTimer >= kWaveStartPeriod && activeWaveCount() < wavesAllowed) {
        m_waveStartTimer = 0;
        startWave(enemies, index);
    }

    runWaves(enemies);
}

}
