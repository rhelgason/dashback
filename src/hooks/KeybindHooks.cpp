#include <Geode/Geode.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

#include "../core/SolverController.hpp"

using namespace geode::prelude;

// Live speed control while solving/replaying:
//   ]  -> faster        [  -> slower
// Adjusts the game's time scale on the fly (see SolverController::adjustSpeed).
class $modify(DBKeyboard, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double delta) {
        auto& solver = dashback::SolverController::get();
        if (down && !repeat && solver.inLevel()) {
            if (key == KEY_RightBracket) { solver.adjustSpeed(+0.5f); return true; }
            if (key == KEY_LeftBracket) { solver.adjustSpeed(-0.5f); return true; }
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, delta);
    }
};
