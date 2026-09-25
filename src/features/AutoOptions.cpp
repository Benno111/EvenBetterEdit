#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

namespace {
void applyAutomaticOptions(GameObject* object) {
    if (!object) {
        return;
    }

    auto mod = Mod::get();
    if (!mod->getSettingValue<bool>("enable-auto-options")) {
        return;
    }

    // Only turn options on. This lets object-specific defaults remain intact and
    // avoids unexpectedly clearing flags set by Geometry Dash or another mod.
    if (mod->getSettingValue<bool>("auto-option-dont-fade")) {
        object->m_isDontFade = true;
    }
    if (mod->getSettingValue<bool>("auto-option-dont-enter")) {
        object->m_isDontEnter = true;
    }
    if (mod->getSettingValue<bool>("auto-option-high-detail")) {
        object->m_isHighDetail = true;
    }
}
}

class $modify(AutoOptionsEditorUI, EditorUI) {
    $override
    GameObject* createObject(int objectID, CCPoint position) {
        auto object = EditorUI::createObject(objectID, position);
        applyAutomaticOptions(object);
        return object;
    }
};
