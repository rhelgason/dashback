#pragma once

class MenuLayer;

namespace dashback {

// Long-term goal: a process that continuously solves the Daily level with the
// best-performing algorithm. This is the entry point for that automation.
//
// STATUS: scaffold. The auto-navigation into the Daily level requires driving
// networked game UI (fetching the daily, opening it, pressing play) that can't
// be meaningfully validated without a running client, so the navigation itself
// is left as clearly-marked TODOs. The hook point, the setting gate, and the
// wiring into MenuLayer are done, so finishing this is a contained task rather
// than an architectural one.
class DailyRunner {
public:
    static DailyRunner& get();

    // Called from MenuLayer::init. If the "auto-solve-daily" setting is on,
    // kicks off the daily-solving flow.
    void onMenuLayer(MenuLayer* menuLayer);

    bool enabled() const;

private:
    DailyRunner() = default;

    bool m_launched = false; // avoid re-triggering every time the menu rebuilds
};

} // namespace dashback
