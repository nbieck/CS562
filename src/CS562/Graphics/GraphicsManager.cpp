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
#include "../Transformation/Transformation.h"

#include "../ImGui/imgui.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <list>
#include <memory>

namespace CS562
{
	namespace
	{
		struct QuadVert { glm::vec3 pos; glm::vec2 uv; };

		const QuadVert quad_verts[] =
		{ {{1, 1, 0}, {1,1}}, {{-1, 1, 0},{0,1}}, {{-1, -1, 0},{0,0}},
		{{1, 1, 0},{1,1}}, {{-1, -1, 0}, {0,0}}, {{1, -1, 0}, {1, 0} }
	};

		const int shadow_map_size = 1024;

		const glm::mat4 bias_mat
			(.5f, 0, 0, 0,
				0, .5f, 0, 0,
				0, 0, .5f, 0,
				0.5f, 0.5f, 0.5f, 1);
	}

	struct GraphicsManager::PImpl
	{
		GraphicsManager& owner;
		WindowManager& window;
		Drawmode mode;

		int width;
		int height;

		bool show_shadow_map;

		HDC device_context;
		HGLRC ogl_context;

		std::unique_ptr<GBuffer> g_buffer;

		std::unique_ptr<VertexArray> FSQ;
		std::shared_ptr<ShaderProgram> buffer_copy;
		std::shared_ptr<Buffer<QuadVert>> quad_buffer;

		std::shared_ptr<Geometry> sphere;
		std::shared_ptr<ShaderProgram> light_sphere_shader;

		std::shared_ptr<ShaderProgram> ambient_shader;
		std::shared_ptr<ShaderProgram> light_shader;
		std::shared_ptr<ShaderProgram> shadow_light_shader;
		std::shared_ptr<ShaderProgram> shadow_map_shader;
		std::shared_ptr<ShaderProgram> show_shadowmap_shader;

		std::unique_ptr<Framebuffer> shadow_buffer;
		std::shared_ptr<Texture> shadow_map;
		std::shared_ptr<Texture> shadow_map_depth;
		std::shared_ptr<Texture> filtered_shadow_map;

		float exp_c;

		struct LightSphereData
		{
			glm::mat4 MVP;
			glm::vec3 color;
			float spacing;
		};
		std::unique_ptr<Buffer<LightSphereData>> light_sphere_MVP_buffer;

		struct LightData
		{
			glm::mat4 model_mat;
			glm::vec3 position;
			float intensity;
			glm::vec3 color;
			float max_distance;
		};
		std::unique_ptr<Buffer<LightData>> light_data_buffer;

		std::list<std::weak_ptr<Drawable>> drawables;
		std::list<std::weak_ptr<Light>> lights;
		std::list<std::weak_ptr<Light>> shadowing_lights;

		PImpl(int width, int height, GraphicsManager& owner, WindowManager& window)
			: width(width), height(height), owner(owner), window(window), mode(Drawmode::Solid), show_shadow_map(true), exp_c(80.f)
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
			quad_buffer = std::make_shared<Buffer<QuadVert>>();
			{
				auto unbind = quad_buffer->Bind(BufferTargets::Vertex);
				quad_buffer->SetUpStorage(sizeof(quad_verts) / sizeof(QuadVert), BufferCreateFlags::None, quad_verts);
			}

			FSQ = std::make_unique<VertexArray>();
			auto unbind = FSQ->Bind();
			unsigned buffer_idx = FSQ->AddDataBuffer(quad_buffer, sizeof(QuadVert));
			FSQ->SetAttributeAssociation(0, buffer_idx, 3, DataTypes::Float, 0);
			FSQ->SetAttributeAssociation(1, buffer_idx, 2, DataTypes::Float, sizeof(glm::vec3));
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
			ambient_shader->SetUniform("AmbientLight", glm::vec3(0.2f));

			light_shader = ResourceLoader::LoadShaderProgramFromFile("shaders/deferred_light.shader");

			light_shader->SetUniform("Position", 1);
			light_shader->SetUniform("Normal", 2);
			light_shader->SetUniform("Diffuse", 3);
			light_shader->SetUniform("Specular", 4);
			light_shader->SetUniform("Shininess", 5);

			light_sphere_shader = ResourceLoader::LoadShaderProgramFromFile("shaders/light_marker.shader");
			
			shadow_map_shader = ResourceLoader::LoadShaderProgramFromFile("shaders/shadow_pass.shader");

			shadow_light_shader = ResourceLoader::LoadShaderProgramFromFile("shaders/shadow_light.shader");

			shadow_light_shader->SetUniform("Position", 1);
			shadow_light_shader->SetUniform("Normal", 2);
			shadow_light_shader->SetUniform("Diffuse", 3);
			shadow_light_shader->SetUniform("Specular", 4);
			shadow_light_shader->SetUniform("Shininess", 5);
			shadow_light_shader->SetUniform("shadow_map", 6);

			show_shadowmap_shader = ResourceLoader::LoadShaderProgramFromFile("shaders/render_shadowmap.shader");

			show_shadowmap_shader->SetUniform("shadow_map", 1);

			std::vector<std::pair<std::shared_ptr<Geometry>, unsigned>> geom;
			std::vector<std::shared_ptr<Material>> mats;
			ResourceLoader::LoadObjFile(geom, mats, "meshes/sphere.obj");

			sphere = geom[0].first;
		}

		void SetupBuffers()
		{
			light_sphere_MVP_buffer = std::make_unique<Buffer<LightSphereData>>();
			{
				auto unbind = light_sphere_MVP_buffer->Bind(BufferTargets::Vertex);
				light_sphere_MVP_buffer->ResizeableStorage(1000);
			}
			light_data_buffer = std::make_unique<Buffer<LightData>>();
			{
				auto unbind = light_data_buffer->Bind(BufferTargets::Vertex);
				light_data_buffer->ResizeableStorage(1000);
			}
		}
		
		void InitShadowMap()
		{
			shadow_buffer = std::make_unique<Framebuffer>();
			shadow_map = std::make_shared<Texture>();
			filtered_shadow_map = std::make_shared<Texture>();
			shadow_map_depth = std::make_shared<Texture>();
			{
				auto unbind = shadow_map->Bind(0);
				shadow_map->AllocateSpace(shadow_map_size, shadow_map_size, TextureFormatInternal::R32F, 1);
			}
			{
				auto unbind = shadow_map_depth->Bind(0);
				shadow_map_depth->AllocateSpace(shadow_map_size, shadow_map_size, TextureFormatInternal::DEPTH24, 1);
			}
			{
				auto unbind = shadow_buffer->Bind();
				shadow_buffer->AttachTexture(Attachments::Color0, shadow_map);
				shadow_buffer->AttachTexture(Attachments::Depth, shadow_map_depth);
			}
			{
				auto unbind = filtered_shadow_map->Bind(0);
				filtered_shadow_map->AllocateSpace(shadow_map_size, shadow_map_size, TextureFormatInternal::R32F, 1);
			}
		}

		void GeometryPass()
		{
			gl::Enable(gl::DEPTH_TEST);

			auto unbind = g_buffer->g_buff->Bind();
			gl::ClearColor(0.f, 0.f, 0.f, 0.f);
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

				unsigned idx = 0;
				{
					auto unbind_buff = light_sphere_MVP_buffer->Bind(BufferTargets::Vertex);
					if (light_sphere_MVP_buffer->GetSize() < lights.size() + shadowing_lights.size())
					{
						light_sphere_MVP_buffer->ResizeableStorage(static_cast<unsigned>(lights.size() + shadowing_lights.size()));
					}

					auto * data = light_sphere_MVP_buffer->Map(MapModes::Write);

					for (auto it = lights.begin(); it != lights.end();)
					{
						if (it->expired())
						{
							it = lights.erase(it);
							continue;
						}

						auto light = it->lock();
						data[idx].MVP = light->owner_world_trans_.GetMatrix();
						data[idx].color = light->color;

						idx++;
						it++;
					}
					for (auto it = shadowing_lights.begin(); it != shadowing_lights.end();)
					{
						if (it->expired())
						{
							it = shadowing_lights.erase(it);
							continue;
						}

						auto light = it->lock();
						data[idx].MVP = light->owner_world_trans_.GetMatrix();
						data[idx].color = light->color;

						idx++;
						it++;
					}

					light_sphere_MVP_buffer->Unmap();
				}

				auto unbind_buff = light_sphere_MVP_buffer->Bind(BufferTargets::ShaderStorage, 0);
				light_sphere_shader->SetUniform("ViewProj", proj * view);
				auto unbind_shader = light_sphere_shader->Bind();
				sphere->DrawInstanced(idx);
			}
		}

		void NonShadowLights(const glm::mat4& view, const glm::mat4& proj)
		{
			gl::Enable(gl::DEPTH_TEST);
			gl::CullFace(gl::FRONT);
			gl::DepthFunc(gl::GREATER);
			gl::DepthMask(gl::FALSE_);
			{
				auto unbind_shader = light_shader->Bind();

				{
					auto unbind_buff = light_data_buffer->Bind(BufferTargets::ShaderStorage);
					if (light_data_buffer->GetSize() < lights.size())
						light_data_buffer->ResizeableStorage(static_cast<unsigned>(lights.size()));

					auto data = light_data_buffer->Map(MapModes::Write);
					unsigned idx = 0;
					for (auto l : lights)
					{
						auto light = l.lock();

						Transformation l_t = light->owner_world_trans_;
						l_t.scale = glm::vec3(light->max_distance);

						data[idx].model_mat = l_t.GetMatrix();
						data[idx].color = light->color;
						data[idx].position = light->owner_world_trans_.position;
						data[idx].intensity = light->intensity;
						data[idx].max_distance = light->max_distance;

						idx++;
					}

					light_data_buffer->Unmap();
				}

				light_shader->SetUniform("ViewProj", proj * view);
				auto unbind_buff = light_data_buffer->Bind(BufferTargets::ShaderStorage, 0);
				sphere->DrawInstanced(static_cast<unsigned>(lights.size()));
			}
			gl::DepthMask(gl::TRUE_);
			gl::DepthFunc(gl::LESS);
			gl::CullFace(gl::BACK);

		}

		void ShadowLights(const glm::mat4& view, const glm::mat4& proj)
		{
			shadow_light_shader->SetUniform("ViewProj", proj * view);

			//for each light:
			for (auto l : shadowing_lights)
			{
				if (l.expired())
					continue;

				auto light = l.lock();

				Camera l_cam(light->owner_world_trans_, 0.1f, light->max_distance, 2.f * glm::acos(light->outer_cos), 1.f);
				glm::mat4 light_view =  l_cam.GetViewMatrix();
				glm::mat4 light_proj = l_cam.GetProjectionMatrix();
				//render shadow map

				{
					auto unbind = shadow_buffer->Bind();
					gl::Viewport(0, 0, shadow_map_size, shadow_map_size);
					gl::ClearColor(1.f, 1.f, 1.f, 1.f);
					gl::Clear(gl::DEPTH_BUFFER_BIT | gl::COLOR_BUFFER_BIT);
					gl::Disable(gl::BLEND);
					auto shader_unbind = shadow_map_shader->Bind();
					shadow_map_shader->SetUniform("VP", light_proj * light_view);
					shadow_map_shader->SetUniform("near", 0.1f);
					shadow_map_shader->SetUniform("far", light->max_distance);
					shadow_map_shader->SetUniform("light_pos", light->owner_world_trans_.position);
					shadow_map_shader->SetUniform("offset", 0.1f);
					shadow_map_shader->SetUniform("c", exp_c);

					for (auto renderable : drawables)
					{
						if (renderable.expired())
							continue;

						auto render = renderable.lock();
						
						shadow_map_shader->SetUniform("M", render->owner_world_trans_.GetMatrix());

						render->geometry->Draw();
					}
				}

				//blur shadow map

				gl::Viewport(0, 0, width, height);
				gl::Enable(gl::BLEND);			
				gl::DepthFunc(gl::GREATER);
				gl::DepthMask(gl::FALSE_);
				gl::CullFace(gl::FRONT);

				shadow_light_shader->SetUniform("shadow_near", 0.1f);
				shadow_light_shader->SetUniform("shadow_far", light->max_distance);
				shadow_light_shader->SetUniform("c", exp_c);
				{
					//render light
					auto unbind = g_buffer->g_buff->Bind();
					auto unbind_shadow_map = shadow_map->Bind(6);
					auto unbind_shader = shadow_light_shader->Bind();

					light->SetUniforms(shadow_light_shader);
					Transformation l_t = light->owner_world_trans_;
					l_t.scale = glm::vec3(light->max_distance);

					shadow_light_shader->SetUniform("model_mat", l_t.GetMatrix());
					shadow_light_shader->SetUniform("shadow_mat", bias_mat * light_proj * light_view);
					glm::vec4 light_dir = light->owner_world_trans_.GetMatrix() * glm::vec4(0, 0, 1, 0);
					shadow_light_shader->SetUniform("Light.direction", glm::normalize(glm::vec3(light_dir)));
					
					sphere->Draw();
				}
				gl::DepthMask(gl::TRUE_);
				gl::DepthFunc(gl::LESS);
				gl::CullFace(gl::BACK);

			}
		}

		void LightingPass()
		{
			gl::Disable(gl::DEPTH_TEST);
			light_shader->SetUniform("CamPos", owner.current_cam->owner_world_trans_.position);
			shadow_light_shader->SetUniform("CamPos", owner.current_cam->owner_world_trans_.position);

			glm::mat4 view, proj;
			view = owner.current_cam->GetViewMatrix();
			proj = owner.current_cam->GetProjectionMatrix();
			{
				auto unbind_buffer = g_buffer->g_buff->Bind();
				g_buffer->g_buff->EnableAttachments({ Buffers::LightAccumulation});

				gl::BlendFunc(gl::ONE, gl::ONE);
				gl::Enable(gl::BLEND);

				g_buffer->BindTextures(1, false);
				auto unbind_vao = FSQ->Bind();

				{
					auto unbind_shader = ambient_shader->Bind();
					FSQ->Draw(PrimitiveTypes::Triangles, 6);
				}

				NonShadowLights(view, proj);
			}

			ShadowLights(view, proj);

			auto unbind_buffer = g_buffer->g_buff->Bind();
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

		void ShowShadowMap()
		{
			gl::Disable(gl::DEPTH_TEST);

			const float margin = 0.1f;
			const float dim = 0.5f;

			const glm::mat4 proj = glm::ortho<float>(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), -10, 10);
			const glm::mat4 quad_model = glm::translate(glm::mat4(), glm::vec3(glm::vec2((margin + dim / 2.f) * height), 0.f)) * glm::scale(glm::mat4(), glm::vec3(height * dim / 2.f));

			auto unbind_shader = show_shadowmap_shader->Bind();
			show_shadowmap_shader->SetUniform("MVP", proj * quad_model);
			show_shadowmap_shader->SetUniform("c", exp_c);

			auto unbind_tex = shadow_map->Bind(1);
			auto unbind_vao = FSQ->Bind();

			FSQ->Draw(PrimitiveTypes::Triangles, 6);

			gl::Enable(gl::DEPTH_TEST);
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
		impl->SetupBuffers();
		impl->InitShadowMap();
	}

	void GraphicsManager::Update()
	{
		//clear the default frame buffer
		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
		impl->GeometryPass();

		impl->LightingPass();

		impl->CopyBufferPass();

		if (impl->show_shadow_map)
			impl->ShowShadowMap();

		ImGui::Render();

		SwapBuffers(impl->device_context);
	}

	void GraphicsManager::RegisterDrawable(const std::weak_ptr<Drawable>& drawable)
	{
		impl->drawables.push_back(drawable);
	}

	void GraphicsManager::RegisterLight(const std::weak_ptr<Light>& light)
	{
		if (light.lock()->cast_shadow)
			impl->shadowing_lights.push_back(light);
		else
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

	void GraphicsManager::SetShowShadowMap(bool show)
	{
		impl->show_shadow_map = show;
	}

	void GraphicsManager::SetShadowC(float c)
	{
		impl->exp_c = c;
	}
}
