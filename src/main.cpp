#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <cocos2d.h>

using namespace geode::prelude;

// state tracking
static int frame = 0;
static bool lastIsDead = false;
static int attempt = 0;
static float bestPercent = 0.0f;

static std::vector<bool> clicks;
static bool clicking = false;
static bool solverActive = true;

// visual text display
static cocos2d::CCLabelBMFont* hudLabel = nullptr;

void updateHUD() {
    if (!hudLabel) return;

    hudLabel->setString(
        fmt::format(
            "Attempt: {}\nBest: {:.2f}%",
            attempt,
            bestPercent
        ).c_str()
    );
}

// backtracking logic
void backtrack() {
    for (int i = frame - 1; i >= 0; --i) {
        if (!clicks[i]) {
            clicks[i] = true;
            clicks.resize(i + 1);
            log::info("Backtrack → click at frame {}", i);
            return;
        }
    }

    log::error("Search exhausted — stopping solver");
    solverActive = false;
    updateHUD();
}

// playlayer controller
class $modify(PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects))
            return false;

        frame = 0;
        clicking = false;
        lastIsDead = false;

        if (attempt == 0) {
            // warm-up attempt to guarantee consistent frame rate
            clicks.clear();
            bestPercent = 0.0f;
            log::info("Warm-up attempt");
        } else {
            log::info("Replay attempt {}", attempt);
        }

        // one-time HUD setup
        if (!hudLabel) {
            hudLabel = cocos2d::CCLabelBMFont::create("", "bigFont.fnt");
            hudLabel->setAnchorPoint({0.f, 1.f});
            hudLabel->setScale(0.5f);

            auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
            hudLabel->setPosition({10.f, winSize.height - 10.f});

            this->addChild(hudLabel, 1000);
        }

        updateHUD();
        return true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (!solverActive)
            return;
        elif (dt <= 0.0f)
            return;

        auto player = m_player1;
        if (!player)
            return;

        // progress tracking
        float percent =
            (player->getPositionX() / m_levelLength) * 100.f;

        if (percent > bestPercent) {
            bestPercent = percent;
            updateHUD();
        }

				// input replay
        if (attempt > 0) {
            if (frame >= static_cast<int>(clicks.size()))
                clicks.push_back(false);

            bool wantClick = clicks[frame];

            if (wantClick && !clicking) {
                player->pushButton(PlayerButton::Jump);
                clicking = true;
            }
            else if (!wantClick && clicking) {
                player->releaseButton(PlayerButton::Jump);
                clicking = false;
            }
        }

        // death detection
        if (player->m_isDead && !lastIsDead) {
            if (attempt > 0) {
                log::info(
                    "Died at frame {} ({}%)",
                    frame - 1,
                    percent
                );
                backtrack();
            } else {
                log::info("Warm-up death ignored");
            }
        }

        // detect level restart
        if (!player->m_isDead && lastIsDead) {
            attempt++;
            frame = 0;
            clicking = false;
            updateHUD();
            log::info("Restart → attempt {}", attempt);
        }

        lastIsDead = player->m_isDead;
        frame++;
    }

    // detection completion
    void levelComplete() {
        solverActive = false;
        updateHUD();

        log::info(
            "Level completed — best {:.2f}%",
            bestPercent
        );

        PlayLayer::levelComplete();
    }
};
