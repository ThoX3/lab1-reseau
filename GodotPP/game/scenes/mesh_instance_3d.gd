@tool
extends MeshInstance3D

func _ready():
	var mat = get_surface_override_material(0)
	
	if mat and mat.has_method("load_weights_from_json"):
		# 1. On corrige le chemin vers ton vrai dossier
		var json_path = "res://neuralmaterials/brick/mlp_weights.json"
		
		print("Chargement des poids du MLP dans l'éditeur...")
		mat.load_weights_from_json(json_path)
		print("Chargement terminé ! Le shader a reçu les poids.")
