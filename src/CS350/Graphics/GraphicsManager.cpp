////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "GraphicsManager.h"

#include "../OpenGL/gl_core_4_4.hpp"
#include "../OpenGL/wgl_common_exts.hpp"
#include "../GLWrapper/Buffer.h"
#include "../GLWrapper/VertexArray.h"
#include "../GLWrapper/ShaderProgram.h"
#include "../ResourceLoader/ResourceLoader.h"

#include "../ImGui/imgui.h"

#include <list>

namespace CS350
{
	namespace
	{
		struct Plane
		{
			glm::vec3 position;
			glm::vec3 normal;
		};
	}

	struct GraphicsManager::PImpl
	{
		GraphicsManager& owner;
		WindowManager& window;
		BoundingVolumeManager& bvs;
		Drawmode mode;

		int width;
		int height;

		HDC device_context;
		HGLRC ogl_context;

		std::list<std::weak_ptr<Drawable>> drawables;

		std::vector<Plane> planes_to_draw;
		std::shared_ptr<Buffer<Plane>> plane_buff;
		std::shared_ptr<VertexArray> plane_vao;
		std::shared_ptr<ShaderProgram> plane_shader;

		PImpl(int width, int height, GraphicsManager& owner, WindowManager& window, BoundingVolumeManager& bvs)
			: width(width), height(height), owner(owner), window(window), bvs(bvs), mode(Drawmode::Solid)
		{

		}

		~PImpl()
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(ogl_context);
		}

		void CreateContext()
		{
			//device context for the temporary window we need
			HDC temp_device_context = GetDC(window.GetWindowHandle());

			PIXELFORMATDESCRIPTOR pfd = { 0 };
			pfd.nSize			= sizeof(PIXELFORMATDESCRIPTOR);
			pfd.nVersion		= 1;
			pfd.dwFlags			= PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
			pfd.iPixelType		= PFD_TYPE_RGBA;
			pfd.cColorBits		= 32;
			pfd.cDepthBits		= 24;
			pfd.cStencilBits	= 8;

			int nPixelFormat = ChoosePixelFormat(temp_device_context, &pfd);
			if (!SetPixelFormat(temp_device_context, nPixelFormat, &pfd))
			{
				DWORD error = GetLastError();
				MessageBox(NULL, "Error on SetPixelFormat for fake context.", "Fatal Error!", MB_ICONERROR | MB_OK);
				throw "SetPixelFormat error";
			}
			
			//create temp context
			HGLRC temp_ogl_context = wglCreateContext(temp_device_context);
			wglMakeCurrent(temp_device_context, temp_ogl_context);

			wgl::exts::LoadTest didLoadWGL = wgl::sys::LoadFunctions(temp_device_context);
			if (!didLoadWGL)
			{
				//clean up context
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(temp_ogl_context);

				MessageBox(NULL, "Error loading WGL functions", "Fatal Error", MB_ICONERROR | MB_OK);
				throw "Error loading WGL";
			}
			
			if (wgl::exts::var_ARB_pixel_format && wgl::exts::var_ARB_pixel_format_float 
				&& wgl::exts::var_ARB_create_context && wgl::exts::var_ARB_create_context_profile)
			{
				HWND temp_window = window.GetWindowHandle();
				window.MakeNewWindow();
				device_context = GetDC(window.GetWindowHandle());

				int pixel_fmt[] = 
				{
					wgl::DRAW_TO_WINDOW_ARB,			TRUE,
					wgl::ACCELERATION_ARB,				wgl::FULL_ACCELERATION_ARB,
					wgl::DOUBLE_BUFFER_ARB,				TRUE,
					wgl::PIXEL_TYPE_ARB,				wgl::TYPE_RGBA_ARB,
					wgl::RED_BITS_ARB,					8,
					wgl::GREEN_BITS_ARB,				8,
					wgl::RED_BITS_ARB,					8,
					wgl::DEPTH_BITS_ARB,				24,
					wgl::STENCIL_BITS_ARB,				8,
					wgl::FRAMEBUFFER_SRGB_CAPABLE_ARB,	TRUE,
					0
				};

				int context_attribs[] =
				{
					wgl::CONTEXT_MAJOR_VERSION_ARB, 4,
					wgl::CONTEXT_MINOR_VERSION_ARB, 4,
					wgl::CONTEXT_FLAGS_ARB, wgl::CONTEXT_CORE_PROFILE_BIT_ARB,
					0
				};

				int fmt;
				unsigned num_fmt;
				if (!wgl::ChoosePixelFormatARB(device_context, pixel_fmt, nullptr, 1, &fmt, &num_fmt))
				{
					wglMakeCurrent(NULL, NULL);
					wglDeleteContext(temp_ogl_context);
					DestroyWindow(temp_window);
					MessageBox(NULL, "Error choosing an appropriate pixel format.", "Fatal Error", MB_OK | MB_ICONERROR);
					throw "Could not choose pixel format.";
				}

				SetPixelFormat(device_context, fmt, NULL);
				ogl_context = wgl::CreateContextAttribsARB(device_context, 0, context_attribs);
				//temporary to avoid leaks
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(temp_ogl_context);
				DestroyWindow(temp_window);
				wglMakeCurrent(device_context, ogl_context);
				
				gl::exts::LoadTest didLoadGL = gl::sys::LoadFunctions();
				if (!didLoadGL)
				{
					//clean up context
					wglMakeCurrent(NULL, NULL);
					wglDeleteContext(temp_ogl_context);

					MessageBox(NULL, "Error loading GL functions", "Fatal Error", MB_ICONERROR | MB_OK);
					throw "Error loading GL";
				}

				if (!gl::sys::IsVersionGEQ(4, 4))
				{
					wglMakeCurrent(NULL, NULL);
					wglDeleteContext(ogl_context);
					MessageBox(NULL, "Your system does not currently support OpenGL 4.4. Try updating your drivers.", "Fatal Error", MB_OK | MB_ICONERROR);
					throw "Wrong GL Version";
				}
			}
			else
			{
				//if we cannot further customize the context we actually want, treat it as failure
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(temp_ogl_context);
				MessageBox(NULL, "Required WGL core extensions not supported.", "Fatal Error", MB_ICONERROR | MB_OK);
				throw "WGL extensions not supported.";
			}
		}

		void InitGL()
		{
			gl::Viewport(0, 0, width, height);
			gl::ClearColor(0.2f, 0.2f, 0.2f, 1.f);
			gl::Enable(gl::DEPTH_TEST);
			gl::DepthFunc(gl::LESS);
			gl::Enable(gl::CULL_FACE);

			gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);

			if (wgl::exts::var_EXT_swap_control)
			{
				wgl::SwapIntervalEXT(0);
			}

			plane_shader = ResourceLoader::LoadShaderProgramFromFile("shaders/plane.shader");
			plane_buff = std::make_shared<Buffer<Plane>>();
			plane_vao = std::make_shared<VertexArray>();

			{
				auto unbind = plane_buff->Bind(BufferTargets::Vertex);

				plane_buff->ResizeableStorage(1);
			}
			{
				auto unbind = plane_vao->Bind();

				unsigned binding = plane_vao->AddDataBuffer(plane_buff, sizeof(glm::vec3));
				plane_vao->SetAttributeAssociation(0, binding, 3, DataTypes::Float, 0);
			}
		}

		void DrawPlanes(const glm::mat4& view, const glm::mat4& proj)
		{
			if (!planes_to_draw.empty())
			{
				{
					auto unbind_buff = plane_buff->Bind(BufferTargets::Vertex);

					if (plane_buff->GetSize() < planes_to_draw.size())
					{
						plane_buff->ResizeableStorage(static_cast<unsigned>(planes_to_draw.size()), planes_to_draw.data());
					}
					else
					{
						plane_buff->SendData(0, static_cast<unsigned>(planes_to_draw.size()), planes_to_draw.data());
					}
				}

				gl::DepthMask(gl::FALSE_);
				gl::Enable(gl::BLEND);
				gl::Disable(gl::CULL_FACE);

				plane_shader->SetUniform("ViewProj", proj * view);

				auto unbind_shader = plane_shader->Bind();
				auto unbind_vao = plane_vao->Bind();

				plane_vao->Draw(PrimitiveTypes::Lines, static_cast<unsigned>(planes_to_draw.size() * 2));

				gl::Enable(gl::CULL_FACE);
				gl::Disable(gl::BLEND);
				gl::DepthMask(gl::TRUE_);

				planes_to_draw.clear();
			}
		}
	};

	GraphicsManager::~GraphicsManager() = default;

	GraphicsManager::GraphicsManager(int width, int height, WindowManager& window, BoundingVolumeManager& bvs)
		: impl(std::make_unique<PImpl>(width, height, *this, window, bvs))
	{
		impl->CreateContext();
		impl->InitGL();
	}

	void GraphicsManager::Update()
	{
		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

		if (current_cam)
		{
			glm::mat4 view, proj;
			view = current_cam->GetViewMatrix();
			proj = current_cam->GetProjectionMatrix();

			if (impl->mode == Drawmode::Wireframe)
			{
				gl::Disable(gl::CULL_FACE);
				gl::PolygonMode(gl::FRONT_AND_BACK, gl::LINE);
			}

			for (auto it = impl->drawables.begin(); it != impl->drawables.end();)
			{
				if (it->expired())
				{
					it = impl->drawables.erase(it);
				}
				else
				{
					auto drawable = it->lock();

					if (curr_light)
						curr_light->SetUniforms(drawable->shader, view);
					drawable->Draw(view, proj);
					++it;
				}
			}

			if (impl->mode == Drawmode::Wireframe)
			{
				gl::Enable(gl::CULL_FACE);
				gl::PolygonMode(gl::FRONT_AND_BACK, gl::FILL);
			}

			impl->bvs.Draw(view, proj, current_cam->GetViewFrustum());

			impl->DrawPlanes(view, proj);
		}

		ImGui::Render();

		SwapBuffers(impl->device_context);
	}

	void GraphicsManager::RegisterDrawable(const std::weak_ptr<Drawable>& drawable)
	{
		impl->drawables.push_back(drawable);
	}

	void GraphicsManager::SetDrawmode(Drawmode mode)
	{
		impl->mode = mode;
	}

	void GraphicsManager::DrawPlane(glm::vec3 position, glm::vec3 normal)
	{
		impl->planes_to_draw.push_back({ position, normal });
	}
}
