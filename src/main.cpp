#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <imgui-cocos.hpp>
#include <deque>

using namespace geode::prelude;

// ---------------------------------------------------------------- settings
struct Config {
    bool enabled = false;
    bool practiceOnly = true;   // safe default: never touch normal runs
    int  delayTicks = 4;        // 1 tick = 1/240 s, 4 ticks = 1 frame @ 60fps
} g_cfg;

static void loadConfig() {
    auto m = Mod::get();
    g_cfg.enabled      = m->getSavedValue<bool>("enabled", false);
    g_cfg.practiceOnly = m->getSavedValue<bool>("practice-only", true);
    g_cfg.delayTicks   = m->getSavedValue<int>("delay-ticks", 4);
}
static void saveConfig() {
    auto m = Mod::get();
    m->setSavedValue("enabled", g_cfg.enabled);
    m->setSavedValue("practice-only", g_cfg.practiceOnly);
    m->setSavedValue("delay-ticks", g_cfg.delayTicks);
}

// ------------------------------------------------------------- input buffer
struct Pending { bool down; int button; bool p1; int ticksLeft; };

class $modify(BufferLayer, GJBaseGameLayer) {
    struct Fields {
        std::deque<Pending> queue;
        bool replaying = false;
    };

    bool bufferActive() {
        if (!g_cfg.enabled || g_cfg.delayTicks <= 0) return false;
        auto pl = PlayLayer::get();
        if (!pl || static_cast<GJBaseGameLayer*>(pl) != this) return false;
        if (m_level && m_level->isPlatformer()) return false;
        if (g_cfg.practiceOnly && !pl->m_isPracticeMode) return false;
        return true;
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        auto f = m_fields.self();
        if (f->replaying || !bufferActive()) {
            return GJBaseGameLayer::handleButton(down, button, isPlayer1);
        }
        // presses AND releases get the same delay, so order is preserved
        f->queue.push_back({down, button, isPlayer1, g_cfg.delayTicks});
    }

    void processCommands(float dt) {
        auto f = m_fields.self();
        if (m_player1 && m_player1->m_isDead) f->queue.clear();

        if (!f->queue.empty()) {
            f->replaying = true;
            for (auto it = f->queue.begin(); it != f->queue.end();) {
                if (--it->ticksLeft <= 0) {
                    this->handleButton(it->down, it->button, it->p1);
                    it = f->queue.erase(it);
                } else ++it;
            }
            f->replaying = false;
        }
        GJBaseGameLayer::processCommands(dt);
    }
};

// ------------------------------------------------------------------ F7 key
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool repeat, double timestamp) {
        if (key == KEY_F7 && down && !repeat) {
            ImGuiCocos::get().toggle();
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat, timestamp);
    }
};

// --------------------------------------------------------------------- GUI
$on_mod(Loaded) {
    loadConfig();

    ImGuiCocos::get().setup([] {
        ImGui::GetStyle().WindowRounding = 6.f;
    }).draw([] {
        bool open = true;
        ImGui::SetNextWindowSize({320, 0}, ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Input Buffer", &open)) {
            bool changed = false;
            changed |= ImGui::Checkbox("Enabled", &g_cfg.enabled);
            changed |= ImGui::Checkbox("Practice mode only", &g_cfg.practiceOnly);
            changed |= ImGui::SliderInt("Delay (ticks)", &g_cfg.delayTicks, 0, 64);
            ImGui::TextDisabled("%.1f ms  (4 ticks = 1 frame @ 60 fps)",
                                g_cfg.delayTicks * 1000.f / 240.f);
            if (changed) saveConfig();
        }
        ImGui::End();
        if (!open) ImGuiCocos::get().setVisible(false);
    });
}
