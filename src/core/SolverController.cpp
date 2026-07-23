#include "SolverController.hpp"

#include "AlgorithmRegistry.hpp"
#include "../algorithms/Builtins.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/GJBaseGameLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/binding/GJGameLevel.hpp>

#include <algorithm>
#include <chrono>

using namespace geode::prelude;

namespace dashback {

SolverController& SolverController::get() {
    static SolverController instance;
    return instance;
}

static double elapsedMs(std::chrono::steady_clock::time_point start) {
    return std::chrono::duration<double, std::milli>(
               std::chrono::steady_clock::now() - start)
        .count();
}

void SolverController::onLevelStart(PlayLayer* pl) {
    m_playLayer = pl;
    m_active = false;
    m_solved = false;
    m_algo.reset();

    if (!pl || !pl->m_level) return;

    if (!Mod::get()->getSettingValue<bool>("enabled")) {
        log::info("dashback: solver disabled via settings");
        return;
    }

    // Idempotent; makes sure the built-in algorithms are linked and registered
    // regardless of static-init ordering.
    registerBuiltinAlgorithms();

    std::string algoId = Mod::get()->getSettingValue<std::string>("algorithm");
    auto& registry = AlgorithmRegistry::get();
    if (!registry.has(algoId)) {
        log::warn("dashback: unknown algorithm '{}', falling back to 'backtracking'", algoId);
        algoId = "backtracking";
    }
    m_algo = registry.create(algoId);
    if (!m_algo) {
        log::error("dashback: failed to create algorithm '{}'", algoId);
        return;
    }

    m_maxAttempts = static_cast<int>(Mod::get()->getSettingValue<int64_t>("max-attempts"));

    m_level.levelID = pl->m_level->m_levelID;
    m_level.name = std::string(pl->m_level->m_levelName);
    m_level.length = pl->m_levelLength;

    m_sessionId = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();
    m_metrics.begin(m_level, m_algo->name(), m_sessionId);

    m_active = true;
    m_attempt = 0;
    m_bestEver = 0.f;
    m_algo->onLevelStart(m_level);

    // Speed the game up for faster iteration. Physics is fixed-step, so this
    // changes wall-clock only, not the deterministic step sequence.
    float speed = static_cast<float>(Mod::get()->getSettingValue<double>("solve-speed"));
    if (auto* sched = CCDirector::sharedDirector()->getScheduler()) {
        sched->setTimeScale(speed);
    }

    log::info("dashback: solving '{}' (id {}) with '{}' at {}x — metrics -> {}",
        m_level.name, m_level.levelID, m_algo->name(), speed, m_metrics.filePath());

    beginAttempt();
}

void SolverController::beginAttempt() {
    m_frame = 0;
    m_bestProgress = 0.f;
    m_lastHold = false;
    m_deadThisAttempt = false;
    m_awaitingFirstStep = true;
    ++m_attempt;
    m_attemptStart = std::chrono::steady_clock::now();
    m_algo->onAttemptStart(m_attempt);
}

void SolverController::onAttemptStart() {
    if (!m_active || !m_algo) return;
    // The game can call resetLevel() more than once before the next physics step
    // (our scheduled reset + the built-in auto-retry). Debounce so we don't burn
    // an attempt number on a reset that never actually ran.
    if (m_awaitingFirstStep) return;
    beginAttempt();
}

void SolverController::onStep(GJBaseGameLayer* gl) {
    if (!m_active || !m_algo || !m_playLayer) return;
    if (gl != static_cast<GJBaseGameLayer*>(m_playLayer)) return; // only the level we're solving

    auto player = gl->m_player1;
    if (!player || player->m_isDead) return;

    m_awaitingFirstStep = false;

    StepContext ctx;
    ctx.frame = m_frame;
    ctx.player = player;
    ctx.gameLayer = gl;
    ctx.playerX = player->getPositionX();
    ctx.playerY = player->getPositionY();
    ctx.playerYVelocity = player->m_yVelocity;
    ctx.onGround = player->m_isOnGround;
    ctx.upsideDown = player->m_isUpsideDown;
    ctx.isShip = player->m_isShip;
    ctx.progress = (gl->m_levelLength > 0.f)
        ? std::clamp(ctx.playerX / gl->m_levelLength, 0.f, 1.f)
        : 0.f;

    if (ctx.progress > m_bestProgress) m_bestProgress = ctx.progress;
    if (ctx.progress > m_bestEver) m_bestEver = ctx.progress;

    // Apply input through the game's own handler so it enters exactly as a real
    // press would. Only on transitions, mirroring real hold/release input.
    InputState in = m_algo->decide(ctx);
    if (in.hold != m_lastHold) {
        gl->handleButton(in.hold, static_cast<int>(PlayerButton::Jump), true);
        m_lastHold = in.hold;
    }

    ++m_frame;
}

void SolverController::onDeath(PlayLayer* pl) {
    if (!m_active || !m_algo) return;
    if (pl != m_playLayer) return;
    if (m_deadThisAttempt) return; // destroyPlayer fires for p1 and p2; count once
    m_deadThisAttempt = true;

    AttemptResult r;
    r.attempt = m_attempt;
    r.success = false;
    r.bestProgress = m_bestProgress;
    r.deathFrame = m_frame;
    r.frames = m_frame;
    r.wallMs = elapsedMs(m_attemptStart);
    m_metrics.record(r);

    DeathInfo info;
    info.frame = m_frame;
    info.progress = m_bestProgress;
    m_algo->onDeath(info);

    log::info("dashback: attempt {} died at frame {} ({:.1f}%, best ever {:.1f}%) [{:.0f}ms]",
        m_attempt, m_frame, m_bestProgress * 100.f, m_bestEver * 100.f, r.wallMs);

    if (reachedAttemptLimit() || !m_algo->wantsAnotherAttempt()) {
        log::info("dashback: stopping after {} attempts", m_attempt);
        m_active = false;
        return;
    }

    // Reset next frame: avoids re-entrancy inside destroyPlayer and skips the
    // slow death animation for faster iteration.
    queueInMainThread([this] {
        if (m_active && m_playLayer) m_playLayer->resetLevel();
    });
}

void SolverController::onComplete(PlayLayer* pl) {
    if (!m_active || !m_algo) return;
    if (pl != m_playLayer) return;

    AttemptResult r;
    r.attempt = m_attempt;
    r.success = true;
    r.bestProgress = 1.0f;
    r.deathFrame = m_frame;
    r.frames = m_frame;
    r.wallMs = elapsedMs(m_attemptStart);
    m_metrics.record(r);
    m_algo->onComplete(r);

    m_solved = true;
    m_active = false;
    m_bestProgress = 1.0f;
    m_bestEver = 1.0f;

    log::info("dashback: SOLVED '{}' on attempt {} with '{}' [{:.0f}ms]",
        m_level.name, m_attempt, m_algo->name(), r.wallMs);
}

void SolverController::onLevelEnd() {
    if (m_active) {
        log::info("dashback: level closed after {} attempts", m_attempt);
    }
    m_active = false;
    m_playLayer = nullptr;
    m_algo.reset();

    // Restore normal game speed when leaving the level.
    if (auto* sched = CCDirector::sharedDirector()->getScheduler()) {
        sched->setTimeScale(1.0f);
    }
}

bool SolverController::reachedAttemptLimit() const {
    return m_maxAttempts > 0 && m_attempt >= m_maxAttempts;
}

std::string SolverController::hudText() const {
    if (!m_active && !m_solved) return "";
    std::string status = m_solved ? "\nSOLVED" : (m_active ? "" : "\nSTOPPED");
    return fmt::format("dashback [{}]\nAttempt: {}\nNow: {:.1f}%  Best: {:.1f}%{}",
        m_algo ? m_algo->name() : "-", m_attempt,
        m_bestProgress * 100.f, m_bestEver * 100.f, status);
}

} // namespace dashback
