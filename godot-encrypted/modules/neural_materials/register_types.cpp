#include "register_types.h"
#include "neural_material.h"

#include "core/object/class_db.h"

void initialize_neural_materials_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    GDREGISTER_CLASS(NeuralMaterial);
}

void uninitialize_neural_materials_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}
