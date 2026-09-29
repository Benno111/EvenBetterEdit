#include <Geode/binding/EditLevelLayer.hpp>
#include <Geode/binding/LocalLevelManager.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/ui/Popup.hpp>

#include <functional>
#include <unordered_set>

using namespace geode::prelude;

namespace {
class LevelCopyOptionsPopup : public Popup {
protected:
    CCMenuItemToggler* m_attemptsToggle = nullptr;
    CCMenuItemToggler* m_progressToggle = nullptr;
    std::function<void(bool, bool)> m_callback;

    bool init(std::function<void(bool, bool)> callback) {
        if (!Popup::init(300.f, 190.f))
            return false;

        m_callback = std::move(callback);
        this->setTitle("Level Copy");

        auto description = CCLabelBMFont::create(
            "Choose which data to keep in the copy.", "goldFont.fnt"
        );
        description->setScale(.45f);
        m_mainLayer->addChildAtPosition(description, Anchor::Top, ccp(0.f, -48.f));

        auto addOption = [this](char const* text, float y) {
            auto menu = CCMenu::create();
            menu->setContentSize(ccp(220.f, 30.f));

            auto toggle = CCMenuItemToggler::createWithStandardSprites(
                this, menu_selector(LevelCopyOptionsPopup::onToggle), .7f
            );
            toggle->m_notClickable = true;
            menu->addChildAtPosition(toggle, Anchor::Left, ccp(12.f, 0.f));

            auto label = CCLabelBMFont::create(text, "bigFont.fnt");
            label->setScale(.45f);
            menu->addChildAtPosition(label, Anchor::Left, ccp(34.f, 0.f), ccp(0.f, .5f));
            m_mainLayer->addChildAtPosition(menu, Anchor::Center, ccp(0.f, y));
            return toggle;
        };

        m_attemptsToggle = addOption("Copy attempts and jumps", 24.f);
        m_progressToggle = addOption("Copy normal and practice progress", -10.f);

        auto copySprite = ButtonSprite::create("Copy", "goldFont.fnt", "GJ_button_01.png", .8f);
        auto copyButton = CCMenuItemSpriteExtra::create(
            copySprite, this, menu_selector(LevelCopyOptionsPopup::onCopy)
        );
        m_buttonMenu->addChildAtPosition(copyButton, Anchor::Bottom, ccp(0.f, 24.f));
        return true;
    }

    void onToggle(CCObject* sender) {
        auto toggle = static_cast<CCMenuItemToggler*>(sender);
        toggle->toggle(!toggle->m_toggled);
    }

    void onCopy(CCObject*) {
        auto callback = std::move(m_callback);
        auto copyAttempts = m_attemptsToggle->m_toggled;
        auto copyProgress = m_progressToggle->m_toggled;
        this->onClose(nullptr);
        callback(copyAttempts, copyProgress);
    }

public:
    static LevelCopyOptionsPopup* create(std::function<void(bool, bool)> callback) {
        auto result = new LevelCopyOptionsPopup();
        if (result->init(std::move(callback))) {
            result->autorelease();
            return result;
        }
        CC_SAFE_DELETE(result);
        return nullptr;
    }
};
}

class $modify(LevelCopyOptionsLayer, EditLevelLayer) {
    struct Fields {
        bool showingCopyOptions = false;
    };

    void onClone() {
        if (m_fields->showingCopyOptions) {
            m_fields->showingCopyOptions = false;
            EditLevelLayer::onClone();
            return;
        }

        auto popup = LevelCopyOptionsPopup::create([self = Ref(this)](bool attempts, bool progress) {
            auto manager = LocalLevelManager::get();
            std::unordered_set<GJGameLevel*> oldLevels;
            for (auto level : CCArrayExt<GJGameLevel*>(manager->m_localLevels))
                oldLevels.insert(level);

            auto source = self->m_level;
            self->m_fields->showingCopyOptions = true;
            self->onClone();

            for (auto level : CCArrayExt<GJGameLevel*>(manager->m_localLevels)) {
                if (oldLevels.contains(level))
                    continue;

                if (attempts) {
                    level->m_attempts = source->m_attempts;
                    level->m_jumps = source->m_jumps;
                    level->m_clicks = source->m_clicks;
                    level->m_attemptTime = source->m_attemptTime;
                }
                if (progress) {
                    level->m_normalPercent = source->m_normalPercent;
                    level->m_practicePercent = source->m_practicePercent;
                }
                break;
            }
        });
        if (popup)
            popup->show();
    }
};
