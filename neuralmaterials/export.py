import torch
import numpy as np
import cv2
import os
import json

from model_bcf import BlockCompressedFeatures, MaterialDecoder

RESOLUTION = 512

def main():
    print("[+] Début de l'exportation des ressources...")
    export_dir = "./build/export"
    os.makedirs(export_dir, exist_ok=True)

    if not os.path.exists("./build/model_final.pt"):
        raise FileNotFoundError("Le fichier modèle est introuvable. Lance train.py d'abord.")

    checkpoint = torch.load("./build/model_final.pt", map_location="cpu")

    # 1. Extraction des Features Latentes
    print("[+] Extraction des Neural Features...")
    features = BlockCompressedFeatures(width=RESOLUTION, height=RESOLUTION, feature_dim=12)
    features.load_state_dict(checkpoint['features_state'])

    latent_image = features().detach().numpy()

    for i in range(4):
        tex_data = latent_image[:, :, i*3:(i+1)*3]
        tex_data = np.clip(tex_data * 255.0, 0, 255).astype(np.uint8)
        tex_data = cv2.cvtColor(tex_data, cv2.COLOR_RGB2BGR)
        cv2.imwrite(f"{export_dir}/neural_feature_{i}.png", tex_data)

    print(f"[+] 4 Textures de features exportées avec succès dans {export_dir}.")

    # 2. Extraction des Poids du MLP
    print("[+] Extraction des poids du décodeur...")
    decoder = MaterialDecoder(in_channels=12, hidden_dim=16, out_channels=9)
    decoder.load_state_dict(checkpoint['decoder_state'])

    weights_dict = {}
    for name, param in decoder.named_parameters():
        weights_dict[name] = param.detach().numpy().flatten().tolist()

    with open(f"{export_dir}/mlp_weights.json", "w") as f:
        json.dump(weights_dict, f, indent=4)

    print(f"[+] Poids du MLP exportés dans {export_dir}/mlp_weights.json")
    print("[+] Exportation terminée avec succès !")

if __name__ == "__main__":
    main()
