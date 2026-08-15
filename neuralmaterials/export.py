import os
import torch
import numpy as np
import cv2
import json

from model_bcf import BlockCompressedFeatures, MaterialDecoder

RESOLUTION = 2048

# Transformation de main() en une fonction paramétrable
def run_export(model_path, export_dir):
    print(f"[+] Début de l'exportation des ressources dans {export_dir}...")
    os.makedirs(export_dir, exist_ok=True)

    # On utilise maintenant la variable model_path envoyée par la boucle
    checkpoint = torch.load(model_path, map_location="cpu", weights_only=True)

    print("[+] Extraction des Neural Features...")
    features = BlockCompressedFeatures(width=RESOLUTION, height=RESOLUTION, feature_dim=12)
    features.load_state_dict(checkpoint['features_state'])

    latent_image = features().detach().numpy().astype(np.float32)

    # --- NORMALISATION ---
    min_val = float(np.min(latent_image))
    max_val = float(np.max(latent_image))

    print("\n" + "="*60)
    print("!!! VALEURS CRITIQUES À COPIER DANS TON SHADER GODOT !!!")
    print(f"const float FEATURE_MIN = {min_val:.6f};")
    print(f"const float FEATURE_MAX = {max_val:.6f};")
    print("="*60 + "\n")

    latent_norm = (latent_image - min_val) / (max_val - min_val)

    for i in range(4):
        tex_data = latent_norm[:, :, i*3:(i+1)*3]
        tex_data_bgr = cv2.cvtColor(tex_data, cv2.COLOR_RGB2BGR)
        # On utilise la variable export_dir envoyée par la boucle
        cv2.imwrite(f"{export_dir}/neural_feature_{i}.hdr", tex_data_bgr)

    print(f"[+] 4 Textures HDR exportées avec succès.")

    # Extraction des Poids du MLP
    decoder = MaterialDecoder(in_channels=12, hidden_dim=16, out_channels=9)
    decoder.load_state_dict(checkpoint['decoder_state'])

    weights_dict = {}
    for name, param in decoder.named_parameters():
        weights_dict[name] = param.detach().numpy().flatten().tolist()

    with open(f"{export_dir}/mlp_weights.json", "w") as f:
        json.dump(weights_dict, f, indent=4)

    print(f"[+] Poids du MLP exportés dans {export_dir}/mlp_weights.json\n")

# Si on lance le script tout seul, on fait un test par défaut
if __name__ == "__main__":
    run_export("./build/model_final.pt", "./build/export")
