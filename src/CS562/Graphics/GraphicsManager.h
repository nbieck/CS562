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

	namespace DrawBuffers
	{
		enum DrawBuffers
		{
			LightAccum = 0,
			Position,
			Normal,
			Diffuse,
			Specular,
			Shininess
		};
	}

	class GraphicsManager
	{
	public:

		GraphicsManager(int width, int height, WindowManager& window);
		~GraphicsManager();

		void Update();

		void RegisterDrawable(const std::weak_ptr<Drawable>& drawable);

		void RegisterLight(const std::weak_ptr<Light>& light);

		void SetDrawmode(Drawmode mode);

		void SetShownBuffer(int buffer);

		void SetShowShadowMap(bool show);

		void SetShadowC(float c);

		void SetShadowBlurWidth(int w);

		static const int max_blur_width = 50;

		std::shared_ptr<Camera> current_cam;

	private:

		struct PImpl;
		std::unique_ptr<PImpl> impl;
	};
}

#endif
