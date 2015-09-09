////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_GRAPHICS_MANAGER_H_
#define CS350_GRAPHICS_MANAGER_H_

#include "../Window/WindowManager.h"

#include "../Drawable/Drawable.h"
#include "../Camera/Camera.h"
#include "../Light/Light.h"

#include <memory>

namespace CS562
{
	enum class Drawmode
	{
		Solid,
		Wireframe
	};

	class GraphicsManager
	{
	public:

		GraphicsManager(int width, int height, WindowManager& window);
		~GraphicsManager();

		void Update();

		void RegisterDrawable(const std::weak_ptr<Drawable>& drawable);

		void SetDrawmode(Drawmode mode);

		void DrawPlane(glm::vec3 position, glm::vec3 normal);

		std::shared_ptr<Camera> current_cam;
		std::shared_ptr<Light> curr_light;

	private:

		struct PImpl;
		std::unique_ptr<PImpl> impl;
	};
}

#endif
