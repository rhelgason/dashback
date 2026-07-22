#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "../daily/DailyRunner.hpp"

using namespace geode::prelude;

// Entry point for the long-term daily-auto-solve goal. Gated behind a setting;
// does nothing unless "auto-solve-daily" is enabled.
class $modify(DBMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        dashback::DailyRunner::get().onMenuLayer(this);
        return true;
    }
};
