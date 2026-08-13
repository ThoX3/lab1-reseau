shader_type spatial;
render_mode blend_mix, depth_draw_opaque, cull_back, diffuse_burley, specular_schlick_ggx;

// 4 textures d'entrée
uniform sampler2D feature_tex_0 : filter_linear;
uniform sampler2D feature_tex_1 : filter_linear;
uniform sampler2D feature_tex_2 : filter_linear;
uniform sampler2D feature_tex_3 : filter_linear;

// Les valeurs extrêmes exactes trouvées par ton réseau PyTorch
const float FEATURE_MIN = -5.142467;
const float FEATURE_MAX = 6.809052;

// Les poids du MLP (à remplir dynamiquement via ton code C++)
// Couche 1 : 12 entrées * 16 sorties = 192 poids, 16 biais
uniform float l1_weights[192];
uniform float l1_bias[16];

// Couche 2 : 16 entrées * 9 sorties = 144 poids, 9 biais
uniform float l2_weights[144];
uniform float l2_bias[9];

void fragment() {
    // 1. Lecture des textures d'entrée normalisées
    vec3 f0 = texture(feature_tex_0, UV).rgb;
    vec3 f1 = texture(feature_tex_1, UV).rgb;
    vec3 f2 = texture(feature_tex_2, UV).rgb;
    vec3 f3 = texture(feature_tex_3, UV).rgb;

    // --- DÉ-NORMALISATION ---
    // On étire les données pour restaurer les nombres négatifs originaux !
    f0 = f0 * (FEATURE_MAX - FEATURE_MIN) + FEATURE_MIN;
    f1 = f1 * (FEATURE_MAX - FEATURE_MIN) + FEATURE_MIN;
    f2 = f2 * (FEATURE_MAX - FEATURE_MIN) + FEATURE_MIN;
    f3 = f3 * (FEATURE_MAX - FEATURE_MIN) + FEATURE_MIN;

    // Aplatissement en 12 neurones d'entrée
    float inputs[12] = float[](f0.r, f0.g, f0.b, f1.r, f1.g, f1.b, f2.r, f2.g, f2.b, f3.r, f3.g, f3.b);
    
    // 2. Évaluation de la couche cachée (16 neurones) + ReLU
    float hidden[16];
    for (int i = 0; i < 16; i++) {
        float sum = l1_bias[i];
        for (int j = 0; j < 12; j++) {
            sum += inputs[j] * l1_weights[i * 12 + j];
        }
        hidden[i] = max(0.0, sum); // Fonction d'activation ReLU
    }

    // 3. Évaluation de la couche de sortie (9 neurones)
    float outputs[9];
    for (int i = 0; i < 9; i++) {
        float sum = l2_bias[i];
        for (int j = 0; j < 16; j++) {
            sum += hidden[j] * l2_weights[i * 16 + j];
        }
        outputs[i] = sum; 
    }

    // 4. Assignation des sorties PBR
    vec3 albedo_srgb = clamp(vec3(outputs[0], outputs[1], outputs[2]), vec3(0.0), vec3(1.0));
    ALBEDO = pow(albedo_srgb, vec3(2.2));
    AO = clamp(outputs[6], 0.0, 1.0);
    ROUGHNESS = clamp(outputs[7], 0.0, 1.0);
    METALLIC = 0.0;
}