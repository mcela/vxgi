namespace
{
	const float FLY_CAMERA_MOVE_SPEED = 3.0f;
	const float FLY_CAMERA_LERP_SPEED = 10.0f;
	const float FLY_CAMERA_MOUSE_SENSITIVITY = 0.0025f;
}

namespace camera
{
	void update(Camera& c)
	{
		c.view = glm::lookAt(c.position, c.position + c.direction, c.up);
		c.VP = c.projection * c.view;
	}

	void set_to_perspective(Camera& c, float aspect_ratio, float fov, float near_plane, float far_plane) {
		c.projection = glm::perspective(DEGREES_TO_RADIANS * fov, aspect_ratio, near_plane, far_plane);
	}

	void set_to_ortho(Camera& c) {
		c.projection = glm::ortho(-1, 1, -1, 1, -1, 1);
	}

	vec3 get_right(Camera& c) {
		return glm::normalize(-glm::cross(c.up, c.direction)); 
	}
	vec3 get_forward(Camera& c) {
		return glm::normalize(c.direction);
	}
}

namespace flycamera
{
	void attach(Fly_Camera_Controls& controls, Camera* camera) {
		controls.camera = camera;
	}

	void update(Fly_Camera_Controls& controls, GLFWwindow* window, float dt) {
		if (!controls.camera)
			return;

		if (controls.is_first_update) {
			controls.lerp_state.direction = controls.camera->direction;
			controls.lerp_state.position = controls.camera->position;
			controls.is_first_update = false;
		}

		int window_w, window_h;
		glfwGetWindowSize(window, &window_w, &window_h);
		float window_half_w = float(window_w) * 0.5f;
		float window_half_h = float(window_h) * 0.5f;

		double cursor_x, cursor_y;
		glfwGetCursorPos(window, &cursor_x, &cursor_y);
		glfwSetCursorPos(window, window_half_w, window_half_h);
		double cursor_delta_x = cursor_x - window_half_w;
		double cursor_delta_y = cursor_y - window_half_h;

		controls.yaw   += cursor_delta_x * FLY_CAMERA_MOUSE_SENSITIVITY;
		controls.pitch += cursor_delta_y * FLY_CAMERA_MOUSE_SENSITIVITY * -1.0f;
		controls.pitch = glm::clamp(controls.pitch, -PI_HALF, +PI_HALF);

		controls.lerp_state.direction.x = cosf(controls.yaw) * cosf(controls.pitch);
		controls.lerp_state.direction.y = sinf(controls.pitch);
		controls.lerp_state.direction.z = sinf(controls.yaw) * cosf(controls.pitch);
		controls.lerp_state.direction = glm::normalize(controls.lerp_state.direction);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) controls.lerp_state.position += camera::get_forward(controls.lerp_state) * FLY_CAMERA_MOVE_SPEED * dt;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) controls.lerp_state.position -= camera::get_forward(controls.lerp_state) * FLY_CAMERA_MOVE_SPEED * dt;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) controls.lerp_state.position += camera::get_right(controls.lerp_state)   * FLY_CAMERA_MOVE_SPEED * dt;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) controls.lerp_state.position -= camera::get_right(controls.lerp_state)   * FLY_CAMERA_MOVE_SPEED * dt;

		controls.camera->direction = mix(controls.camera->direction, controls.lerp_state.direction, glm::clamp(FLY_CAMERA_LERP_SPEED * dt, 0.0f, 1.0f));
		controls.camera->position = mix(controls.camera->position, controls.lerp_state.position, glm::clamp(FLY_CAMERA_LERP_SPEED * dt, 0.0f, 1.0f));
		camera::update(*controls.camera);
	}
}
