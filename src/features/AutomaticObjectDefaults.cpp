#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

namespace {
void applyAutomaticObjectDefaults(GameObject* object) {
    if (!object) {
        return;
    }

    auto mod = Mod::get();
    if (!mod->getSettingValue<bool>("enable-auto-object-defaults")) {
        return;
    }

    // Only turn options on so object-specific defaults and flags set by other
    // mods are never cleared when an object is created.
    if (mod->getSettingValue<bool>("auto-default-dont-fade")) {
        object->m_isDontFade = true;
    }
    if (mod->getSettingValue<bool>("auto-default-dont-enter")) {
        object->m_isDontEnter = true;
    }
    if (mod->getSettingValue<bool>("auto-default-high-detail")) {
        object->m_isHighDetail = true;
    }
}
}

class $modify(AutomaticObjectDefaultsEditorUI, EditorUI) {
    $override
    GameObject* createObject(int objectID, CCPoint position) {
        auto object = EditorUI::createObject(objectID, position);
        applyAutomaticObjectDefaults(object);
        return object;
    }
};
