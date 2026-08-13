extends MeshInstance3D

func _ready():
	var mat = get_surface_override_material(0)
	
	if mat and mat.has_method("load_weights_from_json"):
		var json_path = "res://neural_data/mlp_weights.json"
		
		print("Chargement des poids du MLP...")
		mat.load_weights_from_json(json_path)
		print("Chargement terminé sans crash !")
		
		# Optionnel : charger les textures via GDScript pour tester
		# mat.set_feature_texture(0, load("res://neural_data/neural_feature_0.png"))
