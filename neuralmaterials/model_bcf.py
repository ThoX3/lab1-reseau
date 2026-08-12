import torch
import torch.nn as nn

class MaterialDecoder(nn.Module):
    def __init__(self, in_channels=12, hidden_dim=16, out_channels=9):
        super(MaterialDecoder, self).__init__()
        # Le réseau ultra-léger décrit par Ubisoft La Forge
        # 12 entrées -> 1 couche cachée de 16 neurones -> Sortie PBR (ex: 9 canaux)
        self.net = nn.Sequential(
            nn.Linear(in_channels, hidden_dim),
            nn.ReLU(),
            nn.Linear(hidden_dim, out_channels)
        )

    def forward(self, x):
        return self.net(x)

class BlockCompressedFeatures(nn.Module):
    def __init__(self, width, height, feature_dim=3):
        super(BlockCompressedFeatures, self).__init__()

        # Le format BC compresse par blocs fixes de 4x4 pixels
        self.blocks_w = width // 4
        self.blocks_h = height // 4
        self.feature_dim = feature_dim

        # 1. Les Endpoints (les "bornes" de nos segments)
        # Shape: [Hauteur_en_blocs, Largeur_en_blocs, 2 (début/fin), Dimension_des_features]
        self.endpoints = nn.Parameter(
            torch.rand(self.blocks_h, self.blocks_w, 2, self.feature_dim)
        )

        # 2. Les Indices (les "poids" d'interpolation pour chaque pixel du bloc 4x4)
        # Shape: [Hauteur_en_blocs, Largeur_en_blocs, 4, 4]
        self.indices = nn.Parameter(
            torch.rand(self.blocks_h, self.blocks_w, 4, 4)
        )

    def forward(self):
        # 1. Contrainte des indices (Émulation de la quantification)
        # On s'assure que l'indice de mélange reste strictement entre 0.0 et 1.0.
        clamped_indices = torch.clamp(self.indices, 0.0, 1.0)

        # 2. Séparation des endpoints (e0 = début du segment, e1 = fin du segment)
        # On ajoute des dimensions pour pouvoir "broadcaster" (multiplier) avec la grille 4x4
        e0 = self.endpoints[:, :, 0, :].unsqueeze(2).unsqueeze(3) # Shape: [H, W, 1, 1, C]
        e1 = self.endpoints[:, :, 1, :].unsqueeze(2).unsqueeze(3) # Shape: [H, W, 1, 1, C]

        # 3. Interpolation linéaire (Le coeur matériel du BC6)
        # Formule : pixel = e0 + indice * (e1 - e0)[cite: 4]
        indices_expanded = clamped_indices.unsqueeze(-1) # Shape: [H, W, 4, 4, 1]
        blocks = e0 + indices_expanded * (e1 - e0)       # Shape: [H, W, 4, 4, C]

        # 4. Unpacking : Réarrangement des blocs 4x4 en une image 2D complète
        # On passe d'un tableau de blocs à une grille de pixels continue
        image = blocks.permute(0, 2, 1, 3, 4).contiguous()
        image = image.view(self.blocks_h * 4, self.blocks_w * 4, self.feature_dim)

        return image
