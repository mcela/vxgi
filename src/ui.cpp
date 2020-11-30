void render_ui()
{
	using namespace ImGui;

	Scene& scene = scenes::get_current();

	Begin("debug");
	{
		renderer::render_ui();

		if (TreeNode("Lights"))
		{
			Light_Manager& lights = scene.light_manager;

			SliderFloat3("ambient light", (float*) &lights.ambient_light, 0.0f, 1.0f);

			if (TreeNode("Directional lights"))
			{
				for (Directional_Light& p : lights.directional_lights)
				{
					PushID(&p);

					SliderFloat("strength", &p.strength, 0.0f, 10.0f);
					if (SliderFloat3("direction", (float*) &p.direction, -1.0f, 1.0f)) p.is_dirty = true;
					SliderFloat3("color", (float*) &p.color, 0.0f, 1.0f);
					SliderFloat3("attenuation", (float*) &p.attenuation, 0.0f, 1.0f);

					if (TreeNode("shadow map")) {
						if (SliderFloat("ortho size", &p.shadow_map.config.ortho,         1.0f, 100.0f)) p.is_dirty = true;
						if (SliderFloat("near plane", &p.shadow_map.config.near_plane, -100.0f, 100.0f)) p.is_dirty = true;
						if (SliderFloat("far plane",  &p.shadow_map.config.far_plane,     0.0f, 100.0f)) p.is_dirty = true;
						TreePop();
					}

					if (p.is_dirty) {
						shadowmap::update(p.shadow_map, p.direction);
					}

					PopID();
				}
				
				TreePop();
			}

			TreePop();
		}
	}
	End();
}