////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "GraphicsManager.h"

#include "../OpenGL/gl_core_4_4.hpp"
#include "../OpenGL/wgl_common_exts.hpp"
#include "../GLWrapper/Buffer.h"
#include "../GLWrapper/VertexArray.h"
#include "../GLWrapper/ShaderProgram.h"
#include "../ResourceLoader/ResourceLoader.h"
#include "../CompoundObjects/GBuffer.h"

#include "../ImGui/imgui.h"

#include <glm/glm.hpp>

#include <list>
#include <memory>

namespace CS562
{
	namespace
	{
		const glm::vec3 quad_verts[] =
		{ {1, 1, 0}, {-1, 1, 0}, {-1, -1, 0}, 
		  {1, 1, 0}, {-1, -1, 0}, {1, -1, 0} };
	}

	struct GraphicsManager::PImpl
	{
		GraphicsManager& owner;
		WindowManager& window;
		Drawmode mode;

		int width;
		int height;

		HDC device_context;
		HGLRC ogl_context;

		std::unique_ptr<GBuffer> g_buffer;

		std::unique_ptr<VertexArray> FSQ;
		std::shared_ptr<ShaderProgram> buffer_copy;
		std::shared_ptr<Buffer<glm::vec3>> quad_buffer;

		std::shared_ptr<Geometry> sphere;
		std::shared_ptr<ShaderProgram> light_sphere_shader;

		std::shared_ptr<ShaderProgram> ambient_shader;
		std::shared_ptr<ShaderProgram> light_shader;

		std::list<std::weak_ptr<Drawable>> drawables;
		std::list<std::weak_ptr<Light>> lights;

		PImpl(int width, int height, GraphicsManager& owner, WindowManager& window)
			: width(width), height(height), owner(owner), window(window), mode(Drawmode::Solid)
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
			gl::ClearColor(0.0f, 0.0f, 0.0f, 1.f);
			gl::Enable(gl::DEPTH_TEST);
			gl::DepthFunc(gl::LESS);
			gl::Enable(gl::CULL_FACE);

			gl::BlendFunc(gl::ONE, gl::ONE);
			gl::BlendEquation(gl::FUNC_ADD);

			if (wgl::exts::var_EXT_swap_control)
			{
				wgl::SwapIntervalEXT(0);
			}
		}

		void SetupFSQ()
		{
			quad_buffer = std::make_shared<Buffer<glm::vec3>>();
			{
				auto unbind = quad_buffer->Bind(BufferTargets::Vertex);
				quad_buffer->SetUpStorage(sizeof(quad_verts) / sizeof(glm::vec3), BufferCreateFlags::None, quad_verts);
			}

			FSQ = std::make_unique<VertexArray>();
			auto unbind = FSQ->Bind();
			unsigned buffer_idx = FSQ->AddDataBuffer(quad_buffer, sizeof(glm::vec3));
			FSQ->SetAttributeAssociation(0, buffer_idx, 3, DataTypes::Float, 0);
		}
		
		void SetupShaders()
		{
			buffer_copy = ResourceLoader::LoadShaderProgramFromFile("shaders/copy_buffer.shader");

			buffer_copy->SetUniform("LightAccumulation", 1);
			buffer_copy->SetUniform("Position", 2);
			buffer_copy->SetUniform("Normal", 3);
			buffer_copy->SetUniform("Diffuse", 4);
			buffer_copy->SetUniform("Specular", 5);
			buffer_copy->SetUniform("Shininess", 6);
			buffer_copy->SetUniform("BufferToShow", 0);

			ambient_shader = ResourceLoader::LoadShaderProgramFromFile("shaders/ambient_light.shader");

			ambient_shader->SetUniform("Diffuse", 3);
			ambient_shader->SetUniform("AmbientLight", glm::vec3(0.1f));

			light_shader = ResourceLoader::LoadShaderProgramFromFile("shaders/deferred_light.shader");

			light_shader->SetUniform("Position", 1);
			light_shader->SetUniform("Normal", 2);
			light_shader->SetUniform("Diffuse", 3);

			light_sphere_shader = ResourceLoader::LoadShaderProgramFromFile("shaders/light_marker.shader");
			
			std::vector<std::pair<std::shared_ptr<Geometry>, unsigned>> geom;
			std::vector<std::shared_ptr<Material>> mats;
			ResourceLoader::LoadObjFile(geom, mats, "meshes/sphere.obj");

			sphere = geom[0].first;
		}

		void GeometryPass()
		{
			gl::Enable(gl::DEPTH_TEST);

			auto unbind = g_buffer->g_buff->Bind();
			gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

			if (owner.current_cam)
			{
				glm::mat4 view, proj;
				view = owner.current_cam->GetViewMatrix();
				proj = owner.current_cam->GetProjectionMatrix();

				if (mode == Drawmode::Wireframe)
				{
					gl::Disable(gl::CULL_FACE);
					gl::PolygonMode(gl::FRONT_AND_BACK, gl::LINE);
				}

				for (auto it = drawables.begin(); it != drawables.end();)
				{
					if (it->expired())
					{
						it = drawables.erase(it);
					}
					else
					{
						auto drawable = it->lock();
						drawable->Draw(view, proj);
						++it;
					}
				}

				if (mode == Drawmode::Wireframe)
				{
					gl::Enable(gl::CULL_FACE);
					gl::PolygonMode(gl::FRONT_AND_BACK, gl::FILL);
				}

				for (auto l : lights)
				{
					if (!l.expired())
					{
						auto light = l.lock();

						auto unbind_shader = light_sphere_shader->Bind();

						light_sphere_shader->SetUniform("MVP", proj * view * light->owner_world_trans_.GetMatrix());
						light_sphere_shader->SetUniform("color", light->color);

						sphere->Draw();
					}
				}
			}
		}

		void LightingPass()
		{
			gl::Disable(gl::DEPTH_TEST);

			auto unbind_buffer = g_buffer->g_buff->Bind();
			g_buffer->g_buff->EnableAttachments({ Buffers::LightAccumulation});
			{
				gl::BlendFunc(gl::ONE, gl::ONE);
				gl::Enable(gl::BLEND);

				g_buffer->BindTextures(1, false);
				auto unbind_vao = FSQ->Bind();

				{
					auto unbind_shader = ambient_shader->Bind();
					FSQ->Draw(PrimitiveTypes::Triangles, 6);
				}

				{
					auto unbind_shader = light_shader->Bind();
					for (auto l = lights.begin(); l != lights.end();)
					{
						if (l->expired())
						{
							l = lights.erase(l);
							continue;
						}

						auto light = l->lock();
						light->SetUniforms(light_shader);
						FSQ->Draw(PrimitiveTypes::Triangles, 6);

						l++;
					}
				}
			}
			g_buffer->g_buff->EnableAttachments({ Buffers::LightAccumulation, Buffers::Position, Buffers::Normal, Buffers::Diffuse,
				Buffers::Specular, Buffers::Alpha});

			gl::Disable(gl::BLEND);
		}

		void CopyBufferPass()
		{
			gl::Disable(gl::DEPTH_TEST);

			g_buffer->BindTextures(1);

			auto unbind_shader = buffer_copy->Bind();
			auto unbind_vao = FSQ->Bind();
			FSQ->Draw(PrimitiveTypes::Triangles, 6);

			g_buffer->UnbindTextures();
		}
	};

	GraphicsManager::~GraphicsManager() = default;

	GraphicsManager::GraphicsManager(int width, int height, WindowManager& window)
		: impl(std::make_unique<PImpl>(width, height, *this, window))
	{
		impl->CreateContext();
		impl->InitGL();
		impl->g_buffer = std::make_unique<GBuffer>(width, height);
		impl->SetupFSQ();
		impl->SetupShaders();
	}

	void GraphicsManager::Update()
	{
		//clear the default frame buffer
		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
		impl->GeometryPass();

		impl->LightingPass();

		impl->CopyBufferPass();

		ImGui::Render();

		SwapBuffers(impl->device_context);
	}

	void GraphicsManager::RegisterDrawable(const std::weak_ptr<Drawable>& drawable)
	{
		impl->drawables.push_back(drawable);
	}

	void GraphicsManager::RegisterLight(const std::weak_ptr<Light>& light)
	{
		impl->lights.push_back(light);
	}

	void GraphicsManager::SetDrawmode(Drawmode mode)
	{
		impl->mode = mode;
	}

	void GraphicsManager::SetShownBuffer(int buffer)
	{
		impl->buffer_copy->SetUniform("BufferToShow", buffer);
	}
}
