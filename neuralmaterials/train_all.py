import os
from pathlib import Path

# ==========================================
# IMPORTS DE TES SCRIPTS
# ==========================================
from train import run_training
from export import run_export

def main():
    # Définition des dossiers racines
    data_dir = Path("data")
    build_dir = Path("build")

    if not data_dir.exists():
        print(f"❌ Le dossier {data_dir} n'existe pas à la racine.")
        return

    # 1. Parcourir les catégories (ex: asphalt, brick, ceramic...)
    for category_path in data_dir.iterdir():
        if not category_path.is_dir():
            continue
        category_name = category_path.name

        # 2. Parcourir les types de matériaux (ex: clean, damaged...)
        for type_path in category_path.iterdir():
            if not type_path.is_dir():
                continue
            type_name = type_path.name

            textures_dir = type_path / "textures"

            # Vérifier si le dossier textures existe bien
            if not textures_dir.exists():
                print(f"⚠️ Ignoré : Pas de sous-dossier 'textures' dans {type_path}")
                continue

            # 3. Créer l'arborescence miroir dans le dossier build/
            output_dir = build_dir / category_name / type_name
            export_dir = output_dir / "export"

            output_dir.mkdir(parents=True, exist_ok=True)
            export_dir.mkdir(parents=True, exist_ok=True)

            print(f"\n🚀 --- Début du traitement : {category_name} / {type_name} ---")

            # 4. Récupérer automatiquement les fichiers textures peu importe leurs noms
            try:
                diffuse_file = next(textures_dir.glob("*diff*.*"))
                normal_file = next(textures_dir.glob("*nor*.*"))
                roughness_file = next(textures_dir.glob("*rough*.*"))

            except StopIteration:
                print(f"❌ Erreur : Il manque une texture de base dans {textures_dir}")
                continue

            model_output_path = output_dir / "model_final.pt"

            print(f"Textures trouvées dans {textures_dir.name} :")
            print(f" - Diffuse : {diffuse_file.name}")
            print(f" - Normal  : {normal_file.name}")

            # =========================================================
            # 5. EXECUTION DES SCRIPTS
            # =========================================================

            print("⏳ Entraînement en cours...")
            run_training(str(diffuse_file), str(normal_file), str(roughness_file), str(model_output_path))

            print("⏳ Exportation vers Godot en cours...")
            run_export(str(model_output_path), str(export_dir))

if __name__ == "__main__":
    main()
