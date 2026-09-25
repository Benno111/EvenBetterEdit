#include <Geode/Geode.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/binding/PlayerObject.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/utils/cocos.hpp>

using namespace geode::prelude;

namespace {
bool g_enableObjectSpawn = false;
CCMenuItemSpriteExtra* g_toggleBtn = nullptr;

void placeCustomObject(PlayerObject* player, int holdState) {
    if (!g_enableObjectSpawn || !player || !player->m_editorEnabled) {
        return;
    }

    auto editor = LevelEditorLayer::get();
    if (!editor || editor->m_playbackMode != PlaybackMode::Playing) {
        return;
    }

    auto position = player->getPosition();
    position.y -= 90.f;

    auto settingValue = Mod::get()->getSettingValue<int64_t>("custom-setting-value");
    auto playerNumber = player->m_isSecondPlayer ? 199 : 165;

    auto objectString = fmt::format(
        "1,2899,2,{},3,{},{},{}",
        position.x,
        position.y,
        playerNumber,
        holdState
    );

    if (settingValue > 0) {
        objectString += fmt::format(",33,{}", settingValue);
    }

    editor->createObjectsFromString(objectString.c_str(), false, false);
}

void setToggleVisible(bool visible) {
    if (g_toggleBtn) {
        g_toggleBtn->setVisible(visible);
    }
}
}

class $modify(AutoOptionsPlayerObject, PlayerObject) {
    $override
    bool pushButton(PlayerButton button) {
        auto result = PlayerObject::pushButton(button);

        if (button == PlayerButton::Jump) {
            placeCustomObject(this, -1);
        }

        return result;
    }

    $override
    bool releaseButton(PlayerButton button) {
        auto result = PlayerObject::releaseButton(button);

        if (button == PlayerButton::Jump) {
            placeCustomObject(this, 1);
        }

        return result;
    }
};

class $modify(AutoOptionsEditorLayer, LevelEditorLayer) {
    $override
    void onPlaytest() {
        setToggleVisible(false);
        LevelEditorLayer::onPlaytest();
    }

#ifdef GEODE_IS_ANDROID
    $override
    void onResumePlaytest() {
        setToggleVisible(false);
        LevelEditorLayer::onResumePlaytest();
    }

    $override
    void onPausePlaytest() {
        setToggleVisible(true);
        LevelEditorLayer::onPausePlaytest();
    }
#endif

    $override
    void onStopPlaytest() {
        setToggleVisible(true);
        LevelEditorLayer::onStopPlaytest();
    }
};

class $modify(AutoOptionsEditorUI, EditorUI) {
    void updateAutoOptionsButton() {
        if (!g_toggleBtn) {
            return;
        }

        auto sprite = ButtonSprite::create(
            "Auto\nOptions",
            25,
            true,
            "bigFont.fnt",
            "GJ_button_01.png",
            40.f,
            .6f
        );
        sprite->setColor(g_enableObjectSpawn ? ccWHITE : ccColor3B {100, 100, 100});
        g_toggleBtn->setNormalImage(sprite);
    }

    void onToggleAutoOptions(CCObject*) {
        g_enableObjectSpawn = !g_enableObjectSpawn;
        updateAutoOptionsButton();
    }

#ifndef GEODE_IS_ANDROID
    $override
    void onPlaytest(CCObject* sender) {
        EditorUI::onPlaytest(sender);
        if (m_editorLayer->m_playbackMode == PlaybackMode::Paused) {
            setToggleVisible(true);
        }
    }
#endif

    $override
    bool init(LevelEditorLayer* editor) {
        if (!EditorUI::init(editor)) {
            return false;
        }

        g_enableObjectSpawn = false;
        g_toggleBtn = nullptr;

        if (Mod::get()->getSettingValue<bool>("hideBtn")) {
            return true;
        }

        auto sprite = ButtonSprite::create(
            "Auto\nOptions",
            25,
            true,
            "bigFont.fnt",
            "GJ_button_01.png",
            40.f,
            .6f
        );
        sprite->setColor({100, 100, 100});

        g_toggleBtn = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(AutoOptionsEditorUI::onToggleAutoOptions)
        );

        if (m_playtestBtn && m_playtestBtn->getParent()) {
            g_toggleBtn->setPosition(m_playtestBtn->getPosition() + ccp(70.f, 0.f));
            m_playtestBtn->getParent()->addChild(g_toggleBtn);
        } else {
            g_toggleBtn = nullptr;
        }

        return true;
    }
};
