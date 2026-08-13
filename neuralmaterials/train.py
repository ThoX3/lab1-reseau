import os
import torch
import torch.nn as nn
import torch.optim as optim
import numpy as np
from PIL import Image

from model_bcf import MaterialDecoder, BlockCompressedFeatures

EPOCHS = 5000
LEARNING_RATE_FEATURES = 0.05
LEARNING_RATE_MLP = 0.001
RESOLUTION = 2048

def load_texture(path):
    if not os.path.exists(path):
        raise FileNotFoundError(f"Texture introuvable : {os.path.abspath(path)}")

    # Lecture universelle et sans erreur via Pillow (JPG/PNG)
    img = Image.open(path).convert('RGB')
    img = img.resize((RESOLUTION, RESOLUTION))

    img_array = np.array(img, dtype=np.float32) / 255.0

    if len(img_array.shape) == 2:
        img_array = np.expand_dims(img_array, axis=-1)

    return torch.tensor(img_array, dtype=torch.float32)

def main():
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"[+] Démarrage de l'entraînement sur : {device}")

    print("[+] Chargement des textures PBR depuis data/textures/...")

    # Utilisation des fichiers standards .jpg et .png présents dans le dossier
    albedo = load_texture("./data/textures/square_brick_floor_diff_4k.jpg")
    normal = load_texture("./data/textures/square_brick_floor_disp_4k.png")
    roughness = load_texture("./data/textures/square_brick_floor_disp_4k.png")[..., 0:1]

    # Construction de la map ARM (Ambient Occlusion, Roughness, Metalness)
    ao = torch.ones((RESOLUTION, RESOLUTION, 1), dtype=torch.float32)
    metal = torch.zeros((RESOLUTION, RESOLUTION, 1), dtype=torch.float32)

    arm = torch.cat([ao, roughness, metal], dim=-1)
    ground_truth = torch.cat([albedo, normal, arm], dim=-1).to(device)

    # Instanciation de l'architecture Block-Compressed
    features = BlockCompressedFeatures(width=RESOLUTION, height=RESOLUTION, feature_dim=12).to(device)
    decoder = MaterialDecoder(in_channels=12, hidden_dim=16, out_channels=9).to(device)

    optimizer = optim.Adam([
        {'params': features.parameters(), 'lr': LEARNING_RATE_FEATURES},
        {'params': decoder.parameters(), 'lr': LEARNING_RATE_MLP}
    ])
    criterion = nn.MSELoss()

    print("[+] Début de la boucle d'optimisation sur GPU...")
    for epoch in range(EPOCHS):
        optimizer.zero_grad()

        latent_map = features()
        latent_flat = latent_map.view(-1, 12)
        predicted_flat = decoder(latent_flat)
        predicted_image = predicted_flat.view(RESOLUTION, RESOLUTION, 9)

        loss = criterion(predicted_image, ground_truth)
        loss.backward()
        optimizer.step()

        if epoch % 500 == 0:
            print(f"Epoch {epoch}/{EPOCHS} | Loss (MSE): {loss.item():.6f}")

    os.makedirs("./build", exist_ok=True)
    torch.save({
        'features_state': features.state_dict(),
        'decoder_state': decoder.state_dict()
    }, "./build/model_final.pt")

    print("[+] Entraînement terminé. Modèle sauvegardé dans ./build/model_final.pt")

if __name__ == "__main__":
    main()
