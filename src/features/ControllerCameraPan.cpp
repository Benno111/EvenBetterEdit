#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/EditButtonBar.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/utils/cocos.hpp>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <optional>

using namespace geode::prelude;

#ifdef GEODE_IS_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
    #include <Xinput.h>

namespace {
    using XInputGetStateFn = DWORD(WINAPI*)(DWORD, XINPUT_STATE*);

    constexpr float TRIGGER_PRESSED = 30.f;
    constexpr float DPAD_REPEAT_DELAY = .28f;
    constexpr float DPAD_REPEAT_RATE = .08f;
    constexpr float CURSOR_SPEED = 700.f;
    constexpr float ZOOM_SPEED = 2.5f;

    XInputGetStateFn getXInputGetState() {
        static XInputGetStateFn fn = []() -> XInputGetStateFn {
            for (auto dll : { "xinput1_4.dll", "xinput1_3.dll", "xinput9_1_0.dll" }) {
                if (auto module = LoadLibraryA(dll)) {
                    if (auto proc = GetProcAddress(module, "XInputGetState")) {
                        return reinterpret_cast<XInputGetStateFn>(proc);
                    }
                }
            }
            return nullptr;
        }();
        return fn;
    }

    float normalizeThumbAxis(SHORT value) {
        return std::clamp(
            value < 0 ? value / 32768.f : value / 32767.f,
            -1.f,
            1.f
        );
    }

    CCPoint getRightStick(XINPUT_STATE const& state) {
        auto stick = ccp(
            normalizeThumbAxis(state.Gamepad.sThumbRX),
            normalizeThumbAxis(state.Gamepad.sThumbRY)
        );

        constexpr float deadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE / 32767.f;
        auto magnitude = std::sqrt(stick.x * stick.x + stick.y * stick.y);
        if (magnitude <= deadzone) {
            return CCPointZero;
        }

        auto adjustedMagnitude = (magnitude - deadzone) / (1.f - deadzone);
        return stick / magnitude * adjustedMagnitude;
    }

    CCPoint getLeftStick(XINPUT_STATE const& state) {
        auto stick = ccp(
            normalizeThumbAxis(state.Gamepad.sThumbLX),
            normalizeThumbAxis(state.Gamepad.sThumbLY)
        );

        constexpr float deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE / 32767.f;
        auto magnitude = std::sqrt(stick.x * stick.x + stick.y * stick.y);
        if (magnitude <= deadzone) {
            return CCPointZero;
        }

        auto adjustedMagnitude = (magnitude - deadzone) / (1.f - deadzone);
        return stick / magnitude * adjustedMagnitude;
    }

    std::optional<XINPUT_STATE> getControllerState() {
        auto getState = getXInputGetState();
        if (!getState) {
            return std::nullopt;
        }

        XINPUT_STATE state {};
        if (getState(0, &state) != ERROR_SUCCESS) {
            return std::nullopt;
        }
        return state;
    }

    bool isButtonDown(XINPUT_STATE const& state, WORD button) {
        return (state.Gamepad.wButtons & button) != 0;
    }

    bool wasButtonPressed(XINPUT_STATE const& state, WORD lastButtons, WORD button) {
        return isButtonDown(state, button) && (lastButtons & button) == 0;
    }

    bool wasTriggerPressed(BYTE value, BYTE lastValue) {
        return value > TRIGGER_PRESSED && lastValue <= TRIGGER_PRESSED;
    }

    std::vector<CCMenuItemSpriteExtra*> getVisibleButtons(CCNode* parent) {
        std::vector<CCMenuItemSpriteExtra*> buttons;
        if (!parent) {
            return buttons;
        }
        for (auto child : CCArrayExt<CCNode*>(parent->getChildren())) {
            if (auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(child)) {
                if (nodeIsVisible(btn) && btn->isEnabled()) {
                    buttons.push_back(btn);
                }
            }
        }
        return buttons;
    }

    CCMenuItemSpriteExtra* buttonAtPoint(CCNode* node, CCPoint const& point) {
        if (!node || !nodeIsVisible(node)) {
            return nullptr;
        }

        auto children = node->getChildren();
        if (children) {
            for (auto i = static_cast<int>(children->count()) - 1; i >= 0; i -= 1) {
                auto child = static_cast<CCNode*>(children->objectAtIndex(i));
                if (auto btn = buttonAtPoint(child, point)) {
                    return btn;
                }
            }
        }

        if (auto btn = typeinfo_cast<CCMenuItemSpriteExtra*>(node)) {
            if (btn->isEnabled() && btn->getParent()) {
                auto localPoint = btn->getParent()->convertToNodeSpace(point);
                if (btn->boundingBox().containsPoint(localPoint)) {
                    return btn;
                }
            }
        }
        return nullptr;
    }

    CCNode* createVirtualCursor() {
        auto cursor = CCNode::create();
        cursor->setID("controller-virtual-cursor"_spr);
        cursor->setContentSize(ccp(18, 18));
        cursor->setAnchorPoint(ccp(.5f, .5f));
        cursor->setZOrder(10000);

        auto outer = CCSprite::createWithSpriteFrameName("GJ_button_04.png");
        outer->setScale(.18f);
        outer->setColor(ccc3(55, 255, 255));
        outer->setOpacity(160);
        cursor->addChildAtPosition(outer, Anchor::Center);

        auto inner = CCSprite::createWithSpriteFrameName("GJ_button_05.png");
        inner->setScale(.08f);
        inner->setColor(ccWHITE);
        cursor->addChildAtPosition(inner, Anchor::Center);

        return cursor;
    }

    void cycleEditorMode(EditorUI* ui, int direction) {
        if (!ui) {
            return;
        }

        std::vector<CCMenuItemSpriteExtra*> modes;
        for (auto btn : { ui->m_buildModeBtn, ui->m_editModeBtn, ui->m_deleteModeBtn }) {
            if (btn && nodeIsVisible(btn) && btn->isEnabled()) {
                modes.push_back(btn);
            }
        }
        if (auto viewBtn = typeinfo_cast<CCMenuItemSpriteExtra*>(ui->querySelector("view-button"_spr))) {
            if (nodeIsVisible(viewBtn) && viewBtn->isEnabled()) {
                modes.push_back(viewBtn);
            }
        }
        if (modes.empty()) {
            return;
        }

        auto current = std::find_if(modes.begin(), modes.end(), [ui](auto btn) {
            return btn->getTag() == ui->m_selectedMode;
        });
        auto index = current == modes.end() ? 0 : static_cast<int>(std::distance(modes.begin(), current));
        auto nextIndex = (index + direction + static_cast<int>(modes.size())) % static_cast<int>(modes.size());
        modes.at(nextIndex)->activate();
    }

    EditButtonBar* getActiveButtonBar(EditorUI* ui) {
        for (auto bar : { ui->m_createButtonBar, ui->m_editButtonBar }) {
            if (bar && nodeIsVisible(bar)) {
                return bar;
            }
        }
        return nullptr;
    }

    void cycleEditorTab(EditorUI* ui, int direction) {
        if (!ui) {
            return;
        }

        if (auto bar = getActiveButtonBar(ui)) {
            if (direction < 0) {
                bar->onLeft(nullptr);
            }
            else {
                bar->onRight(nullptr);
            }
            return;
        }

        if (auto tabs = ui->m_tabsMenu) {
            auto buttons = getVisibleButtons(tabs);
            if (!buttons.empty()) {
                if (direction < 0) {
                    buttons.front()->activate();
                }
                else {
                    buttons.back()->activate();
                }
            }
        }
    }

    void clickVirtualCursor(EditorUI* ui, CCPoint const& pos) {
        if (!ui) {
            return;
        }

        if (auto btn = buttonAtPoint(CCScene::get(), pos)) {
            btn->activate();
            return;
        }
        ui->clickOnPosition(pos);
    }

    void openEditorPause(EditorUI* ui) {
        if (!ui) {
            return;
        }
        ui->onPause(nullptr);
    }
}
#endif

class $modify(RightStickCameraPan, LevelEditorLayer) {
    struct Fields {
        Ref<CCNode> virtualCursor;
        CCPoint virtualCursorPos = CCPointZero;
        uint16_t lastButtons = 0;
        uint8_t lastLeftTrigger = 0;
        uint8_t lastRightTrigger = 0;
        float dpadRepeatTimer = 0.f;
        uint16_t repeatingDpad = 0;
    };

    $override
    void updateVisibility(float dt) {
        LevelEditorLayer::updateVisibility(dt);

        #ifdef GEODE_IS_WINDOWS
        if (m_playbackMode != PlaybackMode::Not) {
            return;
        }

        auto state = getControllerState();
        if (!state) {
            m_fields->lastButtons = 0;
            m_fields->lastLeftTrigger = 0;
            m_fields->lastRightTrigger = 0;
            m_fields->repeatingDpad = 0;
            return;
        }

        auto ui = m_editorUI;
        auto const buttons = state->Gamepad.wButtons;

        if (ui) {
            if (!m_fields->virtualCursor) {
                m_fields->virtualCursor = createVirtualCursor();
                m_fields->virtualCursorPos = CCDirector::get()->getWinSize() / 2.f;
                this->addChild(m_fields->virtualCursor);
            }

            auto leftStick = getLeftStick(*state);
            if (leftStick.x != 0.f || leftStick.y != 0.f) {
                auto winSize = CCDirector::get()->getWinSize();
                m_fields->virtualCursorPos += leftStick * CURSOR_SPEED * dt;
                m_fields->virtualCursorPos.x = std::clamp(m_fields->virtualCursorPos.x, 0.f, winSize.width);
                m_fields->virtualCursorPos.y = std::clamp(m_fields->virtualCursorPos.y, 0.f, winSize.height);
            }
            m_fields->virtualCursor->setPosition(m_fields->virtualCursorPos);

            if (wasButtonPressed(*state, m_fields->lastButtons, XINPUT_GAMEPAD_A)) {
                clickVirtualCursor(ui, m_fields->virtualCursorPos);
            }
            if (wasButtonPressed(*state, m_fields->lastButtons, XINPUT_GAMEPAD_START)) {
                openEditorPause(ui);
            }

            auto const dpad = buttons & (
                XINPUT_GAMEPAD_DPAD_UP |
                XINPUT_GAMEPAD_DPAD_DOWN |
                XINPUT_GAMEPAD_DPAD_LEFT |
                XINPUT_GAMEPAD_DPAD_RIGHT
            );
            auto shouldMove = false;
            if (dpad && dpad != m_fields->repeatingDpad) {
                m_fields->repeatingDpad = dpad;
                m_fields->dpadRepeatTimer = DPAD_REPEAT_DELAY;
                shouldMove = true;
            }
            else if (dpad) {
                m_fields->dpadRepeatTimer -= dt;
                if (m_fields->dpadRepeatTimer <= 0.f) {
                    m_fields->dpadRepeatTimer = DPAD_REPEAT_RATE;
                    shouldMove = true;
                }
            }
            else {
                m_fields->repeatingDpad = 0;
                m_fields->dpadRepeatTimer = 0.f;
            }

            if (shouldMove) {
                if (dpad & XINPUT_GAMEPAD_DPAD_LEFT) {
                    ui->moveObjectCall(EditCommand::Left);
                }
                if (dpad & XINPUT_GAMEPAD_DPAD_RIGHT) {
                    ui->moveObjectCall(EditCommand::Right);
                }
                if (dpad & XINPUT_GAMEPAD_DPAD_UP) {
                    ui->moveObjectCall(EditCommand::Up);
                }
                if (dpad & XINPUT_GAMEPAD_DPAD_DOWN) {
                    ui->moveObjectCall(EditCommand::Down);
                }
            }

            if (wasButtonPressed(*state, m_fields->lastButtons, XINPUT_GAMEPAD_LEFT_SHOULDER)) {
                cycleEditorTab(ui, -1);
            }
            if (wasButtonPressed(*state, m_fields->lastButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
                cycleEditorTab(ui, 1);
            }
            if (wasTriggerPressed(state->Gamepad.bLeftTrigger, m_fields->lastLeftTrigger)) {
                cycleEditorMode(ui, -1);
            }
            if (wasTriggerPressed(state->Gamepad.bRightTrigger, m_fields->lastRightTrigger)) {
                cycleEditorMode(ui, 1);
            }
        }

        if (m_objectLayer) {
            auto stick = getRightStick(*state);
            if (isButtonDown(*state, XINPUT_GAMEPAD_B) && stick.y != 0.f && ui) {
                auto zoom = m_objectLayer->getScale() * std::pow(ZOOM_SPEED, stick.y * dt);
                ui->updateZoom(std::clamp(zoom, .1f, 10000000.f));
            }
            else if (
                Mod::get()->getSettingValue<bool>("right-stick-camera-pan") &&
                (stick.x != 0.f || stick.y != 0.f)
            ) {
                auto speed = Mod::get()->getSettingValue<float>("right-stick-camera-pan-speed");
                m_objectLayer->setPosition(m_objectLayer->getPosition() - stick * speed * dt);
            }
        }

        m_fields->lastButtons = buttons;
        m_fields->lastLeftTrigger = state->Gamepad.bLeftTrigger;
        m_fields->lastRightTrigger = state->Gamepad.bRightTrigger;
        #endif
    }
};
