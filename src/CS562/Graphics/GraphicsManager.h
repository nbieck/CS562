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
			Shininess,
			AO_NonBlur,
			AO_HorizontalBlur,
			AO_Final
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
		void SetHiZMip(int mip_level);

		void SetShowShadowMap(bool show);

		void SetShadowC(float c);

		void SetShadowBlurWidth(int w);
		
		//filename without extension.
		//assumes existence of a .hdr and .irr.hdr file with that name
		void SetSkybox(const std::string& filename);

		void SetExposure(float e);
		void SetContrast(float c);

		void SetNumSamples(int n);

		void SetAORadius(float r);
		void SetAODelta(float d);
		void SetAOSamples(int n);
		void SetAOScale(float s);
		void SetAOContrast(float k);

		static const int max_blur_width = 50;

		std::shared_ptr<Camera> current_cam;

	private:

		struct PImpl;
		std::unique_ptr<PImpl> impl;
	};
}

#endif
