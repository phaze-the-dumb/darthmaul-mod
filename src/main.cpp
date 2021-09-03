#include "main.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "HMUI/CurvedTextMeshPro.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Vector3.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "questui/shared/BeatSaberUI.hpp"
#include "HMUI/Touchable.hpp"
#include "questui/shared/QuestUI.hpp"
#include "config-utils/shared/config-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "ModConfig.hpp"
#include "System/Action.hpp"
#include "GlobalNamespace/PlayerTransforms.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberClashEffect.hpp"

using namespace QuestUI;
using namespace UnityEngine;
using namespace GlobalNamespace;

float xa = 0.0f;
float ya = 0.0f;
float za = 0.0f;

float xb = 0.0f;
float yb = 0.0f;
float zb = 0.0f;

float xr = 0.0f;
float yr = 0.0f;
float zr = 0.0f;

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup
DEFINE_CONFIG(ModConfig);

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

MAKE_HOOK_MATCH(SaberClashEffect_LateUpdate, &SaberClashEffect::LateUpdate, void,
    SaberClashEffect* self
) {
    if(getModConfig().Active.GetValue()){
        return;
    } else{
        SaberClashEffect_LateUpdate(self);
    }
};

MAKE_HOOK_MATCH(PlayerTransforms_Update, &PlayerTransforms::Update, void,
    PlayerTransforms* self
) {
    if(getModConfig().Active.GetValue()){
        if(getModConfig().OneHand.GetValue()){
            if(getModConfig().LeftHanded.GetValue()){
                self->rightHandTransform->get_transform()->set_localPosition({
                    self->leftHandTransform->get_position().x,
                    self->leftHandTransform->get_position().y,
                    self->leftHandTransform->get_position().z
                });

                self->rightHandTransform->get_transform()->set_eulerAngles({
                    self->leftHandTransform->get_eulerAngles().x + 180,
                    self->leftHandTransform->get_eulerAngles().y,
                    self->leftHandTransform->get_eulerAngles().z,
                });
            } else{
                self->leftHandTransform->get_transform()->set_localPosition({
                    self->rightHandTransform->get_position().x,
                    self->rightHandTransform->get_position().y,
                    self->rightHandTransform->get_position().z
                });

                self->leftHandTransform->get_transform()->set_eulerAngles({
                    self->rightHandTransform->get_eulerAngles().x + 180,
                    self->rightHandTransform->get_eulerAngles().y,
                    self->rightHandTransform->get_eulerAngles().z,
                });
            }
            
        } else{
            xa = self->rightHandTransform->get_position().x;
            ya = self->rightHandTransform->get_position().y;
            za = self->rightHandTransform->get_position().z;

            xb = self->leftHandTransform->get_position().x;
            yb = self->leftHandTransform->get_position().y;
            zb = self->leftHandTransform->get_position().z;

            float xAverage = (xa + xb) / 2;
            float yAverage = (ya + yb) / 2;
            float zAverage = (za + zb) / 2;

            Quaternion rotation = UnityEngine::Quaternion::LookRotation(self->rightHandTransform->get_position() - self->leftHandTransform->get_position());
        
            self->rightHandTransform->get_transform()->set_localPosition({xAverage, yAverage, zAverage});
            self->rightHandTransform->get_transform()->set_eulerAngles(rotation.get_eulerAngles());

            self->leftHandTransform->get_transform()->set_localPosition({xAverage, yAverage, zAverage});
            self->leftHandTransform->get_transform()->set_eulerAngles({
                rotation.get_eulerAngles().x + 180,
                rotation.get_eulerAngles().y,
                rotation.get_eulerAngles().z * -1,
            });
        }
    }
    
    PlayerTransforms_Update(self);
};

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}

void DidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling){
    if(firstActivation){
        GameObject* container = BeatSaberUI::CreateScrollableSettingsContainer(self->get_transform());

        BeatSaberUI::CreateToggle(container->get_transform(), "Active", getModConfig().Active.GetValue(),
            [](bool value) { 
                getModConfig().Active.SetValue(value);
            });

        BeatSaberUI::CreateToggle(container->get_transform(), "One Handed Mode", getModConfig().OneHand.GetValue(),
            [](bool value) { 
                getModConfig().OneHand.SetValue(value);
            });

        BeatSaberUI::CreateToggle(container->get_transform(), "Left Handed Mode", getModConfig().LeftHanded.GetValue(),
            [](bool value) { 
                getModConfig().LeftHanded.SetValue(value);
            });
    }
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();
    getModConfig().Init(modInfo);

    LoggerContextObject logger = getLogger().WithContext("load");

    QuestUI::Init();
    QuestUI::Register::RegisterModSettingsViewController(modInfo, DidActivate);
    QuestUI::Register::RegisterMainMenuModSettingsViewController(modInfo, DidActivate);
    getLogger().info("Successfully installed Settings UI!");

    getLogger().info("Installing hooks...");
    INSTALL_HOOK(logger, PlayerTransforms_Update);
    INSTALL_HOOK(logger, SaberClashEffect_LateUpdate);
    getLogger().info("Installed all hooks!");
}