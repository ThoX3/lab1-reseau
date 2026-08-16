extends MeshInstance3D

func _ready() -> void:
	var file_path = "res://neuralmaterials/brick/square_floor/export/mlp_weights.json"
	var file = FileAccess.open(file_path, FileAccess.READ)
	
	if file:
		var json = JSON.new()
		var error = json.parse(file.get_as_text())
		
		if error == OK:
			var weights_dict = json.data
			var mat = get_surface_override_material(0) as ShaderMaterial
			
			if mat:
				mat.set_shader_parameter("l1_weights", weights_dict["net.0.weight"])
				mat.set_shader_parameter("l1_bias", weights_dict["net.0.bias"])
				mat.set_shader_parameter("l2_weights", weights_dict["net.2.weight"])
				mat.set_shader_parameter("l2_bias", weights_dict["net.2.bias"])
				
				print("[+] Poids du MLP injectés avec succès dans le GPU au Runtime !")
		else:
			push_error("Impossible de parser le JSON des poids.")
	else:
		push_error("Fichier mlp_weights.json introuvable au chemin : " + file_path)
