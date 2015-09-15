////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_GRAPHICS_MANAGER_H_
#define CS350_GRAPHICS_MANAGER_H_

#include "../Window/WindowManager.h"

#include "../CompoundObjects/Drawable.h"
#include "../Camera/Camera.h"
#include "../CompoundObjects/Light.h"

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

		std::shared_ptr<Camera> current_cam;
		std::shared_ptr<Light> curr_light;

	private:

		struct PImpl;
		std::unique_ptr<PImpl> impl;
	};
}

#endif
