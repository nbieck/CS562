////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_GRAPHICS_MANAGER_H_
#define CS350_GRAPHICS_MANAGER_H_

#include "../Window/WindowManager.h"

#include "../Drawable/Drawable.h"
#include "../Camera/Camera.h"
#include "../Light/Light.h"
#include "../Bounding Volumes/BoundingVolumeManager.h"

#include <memory>

namespace CS350
{
	enum class Drawmode
	{
		Solid,
		Wireframe
	};

	class GraphicsManager
	{
	public:

		GraphicsManager(int width, int height, WindowManager& window, BoundingVolumeManager& bvs);
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
