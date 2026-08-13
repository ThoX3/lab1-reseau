#include "neural_material.h"
#include "core/io/json.h"
#include "core/io/file_access.h"

NeuralMaterial::NeuralMaterial() {
}

NeuralMaterial::~NeuralMaterial() {
}

void NeuralMaterial::load_weights_from_json(const String &p_path) {
    Ref<FileAccess> file = FileAccess::open(p_path, FileAccess::READ);
    ERR_FAIL_COND_MSG(file.is_null(), "Impossible d'ouvrir le fichier de poids MLP : " + p_path);

    String json_text = file->get_as_text();
    JSON json;
    Error err = json.parse(json_text);
    ERR_FAIL_COND_MSG(err != OK, "Erreur de parsing JSON pour les poids du MLP.");

    Dictionary weights_dict = json.get_data();

    if (weights_dict.has("net.0.weight")) {
        Array l1_w = weights_dict["net.0.weight"];
        l1_weights.resize(l1_w.size());
        for (int i = 0; i < l1_w.size(); i++) {
            l1_weights.write[i] = (float)l1_w[i];
        }
    }
    if (weights_dict.has("net.0.bias")) {
        Array l1_b = weights_dict["net.0.bias"];
        l1_bias.resize(l1_b.size());
        for (int i = 0; i < l1_b.size(); i++) {
            l1_bias.write[i] = (float)l1_b[i];
        }
    }
    if (weights_dict.has("net.2.weight")) {
        Array l2_w = weights_dict["net.2.weight"];
        l2_weights.resize(l2_w.size());
        for (int i = 0; i < l2_w.size(); i++) {
            l2_weights.write[i] = (float)l2_w[i];
        }
    }
    if (weights_dict.has("net.2.bias")) {
        Array l2_b = weights_dict["net.2.bias"];
        l2_bias.resize(l2_b.size());
        for (int i = 0; i < l2_b.size(); i++) {
            l2_bias.write[i] = (float)l2_b[i];
        }
    }
}

void NeuralMaterial::set_feature_texture(int p_index, const Ref<Texture2D> &p_texture) {
    ERR_FAIL_INDEX(p_index, 4);
    features_textures[p_index] = p_texture;
    emit_changed();
}

Ref<Texture2D> NeuralMaterial::get_feature_texture(int p_index) const {
    ERR_FAIL_INDEX_V(p_index, 4, Ref<Texture2D>());
    return features_textures[p_index];
}

void NeuralMaterial::_bind_methods() {
    ClassDB::bind_method(D_METHOD("load_weights_from_json", "path"), &NeuralMaterial::load_weights_from_json);
    ClassDB::bind_method(D_METHOD("set_feature_texture", "index", "texture"), &NeuralMaterial::set_feature_texture);
    ClassDB::bind_method(D_METHOD("get_feature_texture", "index"), &NeuralMaterial::get_feature_texture);
}
