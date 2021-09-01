#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(Active, bool, "Active", true);
    CONFIG_VALUE(OneHand, bool, "One Hand", false);
    CONFIG_VALUE(LeftHanded, bool, "Left Handed", false);

    CONFIG_INIT_FUNCTION(
        CONFIG_INIT_VALUE(Active);
        CONFIG_INIT_VALUE(OneHand);
        CONFIG_INIT_VALUE(LeftHanded);
    )
)