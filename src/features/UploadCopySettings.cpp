#include <Geode/binding/GJGameLevel.hpp>
#include <Geode/modify/ShareLevelLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

#include <charconv>
#include <functional>

using namespace geode::prelude;

namespace {
class UploadCopySettingsPopup : public Popup {
protected:
    CCMenuItemToggler* m_allowCopyToggle = nullptr;
    CCMenuItemToggler* m_freeCopyToggle = nullptr;
    TextInput* m_passwordInput = nullptr;
    std::function<void(int)> m_callback;

    bool init(int password, std::function<void(int)> callback) {
        if (!Popup::init(310.f, 220.f))
            return false;

        m_callback = std::move(callback);
        this->setTitle("Level Copy Settings");

        auto addToggle = [this](char const* text, float y) {
            auto menu = CCMenu::create();
            menu->setContentSize(ccp(240.f, 30.f));

            auto toggle = CCMenuItemToggler::createWithStandardSprites(
                this, menu_selector(UploadCopySettingsPopup::onToggle), .7f
            );
            toggle->m_notClickable = true;
            menu->addChildAtPosition(toggle, Anchor::Left, ccp(12.f, 0.f));

            auto label = CCLabelBMFont::create(text, "bigFont.fnt");
            label->setScale(.45f);
            menu->addChildAtPosition(label, Anchor::Left, ccp(34.f, 0.f), ccp(0.f, .5f));
            m_mainLayer->addChildAtPosition(menu, Anchor::Center, ccp(0.f, y));
            return toggle;
        };

        m_allowCopyToggle = addToggle("Allow level copying", 42.f);
        m_freeCopyToggle = addToggle("Free copy (no password)", 8.f);
        m_allowCopyToggle->toggle(password != 0);
        m_freeCopyToggle->toggle(password == 1);

        auto passwordLabel = CCLabelBMFont::create("Password:", "bigFont.fnt");
        passwordLabel->setScale(.4f);
        m_mainLayer->addChildAtPosition(passwordLabel, Anchor::Center, ccp(-58.f, -29.f));

        m_passwordInput = TextInput::create(90.f, "2-999999");
        m_passwordInput->setFilter("0123456789");
        m_passwordInput->setMaxCharCount(6);
        if (password > 1)
            m_passwordInput->setString(fmt::format("{}", password));
        m_mainLayer->addChildAtPosition(m_passwordInput, Anchor::Center, ccp(38.f, -29.f));

        auto saveSprite = ButtonSprite::create("Save", "goldFont.fnt", "GJ_button_01.png", .8f);
        auto saveButton = CCMenuItemSpriteExtra::create(
            saveSprite, this, menu_selector(UploadCopySettingsPopup::onSave)
        );
        m_buttonMenu->addChildAtPosition(saveButton, Anchor::Bottom, ccp(0.f, 24.f));

        this->updateState();
        return true;
    }

    void onToggle(CCObject* sender) {
        auto toggle = static_cast<CCMenuItemToggler*>(sender);
        toggle->toggle(!toggle->m_toggled);
        if (toggle == m_freeCopyToggle && toggle->m_toggled && !m_allowCopyToggle->m_toggled)
            m_allowCopyToggle->toggle(true);
        this->updateState();
    }

    void updateState() {
        auto passwordRequired = m_allowCopyToggle->m_toggled && !m_freeCopyToggle->m_toggled;
        m_freeCopyToggle->setOpacity(m_allowCopyToggle->m_toggled ? 255 : 100);
        m_passwordInput->setVisible(passwordRequired);
    }

    void onSave(CCObject*) {
        int password = 0;
        if (m_allowCopyToggle->m_toggled) {
            password = 1;
            if (!m_freeCopyToggle->m_toggled) {
                auto const& text = m_passwordInput->getString();
                auto result = std::from_chars(text.data(), text.data() + text.size(), password);
                if (
                    result.ec != std::errc() || result.ptr != text.data() + text.size() ||
                    password < 2 || password > 999999
                ) {
                    FLAlertLayer::create(
                        "Invalid Password",
                        "Enter a numeric password from <cy>2</c> to <cy>999999</c>.",
                        "OK"
                    )->show();
                    return;
                }
            }
        }

        auto callback = std::move(m_callback);
        this->onClose(nullptr);
        callback(password);
    }

public:
    static UploadCopySettingsPopup* create(int password, std::function<void(int)> callback) {
        auto result = new UploadCopySettingsPopup();
        if (result->init(password, std::move(callback))) {
            result->autorelease();
            return result;
        }
        CC_SAFE_DELETE(result);
        return nullptr;
    }
};
}

class $modify(UploadCopySettingsLayer, ShareLevelLayer) {
    struct Fields {
        GJGameLevel* level = nullptr;
    };

    bool init(GJGameLevel* level) {
        if (!ShareLevelLayer::init(level))
            return false;

        m_fields->level = level;

        auto sprite = ButtonSprite::create("Copy Settings", "goldFont.fnt", "GJ_button_04.png", .65f);
        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(UploadCopySettingsLayer::onCopySettings)
        );
        auto menu = CCMenu::create();
        menu->addChild(button);
        menu->setPosition(CCDirector::get()->getWinSize() / 2 + ccp(125.f, -112.f));
        this->addChild(menu);
        return true;
    }

    void onCopySettings(CCObject*) {
        auto level = m_fields->level;
        auto popup = UploadCopySettingsPopup::create(level->m_password, [level = Ref(level)](int password) {
            level->m_password = password;
        });
        if (popup)
            popup->show();
    }
};
