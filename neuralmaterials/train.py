import os

os.environ["OPENCV_IO_ENABLE_OPENEXR"] = "1"
from pathlib import Path
import cv2
import imageio
import imageio.plugins.freeimage
import torch
import torch.nn as nn
import torch.optim as optim
import numpy as np
import OpenEXR
import Imath
from PIL import Image

from model_bcf import MaterialDecoder, BlockCompressedFeatures

EPOCHS = 5000
LEARNING_RATE_FEATURES = 0.05
LEARNING_RATE_MLP = 0.001
RESOLUTION = 2048

def load_exr(path):
    exr_file = OpenEXR.InputFile(path)
    header = exr_file.header()
    dw = header['dataWindow']
    width = dw.max.x - dw.min.x + 1
    height = dw.max.y - dw.min.y + 1

    pt = Imath.PixelType(Imath.PixelType.FLOAT)
    available = list(header['channels'].keys())

    # Try common orderings for color, then fall back to single-channel greyscale
    if all(c in available for c in ('R', 'G', 'B')):
        chan_names = ['R', 'G', 'B']
    elif 'Y' in available:
        chan_names = ['Y']
    elif 'G' in available:
        chan_names = ['G']
    elif len(available) >= 1:
        chan_names = [available[0]]
    else:
        raise RuntimeError(f"Aucun canal lisible trouvé dans {path}: {available}")

    arrays = []
    for c in chan_names:
        raw = exr_file.channel(c, pt)
        arr = np.frombuffer(raw, dtype=np.float32).reshape((height, width))
        arrays.append(arr)

    img = np.stack(arrays, axis=-1)
    return img

def load_texture(path):
    if not os.path.exists(path):
        raise FileNotFoundError(f"Texture introuvable : {os.path.abspath(path)}")

    if path.lower().endswith(('.exr', '.hdr')):
        img_array = load_exr(path)
        img_array = cv2.resize(img_array, (RESOLUTION, RESOLUTION), interpolation=cv2.INTER_LINEAR)
        img_array = img_array.astype(np.float32)
    else:
        img = Image.open(path).convert('RGB')
        img = img.resize((RESOLUTION, RESOLUTION))
        img_array = np.array(img, dtype=np.float32) / 255.0

    if len(img_array.shape) == 2:
        img_array = np.expand_dims(img_array, axis=-1)
    return torch.tensor(img_array, dtype=torch.float32)

def run_training(diffuse_path, normal_path, roughness_path, output_model_path):
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    print(f"[+] Démarrage de l'entraînement sur : {device}")

    print(f"[+] Chargement des textures PBR depuis le dossier en cours...")

    # Utilisation des variables envoyées par train_all.py au lieu des chemins codés en dur
    albedo = load_texture(diffuse_path)
    normal = load_texture(normal_path)
    roughness = load_texture(roughness_path)[..., 0:1]

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

    # Création du dossier d'arrivée (au cas où) et sauvegarde dynamique
    os.makedirs(os.path.dirname(output_model_path), exist_ok=True)
    torch.save({
        'features_state': features.state_dict(),
        'decoder_state': decoder.state_dict()
    }, output_model_path)

    print(f"[+] Entraînement terminé. Modèle sauvegardé dans {output_model_path}")

if __name__ == "__main__":
    print("⚠️ Pour lancer l'entraînement, exécutez le script d'automatisation : python train_all.py")
