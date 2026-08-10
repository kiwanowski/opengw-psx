#include "enemies.hh"

#include "models.hh"
#include "player.hh"
#include "world.hh"

namespace gw {

static constexpr Angle kHalfTurn = 1.0L;


static constexpr Fixed kGruntAccel = 0.01L;
static constexpr Fixed kGruntMaxSpeed = 0.3L;
static constexpr Fixed kGruntFriction = 0.99L;
static constexpr Angle kGruntAnimRate = 0.0223L;

Grunt::Grunt() {
    m_type = TYPE_GRUNT;
    m_model = &models::grunt;
    m_scale = Vec2(Fixed(1.5L), Fixed(1.5L));
    m_radius = Fixed(3, 0);
    m_scoreValue = 50;
    m_pen = Pen(128, 255, 255, 179, 1);
    m_state = STATE_INACTIVE;
}

void Grunt::run() {
    if (getEnabled() && g_world.player1) {
        Angle angle = mathutils::angleTo(m_pos, g_world.player1->getPos());
        m_speed += mathutils::fromAngle(angle, kGruntAccel);
        m_speed = mathutils::clamp(m_speed, kGruntMaxSpeed * m_aggression);
        m_speed *= kGruntFriction;

        m_animationIndex = wrapAngle(m_animationIndex + kGruntAnimRate);
        m_scale.x = Fixed(2, 0) + gw::sin(m_animationIndex) * Fixed(0.4L);
        m_scale.y = Fixed(2.5L) + gw::sin(-m_animationIndex) * Fixed(0.4L);
    }
    Entity::run();
}

void Grunt::spawnTransition() {
    Entity::spawnTransition();
    m_scale = Vec2(Fixed(2, 0), Fixed(2.5L));
}


static constexpr Fixed kWandererAccel = 0.016L;
static constexpr Fixed kWandererMaxSpeed = 0.2L;
static constexpr Fixed kWandererFriction = 0.98L;

Wanderer::Wanderer() {
    m_type = TYPE_WANDERER;
    m_scale = Vec2(Fixed(1.7L), Fixed(1.7L));
    m_radius = Fixed(2.6L);
    m_scoreValue = 25;
    m_edgeBounce = true;
    m_pen = Pen(166, 128, 255, 179, 1);
    m_model = &models::wanderer;
    m_state = STATE_INACTIVE;
}

void Wanderer::run() {
    if (getEnabled()) {
        if (mathutils::randFromTo(0, 39) == 0) {
            static constexpr Angle kVariation = 0.2387L;
            Angle jitter = Angle(int32_t(mathutils::random32() % uint32_t(kVariation.value * 2)), Angle::RAW);
            m_currentHeading = wrapAngle(m_currentHeading + jitter - kVariation);
        }

        m_speed += mathutils::fromAngle(m_currentHeading, kWandererAccel);
        m_speed = mathutils::clamp(m_speed, kWandererMaxSpeed);
        m_speed *= kWandererFriction;

        if (g_world.grid.hitTest(m_pos, m_radius * 2)) {
            m_currentHeading = mathutils::randomAngle();
        }
    }
    Entity::run();
}

void Wanderer::spawnTransition() {
    Entity::spawnTransition();

    m_speed = Vec2();
    m_angle = Angle();
    m_currentHeading = mathutils::randomAngle();

    m_flipped = (mathutils::random32() & 1) != 0;
    m_model = m_flipped ? &models::wandererFlipped : &models::wanderer;
    m_rotationRate = m_flipped ? Angle(-0.0382L) : Angle(0.0382L);
}


static constexpr Fixed kWeaverAccel = 0.05L;
static constexpr Fixed kWeaverMaxSpeed = 0.4L;
static constexpr Fixed kWeaverFriction = 0.95L;
static constexpr Fixed kWeaverDodgeRange = 25.0L;
static constexpr Fixed kWeaverDodgeForce = 0.06L;

Weaver::Weaver() {
    m_type = TYPE_WEAVER;
    m_model = &models::weaver;
    m_scale = Vec2(Fixed(1.5L), Fixed(1.5L));
    m_radius = Fixed(2.4L);
    m_scoreValue = 100;
    m_pen = Pen(77, 255, 89, 128, 1);
    m_state = STATE_INACTIVE;
}

void Weaver::run() {
    Player* player = g_world.player1;

    if (getEnabled() && player) {
        static constexpr Fixed kCosSqTolerance = 0.2919L;

        PlayerMissile* const* live = player->liveMissiles();
        const int liveCount = player->liveMissileCount();

        for (int i = 0; i < liveCount; i++) {
            const PlayerMissile& m = *live[i];

            Vec2 delta = m_pos - m.getPos();
            Fixed distSq = delta.lengthSquared();
            if (distSq >= kWeaverDodgeRange * kWeaverDodgeRange) continue;

            const Vec2& vel = m.getSpeed();
            Fixed dot = Vec2::dot(vel, delta);
            if (dot.value <= 0) continue;

            if (dot * dot <= vel.lengthSquared() * distSq * kCosSqTolerance) continue;

            Fixed cross = vel.x * delta.y - vel.y * delta.x;

            Angle away = mathutils::angleTo(m_pos, player->getPos()) + kHalfTurn;
            Vec2 move(Fixed(0.8L), cross.value > 0 ? Fixed(1.1L) : Fixed(-1.1L));
            m_drift += mathutils::clamp(mathutils::rotate(move, away), kWeaverDodgeForce);
        }

        Angle angle = mathutils::angleTo(m_pos, player->getPos());
        m_speed += mathutils::clamp(mathutils::fromAngle(angle, Fixed(1, 0)), kWeaverAccel);
        m_speed = mathutils::clamp(m_speed, kWeaverMaxSpeed * m_aggression);
        m_speed *= kWeaverFriction;
    }

    Entity::run();
}

void Weaver::spawnTransition() {
    Entity::spawnTransition();
    m_rotationRate = Angle(-0.0191L);
}


static constexpr Fixed kSpinnerAccel = 0.2L;
static constexpr Fixed kSpinnerMaxSpeed = 0.4L;
static constexpr Fixed kSpinnerFriction = 0.9L;
static constexpr Angle kSpinnerAnimRate = 0.0382L;

static constexpr Fixed kFragmentOffset = 12.0L;
static constexpr Fixed kFragmentSpeedScale = 1.4L;

Spinner::Spinner() {
    m_type = TYPE_SPINNER;
    m_model = &models::spinner;
    m_scale = Vec2(Fixed(1.6L), Fixed(1.6L));
    m_radius = Fixed(2.5L);
    m_scoreValue = 100;
    m_pen = Pen(255, 128, 255, 179, 1);
    m_state = STATE_INACTIVE;
}

void Spinner::run() {
    if (getEnabled() && g_world.player1) {
        Angle angle = mathutils::angleTo(m_pos, g_world.player1->getPos());
        m_speed += mathutils::fromAngle(angle, kSpinnerAccel);
        m_speed = mathutils::clamp(m_speed, kSpinnerMaxSpeed * m_aggression);
        m_speed *= kSpinnerFriction;

        m_animationIndex = wrapAngle(m_animationIndex + kSpinnerAnimRate);
        m_angle = Angle(int32_t(gw::sin(m_animationIndex).value >> 3), Angle::RAW);
    }
    Entity::run();
}

void Spinner::spawnTransition() {
    Entity::spawnTransition();
    m_hitSpeed = Vec2();
}


static constexpr Fixed kTinyAccel = 0.02L;
static constexpr Fixed kTinyMaxSpeed = 0.2L;
static constexpr Fixed kTinyOrbitRadius = 5.0L;

TinySpinner::TinySpinner() {
    m_type = TYPE_TINYSPINNER;
    m_model = &models::spinner;
    m_scale = Vec2(Fixed(1, 0), Fixed(1, 0));
    m_radius = Fixed(2.2L);
    m_scoreValue = 50;
    m_pen = Pen(255, 128, 255, 128, 1);
    m_state = STATE_INACTIVE;
}

void TinySpinner::run() {
    if (getEnabled() && g_world.player1) {
        Angle angle = mathutils::angleTo(m_pos, g_world.player1->getPos());
        m_speed += mathutils::fromAngle(angle, kTinyAccel);
        m_speed = mathutils::clamp(m_speed, kTinyMaxSpeed * m_aggression);
        m_speed *= kSpinnerFriction;

        m_animationIndex = wrapAngle(m_animationIndex + kSpinnerAnimRate);
        m_virtualPos = m_pos + mathutils::fromAngle(m_animationIndex, kTinyOrbitRadius);
        m_angle = wrapAngle(m_animationIndex * 2);

        Fixed maxX(kGridExtentX - 1, 0), maxY(kGridExtentY - 1, 0);
        if (m_virtualPos.x.value < 0) m_virtualPos.x = Fixed();
        else if (m_virtualPos.x > maxX) m_virtualPos.x = maxX;
        if (m_virtualPos.y.value < 0) m_virtualPos.y = Fixed();
        else if (m_virtualPos.y > maxY) m_virtualPos.y = maxY;
    }
    Entity::run();
}

void TinySpinner::spawnTransition() {
    Entity::spawnTransition();
    m_drift = m_initialSpeed;
    m_initialSpeed = Vec2();
    m_virtualPos = m_pos;
}

void TinySpinner::spawn() {
    Entity::spawn();

    if (m_stateTimer > 0) {
        Fixed c = Fixed(m_stateTimer, 0) / (m_spawnTime ? m_spawnTime : 1);
        uint8_t green = uint8_t(128 + int((Fixed(127, 0) * c).value >> 12));
        m_pen = Pen(255, green, 255, 128, 1);
        run();
    } else {
        m_pen = Pen(255, 128, 255, 128, 1);
    }
}

void TinySpinner::draw(Renderer& r) {
    if (!m_model) return;

    if (m_state == STATE_INDICATING) {
        if ((m_stateTimer / 10) & 1) m_model->draw(r, m_virtualPos, m_angle, m_scale, m_pen);
        return;
    }
    if (!getEnabled()) return;

    m_model->draw(r, m_virtualPos, m_angle, m_scale, m_pen);
}


static constexpr Fixed kMayflyAccel = 0.5L;
static constexpr Fixed kMayflyMaxSpeed = 0.45L;
static constexpr Fixed kMayflyFriction = 0.9L;

static constexpr int kMayflyFlipPeriod = 15;
static constexpr Fixed kMayflyTargetJitter = 30.0L;

static constexpr Angle kMayflyBank = 0.382L;
static constexpr Fixed kMayflyBankRate = 0.03L;
static constexpr Fixed kMayflyBankDamping = 0.9L;

Mayfly::Mayfly() {
    m_type = TYPE_MAYFLY;
    m_model = &models::mayfly;
    m_scale = Vec2(Fixed(0.8L), Fixed(0.8L));
    m_radius = Fixed(2.5L);
    m_scoreValue = 50;
    m_pen = Pen(128, 128, 255, 255, 1);
    m_state = STATE_INACTIVE;
}

void Mayfly::spawnTransition() {
    Entity::spawnTransition();
    m_flipTimer = int16_t(mathutils::randFromTo(0, kMayflyFlipPeriod));
    m_flipDirection = 1;
    m_target = m_pos;
}

void Mayfly::run() {
    if (getEnabled() && g_world.player1) {
        if (--m_flipTimer <= 0) {
            m_flipTimer = kMayflyFlipPeriod;
            m_flipDirection = int8_t(-m_flipDirection);

            m_target = g_world.player1->getPos();
            m_target.x += mathutils::frand() * kMayflyTargetJitter - kMayflyTargetJitter / 2;
            m_target.y += mathutils::frand() * kMayflyTargetJitter - kMayflyTargetJitter / 2;
        }

        Angle desired = (m_flipDirection > 0) ? kMayflyBank : -kMayflyBank;
        Angle diff = desired - m_angle;
        m_rotationRate += scaleAngle(diff, kMayflyBankRate);
        m_rotationRate = scaleAngle(m_rotationRate, kMayflyBankDamping);

        Angle angle = mathutils::angleTo(m_pos, m_target);
        m_speed += mathutils::fromAngle(angle, kMayflyAccel);
        m_speed = mathutils::clamp(m_speed, kMayflyMaxSpeed * m_aggression);
        m_speed *= kMayflyFriction;
    }
    Entity::run();
}


static constexpr Fixed kProtonAccel = 0.1L;
static constexpr Fixed kProtonMaxSpeed = 0.6L;

Proton::Proton() {
    m_type = TYPE_PROTON;
    m_model = &models::proton;
    m_scale = Vec2(Fixed(1, 0), Fixed(1, 0));
    m_radius = Fixed(1.2L);
    m_scoreValue = 50;
    m_edgeBounce = true;
    m_spawnTime = 0;
    m_destroyTime = 0;
    m_pen = Pen(128, 153, 255, 179, 1);
    m_state = STATE_INACTIVE;
}

void Proton::run() {
    if (getEnabled() && g_world.player1) {
        Angle angle = mathutils::angleTo(m_pos, g_world.player1->getPos());
        m_speed += mathutils::fromAngle(angle, kProtonAccel);
        m_speed = mathutils::clamp(m_speed, kProtonMaxSpeed * m_aggression);
    }
    Entity::run();
}

void Proton::spawnTransition() {
    Entity::spawnTransition();
    m_drift = Vec2(mathutils::frand() * Fixed(4, 0) - Fixed(2, 0),
                   mathutils::frand() * Fixed(4, 0) - Fixed(2, 0));
}


static constexpr Fixed kSnakeMaxSpeed = 0.6L;
static constexpr Fixed kSnakeFriction = 0.95L;

static constexpr Angle kSnakeTurnAccel = 0.00159L;
static constexpr Fixed kSnakeTurnDamping = 0.98L;
static constexpr Angle kSnakeMaxTurnRate = 0.6366L;
static constexpr Angle kSnakeAimTolerance = 0.0318L;

static constexpr Fixed kSnakeTargetDistance = 40.0L;
static constexpr Fixed kSnakeTargetReached = 10.0L;
static constexpr Fixed kSnakeTargetMargin = 15.0L;

static constexpr Fixed kSnakeTailGrowth = 0.1L;

Snake::Snake() {
    m_type = TYPE_SNAKE;
    m_model = &models::snakeHead;
    m_scale = Vec2(Fixed(1.25L), Fixed(1.25L));
    m_radius = Fixed(3, 0);
    m_scoreValue = 50;
    m_pen = Pen(128, 128, 255, 200, 1);
    m_state = STATE_INACTIVE;

    for (int i = 0; i < kSnakeSegments; i++) {
        Fixed taper = (Fixed(1, 0) - Fixed(i, 0) / kSnakeSegments) * 2;
        if (taper > Fixed(1.1L)) taper = Fixed(1.1L);
        m_segments[i].scale = Vec2(taper, Fixed(1.4L));
    }
}

void Snake::updateTarget() {
    m_target = m_pos + mathutils::fromAngle(mathutils::randomAngle(), kSnakeTargetDistance);

    const Fixed left = kSnakeTargetMargin;
    const Fixed bottom = kSnakeTargetMargin;
    const Fixed right = Fixed(kGridExtentX - 1, 0) - kSnakeTargetMargin;
    const Fixed top = Fixed(kGridExtentY - 1, 0) - kSnakeTargetMargin;

    if (m_target.x < left) m_target.x = left;
    else if (m_target.x > right) m_target.x = right;
    if (m_target.y < bottom) m_target.y = bottom;
    else if (m_target.y > top) m_target.y = top;
}

void Snake::spawnTransition() {
    Entity::spawnTransition();

    m_angle = mathutils::randomAngle();
    updateTarget();

    for (int i = 0; i < kSnakeSegments; i++) {
        Segment& seg = m_segments[i];
        Vec2 offset = mathutils::fromAngle(m_angle + Angle(0.5L), Fixed(-(i + 1), 0) * Fixed(0.2L));
        seg.pos = m_pos + offset;
        seg.angle = m_angle;
        seg.tail = Fixed();
        for (int j = 0; j < kSnakeStreamItems; j++) {
            seg.streamPos[j] = seg.pos;
            seg.streamAngle[j] = m_angle;
        }
    }
}

void Snake::run() {
    if (getEnabled()) {
        if (mathutils::distance(m_pos, m_target) < kSnakeTargetReached) updateTarget();

        Angle desired = mathutils::angleTo(m_pos, m_target) - Angle(0.5L);
        Angle diff = diffAngles(desired, m_angle);

        if (diff.value > kSnakeAimTolerance.value) {
            m_rotationRate += kSnakeTurnAccel;
        } else if (diff.value < -kSnakeAimTolerance.value) {
            m_rotationRate -= kSnakeTurnAccel;
        }

        m_rotationRate = scaleAngle(m_rotationRate, kSnakeTurnDamping);

        if (m_rotationRate.value > kSnakeMaxTurnRate.value) {
            m_rotationRate = kSnakeMaxTurnRate;
        } else if (m_rotationRate.value < -kSnakeMaxTurnRate.value) {
            m_rotationRate = -kSnakeMaxTurnRate;
        }

        m_speed += mathutils::fromAngle(m_angle + Angle(0.5L), Fixed(1, 0));
        m_speed = mathutils::clamp(m_speed, kSnakeMaxSpeed);
        m_speed *= kSnakeFriction;

        m_segments[0].streamPos[0] = m_pos;
        m_segments[0].streamAngle[0] = m_angle;

        for (int i = kSnakeSegments - 2; i >= 0; i--) {
            const Segment& from = m_segments[i];
            int tailIndex = from.tail.value >> 12;
            if (tailIndex >= kSnakeStreamItems) tailIndex = kSnakeStreamItems - 1;

            m_segments[i + 1].streamPos[0] = from.streamPos[tailIndex];
            m_segments[i + 1].streamAngle[0] = from.streamAngle[tailIndex];
        }

        for (int i = 0; i < kSnakeSegments; i++) {
            Segment& seg = m_segments[i];

            seg.pos = seg.streamPos[0];
            seg.angle = seg.streamAngle[0];

            for (int j = kSnakeStreamItems - 2; j >= 0; j--) {
                seg.streamPos[j + 1] = seg.streamPos[j];
                seg.streamAngle[j + 1] = seg.streamAngle[j];
            }

            if (seg.tail < Fixed(kSnakeStreamItems - 1, 0)) {
                seg.tail += kSnakeTailGrowth;
                if (seg.tail > Fixed(kSnakeStreamItems - 1, 0)) {
                    seg.tail = Fixed(kSnakeStreamItems - 1, 0);
                }
            }
        }
    }
    Entity::run();
}

Fixed Snake::boundingRadius() const {
    static constexpr Fixed kMaxBodyReach =
        Fixed(kSnakeSegments, 0) * kSnakeStreamItems * kSnakeMaxSpeed;
    return m_radius + kMaxBodyReach;
}

bool Snake::overlapsSegment(const Vec2& from, const Vec2& to, Fixed radius) const {
    if (Entity::overlapsSegment(from, to, radius)) return true;

    Fixed reach = m_radius + radius;
    Fixed reachSq = reach * reach;
    for (const auto& seg : m_segments) {
        if (mathutils::pointSegmentDistanceSquared(from, to, seg.pos) < reachSq) return true;
    }
    return false;
}

bool Snake::overlaps(const Vec2& pos, Fixed radius) const {
    if (Entity::overlaps(pos, radius)) return true;

    Fixed reach = m_radius + radius;
    Fixed reachSq = reach * reach;
    for (const auto& seg : m_segments) {
        if ((seg.pos - pos).lengthSquared() < reachSq) return true;
    }
    return false;
}

void Snake::draw(Renderer& r) {
    if (m_state == STATE_INACTIVE || m_state == STATE_SPAWN_TRANSITION) return;

    for (int i = kSnakeSegments - 1; i >= 0; i--) {
        const Segment& seg = m_segments[i];
        models::snakeSegment.draw(r, seg.pos, seg.angle, seg.scale, m_pen);
    }

    models::snakeHead.drawStrip(r, m_pos, m_angle, m_scale, m_pen);
}


static constexpr Fixed kHoleMaxSpeed = 0.2L;
static constexpr Fixed kHoleFriction = 0.96L;
static constexpr Fixed kHoleSeekAccel = 0.002L;
static constexpr Fixed kHoleSeekFriction = 0.98L;

static constexpr Fixed kHoleGrowthRate = 0.0003L;
static constexpr Fixed kHoleFeedGrowth = 0.06L;
static constexpr Fixed kHoleFeedBalance = 0.14L;

static constexpr Fixed kHoleWobbleDepth = 0.2L;
static constexpr Angle kHoleWobbleRate = 0.159L;

static constexpr Fixed kHolePullRange = 30.0L;
static constexpr Fixed kHolePullStrength = 0.06L;

static constexpr int kHoleRingSegments = 16;

BlackHole::BlackHole() {
    m_type = TYPE_BLACKHOLE;
    m_model = nullptr;
    m_scale = Vec2(Fixed(1.5L), Fixed(1.5L));
    m_radius = Fixed(2.5L);
    m_scoreValue = 50;
    m_edgeBounce = true;
    m_destroyTime = 0;
    m_pen = Pen(255, 128, 128, 179, 1);
    m_state = STATE_INACTIVE;
}

void BlackHole::spawnTransition() {
    Entity::spawnTransition();
    m_activated = false;
    m_strength = Fixed(1, 0);
    m_balance = Fixed();
    m_balanceRate = Fixed();
    m_points = 0;
    m_animationIndex = Angle();
    m_gridPullIndex = mathutils::randomAngle();
}

void BlackHole::activate() {
    m_activated = true;
    m_balance = Fixed(1.6L);
    m_balanceRate = Fixed();
}

void BlackHole::feed(int points) {
    m_points += points * 2;
    m_strength += kHoleFeedGrowth;
    m_balanceRate += kHoleFeedBalance;
}

bool BlackHole::weaken() {
    m_strength = m_strength * Fixed(0.98L);
    return m_strength < kCollapseStrength;
}

Fixed BlackHole::ringRadius() const {
    if (!m_activated) return m_radius;
    Fixed r = m_radius + gw::sin(m_animationIndex) * kHoleWobbleDepth;
    return r * (m_strength + m_balance * Fixed(0.1L));
}

Fixed BlackHole::absorbRadius() const {
    if (!m_activated) return m_radius;
    return ringRadius() * 2;
}

void BlackHole::run() {
    m_speed = mathutils::clamp(m_speed, kHoleMaxSpeed);
    m_speed *= kHoleFriction;

    if (m_activated) {
        m_strength += kHoleGrowthRate;

        if (m_balance.value > 0) {
            m_balanceRate -= Fixed(0.01L);
        } else if (m_balance.value < 0) {
            m_balanceRate += Fixed(0.01L);
        }
        m_balanceRate *= Fixed(0.95L);
        m_balance += m_balanceRate;
        m_balance *= Fixed(0.95L);

        if (Attractor* att = g_world.grid.getAttractor()) {
            m_gridPullIndex = wrapAngle(m_gridPullIndex + Angle(0.0127L));
            Fixed s = gw::sin(m_gridPullIndex);
            if (s.value > 0) s = s * Fixed(0.5L);
            s = (s + Fixed(1, 0)) / 2 + Fixed(0.5L);

            att->strength = s * Fixed(-30, 0);
            att->radius = Fixed(6, 0);
            att->pos = m_pos;
            att->enabled = true;
            att->attractsParticles = true;
        }

        m_animationIndex = wrapAngle(m_animationIndex + kHoleWobbleRate);
    }

    if (g_world.player1) {
        Angle angle = mathutils::angleTo(m_pos, g_world.player1->getPos());
        m_speed += mathutils::fromAngle(angle, kHoleSeekAccel);
    }
    m_speed *= kHoleSeekFriction;

    Entity::run();
}

void BlackHole::draw(Renderer& r) {
    if (m_state == STATE_INACTIVE || m_state == STATE_SPAWN_TRANSITION) return;

    if (m_state == STATE_INDICATING && !((m_stateTimer / 5) & 1)) return;

    Fixed radius = ringRadius();

    Pen pen = m_pen;
    if (m_activated) {
        Fixed danger = m_strength / kBurstStrength;
        if (danger > Fixed(1, 0)) danger = Fixed(1, 0);
        pen.g = uint8_t(128 - int((Fixed(128, 0) * danger).value >> 12));
        pen.b = pen.g;
    }

    Vec2 prev = m_pos + Vec2(radius, Fixed());
    for (int i = 1; i <= kHoleRingSegments; i++) {
        Angle a(int32_t((kTurnRaw * i) / kHoleRingSegments), Angle::RAW);
        Vec2 next = m_pos + mathutils::fromAngle(a, radius);
        r.line(prev, next, pen);
        prev = next;
    }
}


static constexpr Fixed kRepulsorChargeAccel = 0.07L;
static constexpr Fixed kRepulsorShieldRange = 40.0L;
static constexpr Fixed kRepulsorRepelRange = 25.0L;
static constexpr Fixed kRepulsorRepelForce = 0.06L;
static constexpr Angle kRepulsorArc = 0.2387L;

Repulsor::Repulsor() {
    m_type = TYPE_REPULSOR;
    m_model = &models::repulsor;
    m_scale = Vec2(Fixed(0.16L), Fixed(0.16L));
    m_radius = Fixed(2.6L);
    m_scoreValue = 200;
    m_pen = Pen(245, 130, 89, 179, 1);
    m_state = STATE_INACTIVE;
}

void Repulsor::spawnTransition() {
    Entity::spawnTransition();
    m_aiState = State_Thinking;
    m_timer = 30;
    m_shieldsEnabled = false;
}

void Repulsor::run() {
    Player* player = g_world.player1;

    if (getEnabled() && player) {
        m_shieldsEnabled = false;
        PlayerMissile* const* live = player->liveMissiles();
        const int liveCount = player->liveMissileCount();
        const Fixed rangeSq = kRepulsorShieldRange * kRepulsorShieldRange;

        for (int i = 0; i < liveCount; i++) {
            if ((live[i]->getPos() - m_pos).lengthSquared() < rangeSq) {
                m_shieldsEnabled = true;
                break;
            }
        }

        if (m_shieldsEnabled) m_shieldPhase = wrapAngle(m_shieldPhase + Angle(0.02L));

        Angle desired = mathutils::angleTo(m_pos, player->getPos()) - Angle(0.5L);
        Angle angleDiff = diffAngles(m_angle, desired);

        switch (m_aiState) {
        case State_Charging:
            m_speed += mathutils::fromAngle(m_angle + Angle(0.5L), kRepulsorChargeAccel);

            if (angleDiff.value > Angle(0.333L).value || angleDiff.value < -Angle(0.333L).value) {
                if (--m_timer <= 0) {
                    m_aiState = State_Thinking;
                    m_timer = 10;
                }
            }
            m_angle = wrapAngle(m_angle - scaleAngle(angleDiff, Fixed(0.035L)));
            break;

        case State_Aiming:
            if (angleDiff.value < Angle(0.0637L).value && angleDiff.value > -Angle(0.0637L).value) {
                m_aiState = State_Charging;
                m_timer = 20;
            } else {
                m_angle = wrapAngle(m_angle - scaleAngle(angleDiff, Fixed(0.05L)));
            }
            break;

        case State_Thinking:
        default:
            if (--m_timer <= 0) m_aiState = State_Aiming;
            break;
        }

        m_speed = mathutils::clamp(m_speed, Fixed(0.5L));
        m_speed *= Fixed(0.97L);
    }

    Entity::run();
}

void Repulsor::repelEntity(Entity& e) const {
    if (m_state != STATE_RUNNING || !m_shieldsEnabled) return;

    Vec2 delta = e.getPos() - m_pos;
    if (delta.lengthSquared() >= kRepulsorRepelRange * kRepulsorRepelRange) return;

    Angle toTarget = gw::atan2(delta.y, delta.x);
    Angle diff = diffAngles(m_angle + Angle(0.5L), toTarget);
    if (diff.value > kRepulsorArc.value || diff.value < -kRepulsorArc.value) return;

    Vec2 push(Fixed(), diff.value > 0 ? Fixed(200, 0) : Fixed(-200, 0));
    push = mathutils::rotate(push, toTarget + Angle(1.0L));
    e.setDrift(e.getDrift() + mathutils::clamp(push, kRepulsorRepelForce));
}

void Repulsor::draw(Renderer& r) {
    Entity::draw(r);

    if (!m_shieldsEnabled || m_state != STATE_RUNNING) return;

    Pen shieldPen(245, 130, 89, 179, 1);
    Vec2 shieldScale(Fixed(0.12L), Fixed(0.12L));

    for (int i = 0; i < kRepulsorShields; i++) {
        Angle spread(int32_t((kTurnRaw / 8) * (i - 1) / 2), Angle::RAW);
        Angle a = m_angle + spread + scaleAngle(gw::sin(m_shieldPhase) < Fixed() ? -Angle(0.05L) : Angle(0.05L), Fixed(1, 0));
        models::repulsorShield.draw(r, m_pos, a, shieldScale, shieldPen);
    }
}


LineDebris::LineDebris() {
    m_type = TYPE_LINE;
    m_gridBound = false;
    m_state = STATE_INACTIVE;
}

void LineDebris::launch(const Vec2& from, const Vec2& to, const Vec2& drift, const Pen& pen) {
    m_pos = from;
    m_to = to;
    m_speed = drift;
    m_pen = pen;
    m_life = 40;
    m_rotationRate = Angle(int32_t(mathutils::random32() % 64) - 32, Angle::RAW);
    m_angle = Angle();
    m_state = STATE_RUNNING;
}

void LineDebris::run() {
    if (--m_life <= 0) {
        m_state = STATE_INACTIVE;
        return;
    }

    Vec2 delta = m_speed;
    m_pos += delta;
    m_to += delta;
    m_speed *= Fixed(0.96L);

    m_pen.a = uint8_t((m_life * 255) / 40);
}

void LineDebris::draw(Renderer& r) {
    if (m_state != STATE_RUNNING) return;
    r.line(m_pos, m_to, m_pen);
}


template <typename Fn>
void Enemies::forEachPool(Fn&& fn) {
    for (auto& e : m_grunts) fn(e);
    for (auto& e : m_wanderers) fn(e);
    for (auto& e : m_weavers) fn(e);
    for (auto& e : m_spinners) fn(e);
    for (auto& e : m_tinySpinners) fn(e);
    for (auto& e : m_mayflies) fn(e);
    for (auto& e : m_protons) fn(e);
    for (auto& e : m_snakes) fn(e);
    for (auto& e : m_blackHoles) fn(e);
    for (auto& e : m_repulsors) fn(e);
}

void Enemies::splitSpinner(Spinner& spinner, const Vec2& missileSpeed) {
    Vec2 origin = spinner.getPos();
    Angle toPlayer = g_world.player1 ? mathutils::angleTo(origin, g_world.player1->getPos()) : Angle();

    static constexpr Angle kQuarter = 0.5L;
    const Angle offsets[2] = {toPlayer + kQuarter, toPlayer - kQuarter};

    for (int i = 0; i < 2; i++) {
        Entity* slot = getUnusedEnemyOfType(Entity::TYPE_TINYSPINNER);
        if (!slot) return;

        auto* tiny = static_cast<TinySpinner*>(slot);
        tiny->setPos(origin + mathutils::fromAngle(offsets[i], kFragmentOffset));
        tiny->setInitialSpeed(missileSpeed * kFragmentSpeedScale);
        tiny->setWaveId(Entity::kNoWave);
        tiny->setState(Entity::STATE_SPAWN_TRANSITION);
    }
}

void Enemies::init() { disableAll(); }

void Enemies::disableAll() {
    forEachPool([](Entity& e) { e.setState(Entity::STATE_INACTIVE); });
    for (auto& d : m_debris) d.setState(Entity::STATE_INACTIVE);
    m_activeCount = 0;
}

void Enemies::run() {
    forEachPool([](Entity& e) {
        if (e.getEnabled()) e.tick();
    });
    for (auto& d : m_debris) {
        if (d.getEnabled()) d.run();
    }

    rebuildActiveList();
}

void Enemies::draw(Renderer& r) {
    forEachPool([&r](Entity& e) {
        if (e.getEnabled()) e.draw(r);
    });
    for (auto& d : m_debris) {
        if (d.getEnabled()) d.draw(r);
    }
}

Entity* Enemies::getUnusedEnemyOfType(Entity::Type type) {
    switch (type) {
    case Entity::TYPE_GRUNT:
        for (auto& e : m_grunts)
            if (!e.getEnabled()) return &e;
        break;
    case Entity::TYPE_WANDERER:
        for (auto& e : m_wanderers)
            if (!e.getEnabled()) return &e;
        break;
    case Entity::TYPE_WEAVER:
        for (auto& e : m_weavers)
            if (!e.getEnabled()) return &e;
        break;
    case Entity::TYPE_SPINNER:
        for (auto& e : m_spinners)
            if (!e.getEnabled()) return &e;
        break;
    case Entity::TYPE_TINYSPINNER:
        for (auto& e : m_tinySpinners)
            if (!e.getEnabled()) return &e;
        break;
    case Entity::TYPE_MAYFLY:
        for (auto& e : m_mayflies)
            if (!e.getEnabled()) return &e;
        break;
    case Entity::TYPE_PROTON:
        for (auto& e : m_protons)
            if (!e.getEnabled()) return &e;
        break;
    case Entity::TYPE_SNAKE:
        for (auto& e : m_snakes)
            if (!e.getEnabled()) return &e;
        break;
    case Entity::TYPE_BLACKHOLE:
        for (auto& e : m_blackHoles)
            if (!e.getEnabled()) return &e;
        break;
    case Entity::TYPE_REPULSOR:
        for (auto& e : m_repulsors)
            if (!e.getEnabled()) return &e;
        break;
    default:
        break;
    }
    return nullptr;
}

void Enemies::rebuildActiveList() {
    m_activeCount = 0;
    forEachPool([&](Entity& e) {
        if (e.getState() != Entity::STATE_RUNNING) return;

        if (m_activeCount >= kTotalPoolSlots) return;

        ActiveEnemy& a = m_active[m_activeCount++];
        a.entity = &e;
        a.hitPos = e.getHitPos();
        a.radius = e.boundingRadius();
    });
}

Entity* Enemies::hitTestSegment(const Vec2& from, const Vec2& to, Fixed radius) {
    Vec2 travel = to - from;
    Fixed travelLen = gw::approxLength(travel.x, travel.y);

    for (int i = 0; i < m_activeCount; i++) {
        const ActiveEnemy& a = m_active[i];

        Fixed reach = a.radius + radius + travelLen;
        if ((a.hitPos - to).lengthSquared() >= reach * reach) continue;

        if (a.entity->getState() != Entity::STATE_RUNNING) continue;

        if (a.entity->overlapsSegment(from, to, radius)) return a.entity;
    }
    return nullptr;
}

Entity* Enemies::hitTestAtPosition(const Vec2& pos, Fixed radius) {
    for (int i = 0; i < m_activeCount; i++) {
        const ActiveEnemy& a = m_active[i];

        Fixed reach = a.radius + radius;
        if ((a.hitPos - pos).lengthSquared() >= reach * reach) continue;

        if (a.entity->getState() != Entity::STATE_RUNNING) continue;

        if (a.entity->overlaps(pos, radius)) return a.entity;
    }
    return nullptr;
}

void Enemies::runBlackHoles() {
    for (auto& hole : m_blackHoles) {
        if (hole.getState() != Entity::STATE_RUNNING || !hole.activated()) continue;

        if (hole.shouldExplode()) {
            explodeEntity(hole);
            hole.setState(Entity::STATE_DESTROY_TRANSITION);

            for (int i = 0; i < 12; i++) {
                Entity* slot = getUnusedEnemyOfType(Entity::TYPE_PROTON);
                if (!slot) break;

                Vec2 offset = mathutils::fromAngle(mathutils::randomAngle(),
                                                   mathutils::frand() * Fixed(10, 0));
                slot->setPos(hole.getPos() + offset);
                slot->setWaveId(Entity::kNoWave);
                slot->setState(Entity::STATE_SPAWN_TRANSITION);
                slot->setDrift(offset);
            }

            Pen burst = hole.getPen();
            burst.a = 220;
            g_world.particles.emit(hole.getPos(), Angle(), Fixed(2.5L), Angle(2.0L), 20, burst, 220);
            continue;
        }

        Vec2 center = hole.getPos();
        Fixed rangeSq = kHolePullRange * kHolePullRange;
        const Fixed absorb = hole.absorbRadius();
        const Fixed absorbSq = absorb * absorb;

        for (int i = 0; i < m_activeCount; i++) {
            Entity* e = m_active[i].entity;
            if (e == &hole) continue;

            Vec2 delta = center - m_active[i].hitPos;
            Fixed distSq = delta.lengthSquared();
            if (distSq.value == 0 || distSq >= rangeSq) continue;

            if (e->getState() != Entity::STATE_RUNNING) continue;

            if (distSq < absorbSq && e->getType() != Entity::TYPE_BLACKHOLE) {
                hole.setDrift(hole.getDrift() + e->getDrift() * Fixed(0.25L));

                explodeEntity(*e);
                e->setState(Entity::STATE_DESTROY_TRANSITION);
                hole.feed(e->getScoreValue());

                if (Attractor* att = g_world.grid.getAttractor()) {
                    att->strength = Fixed(1.5L);
                    att->radius = Fixed(30, 0);
                    att->pos = center;
                    att->enabled = true;
                    att->attractsParticles = false;
                }
                continue;
            }

            Fixed dist = gw::sqrt(distSq);
            Fixed falloff = Fixed(1, 0) - dist / kHolePullRange;
            e->setDrift(e->getDrift() + delta * (kHolePullStrength * falloff / dist));
        }
    }
}

void Enemies::repelMissiles(PlayerMissile* const* missiles, int count) const {
    for (const auto& rep : m_repulsors) {
        if (rep.getState() != Entity::STATE_RUNNING) continue;
        for (int i = 0; i < count; i++) rep.repelEntity(*missiles[i]);
    }
}

void Enemies::destroyAllWithExplosion(Entity* spare) {
    forEachPool([&](Entity& e) {
        switch (e.getState()) {
        case Entity::STATE_INACTIVE:
        case Entity::STATE_INDICATING:
        case Entity::STATE_INDICATE_TRANSITION:
        case Entity::STATE_DESTROY_TRANSITION:
        case Entity::STATE_DESTROYED:
            return;
        default:
            break;
        }

        if (&e == spare) {
            e.setState(Entity::STATE_INDICATE_TRANSITION);
            return;
        }

        explodeEntity(e);
        e.setState(Entity::STATE_DESTROY_TRANSITION);
    });
}

void Enemies::destroyInShell(const Vec2& center, Fixed innerRadius, Fixed outerRadius) {
    Fixed innerSq = innerRadius * innerRadius;
    Fixed outerSq = outerRadius * outerRadius;

    forEachPool([&](Entity& e) {
        switch (e.getState()) {
        case Entity::STATE_INACTIVE:
        case Entity::STATE_INDICATING:
        case Entity::STATE_INDICATE_TRANSITION:
        case Entity::STATE_DESTROY_TRANSITION:
        case Entity::STATE_DESTROYED:
            return;
        default:
            break;
        }

        Fixed distSq = (e.getHitPos() - center).lengthSquared();
        if (distSq <= innerSq || distSq >= outerSq) return;

        explodeEntity(e);
        e.setState(Entity::STATE_DESTROY_TRANSITION);
    });
}


int Enemies::countActiveOfType(Entity::Type type) const {
    int count = 0;
    auto tally = [&](const auto& pool) {
        for (const auto& e : pool) {
            if (e.getEnabled() && e.getType() == type) count++;
        }
    };
    tally(m_grunts);
    tally(m_wanderers);
    tally(m_weavers);
    tally(m_spinners);
    tally(m_tinySpinners);
    tally(m_mayflies);
    tally(m_protons);
    tally(m_snakes);
    tally(m_blackHoles);
    tally(m_repulsors);
    return count;
}

int Enemies::countActive() const {
    int count = 0;
    auto tally = [&](const auto& pool) {
        for (const auto& e : pool) {
            if (e.getEnabled()) count++;
        }
    };
    tally(m_grunts);
    tally(m_wanderers);
    tally(m_weavers);
    tally(m_spinners);
    tally(m_tinySpinners);
    tally(m_mayflies);
    tally(m_protons);
    tally(m_snakes);
    tally(m_blackHoles);
    tally(m_repulsors);
    return count;
}

int Enemies::poolSize(Entity::Type type) const {
    switch (type) {
    case Entity::TYPE_GRUNT: return kMaxGrunts;
    case Entity::TYPE_WANDERER: return kMaxWanderers;
    case Entity::TYPE_WEAVER: return kMaxWeavers;
    case Entity::TYPE_SPINNER: return kMaxSpinners;
    case Entity::TYPE_TINYSPINNER: return kMaxTinySpinners;
    case Entity::TYPE_MAYFLY: return kMaxMayflies;
    case Entity::TYPE_PROTON: return kMaxProtons;
    case Entity::TYPE_SNAKE: return kMaxSnakes;
    case Entity::TYPE_BLACKHOLE: return kMaxBlackHoles;
    case Entity::TYPE_REPULSOR: return kMaxRepulsors;
    default: return 0;
    }
}

bool Enemies::anyAliveFromWave(uint8_t waveId) const {
    auto search = [&](const auto& pool) {
        for (const auto& e : pool) {
            if (e.getEnabled() && e.getWaveId() == waveId) return true;
        }
        return false;
    };

    return search(m_grunts) || search(m_wanderers) || search(m_weavers) || search(m_spinners) ||
           search(m_tinySpinners) || search(m_mayflies) || search(m_protons) || search(m_snakes) ||
           search(m_blackHoles) || search(m_repulsors);
}

void Enemies::explodeEntity(Entity& e) {
    const Model* model = e.getModel();
    if (!model) return;

    Fixed sinA = gw::sin(e.getAngle());
    Fixed cosA = gw::cos(e.getAngle());
    Vec2 scale = e.getScale();
    Vec2 origin = e.getHitPos();

    int emitted = 0;
    for (uint8_t i = 0; i < model->edgeCount && emitted < 12; i++) {
        LineDebris* slot = nullptr;
        for (auto& d : m_debris) {
            if (!d.getEnabled()) {
                slot = &d;
                break;
            }
        }
        if (!slot) return;

        Vec2 from = model->transform(model->vertices[model->edges[i].from], origin, sinA, cosA, scale);
        Vec2 to = model->transform(model->vertices[model->edges[i].to], origin, sinA, cosA, scale);

        Vec2 mid((from.x + to.x) / 2, (from.y + to.y) / 2);
        Angle outward = mathutils::angleTo(origin, mid);
        Fixed speed = Fixed(0.2L) + mathutils::frand() * Fixed(0.4L);

        slot->launch(from, to, mathutils::fromAngle(outward, speed), e.getPen());
        emitted++;
    }
}

}
