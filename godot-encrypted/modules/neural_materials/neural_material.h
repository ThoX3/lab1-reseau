#ifndef NEURAL_MATERIAL_H
#define NEURAL_MATERIAL_H

#pragma once

#include "scene/resources/material.h" // Contient à la fois Material et ShaderMaterial
#include "scene/resources/texture.h"

class NeuralMaterial : public ShaderMaterial {
    GDCLASS(NeuralMaterial, ShaderMaterial);

private:
    Ref<Texture2D> features_textures[4];

    Vector<float> l1_weights;
    Vector<float> l1_bias;
    Vector<float> l2_weights;
    Vector<float> l2_bias;

    void _update_shader_uniforms();

protected:
    static void _bind_methods();

public:
    NeuralMaterial();
    ~NeuralMaterial() override;

    void load_weights_from_json(const String &p_path);
    void set_feature_texture(int p_index, const Ref<Texture2D> &p_texture);
    Ref<Texture2D> get_feature_texture(int p_index) const;
};

#endif // NEURAL_MATERIAL_H
