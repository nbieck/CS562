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
		const unsigned compute_group_size = 128;

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

		std::shared_ptr<Geometry> sky_sphere;
		std::shared_ptr<Texture> sky_sphere_img;
		std::shared_ptr<Texture> sky_sphere_irradiance;
		std::shared_ptr<ShaderProgram> sky_sphere_shader;

		std::shared_ptr<ShaderProgram> ambient_shader;
		std::shared_ptr<ShaderProgram> light_shader;
		std::shared_ptr<ShaderProgram> shadow_light_shader;
		std::shared_ptr<ShaderProgram> shadow_map_shader;
		std::shared_ptr<ShaderProgram> show_shadowmap_shader;

		std::shared_ptr<ShaderProgram> horizontal_blur;
		std::shared_ptr<ShaderProgram> vertical_blur;

		std::unique_ptr<Framebuffer> shadow_buffer;
		std::shared_ptr<Texture> shadow_map;
		std::shared_ptr<Texture> shadow_map_depth;
		std::shared_ptr<Texture> filtered_shadow_map;

		std::shared_ptr<Framebuffer> ao_fb;
		std::shared_ptr<Texture> base_ao_map;
		std::shared_ptr<ShaderProgram> ao_comp;
		std::shared_ptr<Texture> ao_horiz;
		std::shared_ptr<ShaderProgram> bilateral_horiz;
		std::shared_ptr<Texture> ao_final;
		std::shared_ptr<ShaderProgram> bilateral_vert;

		std::shared_ptr<Texture> hi_z_buffer;
		std::shared_ptr<ShaderProgram> hi_z_base_level;
		std::shared_ptr<ShaderProgram> hi_z_comp_mip;

		float exp_c;

		float exposure;
		float contrast;

		int num_samples;
		std::unique_ptr<Buffer<glm::vec2>> random_numbers;

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

		int shadow_blur_width;
		std::unique_ptr<Buffer<float>> blur_weights;

		bool do_ssr;

		PImpl(int width, int height, GraphicsManager& owner, WindowManager& window)
			: width(width), height(height), owner(owner), window(window), mode(Drawmode::Solid), show_shadow_map(true), exp_c(80.f), shadow_blur_width(5), exposure(1.f), contrast(1.f), do_ssr(true)
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
			buffer_copy->SetUniform("AO_NonBlur", 7);
			buffer_copy->SetUniform("AO_HorizontalBlur", 8);
			buffer_copy->SetUniform("AO_Final", 9);
			buffer_copy->SetUniform("BufferToShow", 0);

			ambient_shader = ResourceLoader::LoadShaderProgramFromFile("shaders/ambient_light.shader");

			ambient_shader->SetUniform("Position", 1);
			ambient_shader->SetUniform("Normal", 2);
			ambient_shader->SetUniform("Diffuse", 3);
			ambient_shader->SetUniform("Specular", 4);
			ambient_shader->SetUniform("Shininess", 5);
			ambient_shader->SetUniform("Irradiance", 6);
			ambient_shader->SetUniform("Skysphere", 7);
			ambient_shader->SetUniform("AO", 8);

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

			horizontal_blur = ResourceLoader::LoadShaderProgramFromFile("shaders/blur_horizontal.shader");

			vertical_blur = ResourceLoader::LoadShaderProgramFromFile("shaders/blur_vertical.shader");

			sky_sphere_shader = ResourceLoader::LoadShaderProgramFromFile("shaders/sky_sphere.shader");

			sky_sphere_shader->SetUniform("sky_tex", 1);

			ao_comp = ResourceLoader::LoadShaderProgramFromFile("shaders/ambient_occlusion.shader");
			ao_comp->SetUniform("PositionBuffer", 1);
			ao_comp->SetUniform("NormalBuffer", 2);
			ao_comp->SetUniform("W", static_cast<float>(width));
			ao_comp->SetUniform("H", static_cast<float>(height));

			bilateral_horiz = ResourceLoader::LoadShaderProgramFromFile("shaders/bilateral_horizontal.shader");

			bilateral_vert = ResourceLoader::LoadShaderProgramFromFile("shaders/bilateral_vertical.shader");

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
			blur_weights = std::make_unique<Buffer<float>>();
			{
				auto unbind = blur_weights->Bind(BufferTargets::Vertex);
				blur_weights->SetUpStorage(2 * max_blur_width + 1, BufferCreateFlags::MapRead | BufferCreateFlags::MapWrite);
			}
			owner.SetShadowBlurWidth(shadow_blur_width);
			random_numbers = std::make_unique<Buffer<glm::vec2>>();

			base_ao_map = std::make_shared<Texture>();
			ao_fb = std::make_shared<Framebuffer>();
			{
				auto unbind_tex = base_ao_map->Bind(0);
				base_ao_map->AllocateSpace(width, height, TextureFormatInternal::R8, 1);
				auto unbind_fb = ao_fb->Bind();
				ao_fb->AttachTexture(Attachments::Color0, base_ao_map);
			}
			ao_horiz = std::make_shared<Texture>();
			{
				auto unbind = ao_horiz->Bind(0);
				ao_horiz->AllocateSpace(width, height, TextureFormatInternal::R8, 1);
			}
			ao_final = std::make_shared<Texture>();
			{
				auto unbind = ao_final->Bind(0);
				ao_final->AllocateSpace(width, height, TextureFormatInternal::R8, 1);
			}
		}
		
		void SetupSkysphere()
		{
			std::vector<std::pair<std::shared_ptr<Geometry>, unsigned>> geom;
			std::vector<std::shared_ptr<Material>> mtl;

			ResourceLoader::LoadObjFile(geom, mtl, "meshes/skysphere.obj");

			sky_sphere = geom.front().first;


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
				filtered_shadow_map->SetParameter(TextureParameter::MagFilter, TextureParamValue::FilterLinear);
			}
		}

		void InitSSR()
		{
			hi_z_buffer = std::make_shared<Texture>();
			{
				auto unbind = hi_z_buffer->Bind(0);
				hi_z_buffer->AllocateSpace(width, height, TextureFormatInternal::RG32F, ResourceLoader::ComputeMipLevels(width, height));
			}
		}
		
		void GeometryPass()
		{
			gl::Enable(gl::DEPTH_TEST);

			auto unbind = g_buffer->g_buff->Bind();
			g_buffer->g_buff->EnableAttachments({ Buffers::LightAccumulation, Buffers::Position, Buffers::Normal, Buffers::Diffuse,
				Buffers::Specular, Buffers::Alpha});
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
					shadow_map_shader->SetUniform("offset", 0.f);
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
				{
					auto unbind_buffer = blur_weights->Bind(BufferTargets::ShaderStorage, 0);
					//horizontal blur
					{
						auto unbind_shader = horizontal_blur->Bind();
						horizontal_blur->SetUniform("filter_width", shadow_blur_width);

						auto unbind_src_tex = shadow_map->BindImage(0, ImageAccessMode::ReadOnly);
						auto unbind_dest_tex = filtered_shadow_map->BindImage(1, ImageAccessMode::WriteOnly);

						horizontal_blur->RunCompute(shadow_map_size / compute_group_size, shadow_map_size);

						gl::MemoryBarrier(gl::SHADER_IMAGE_ACCESS_BARRIER_BIT);
					}

					//vertical blur
					{
						auto unbind_shader = vertical_blur->Bind();
						vertical_blur->SetUniform("filter_width", shadow_blur_width);

						auto unbind_src_tex = shadow_map->BindImage(1, ImageAccessMode::WriteOnly);
						auto unbind_dest_tex = filtered_shadow_map->BindImage(0, ImageAccessMode::ReadOnly);

						vertical_blur->RunCompute(shadow_map_size, shadow_map_size / compute_group_size);

						gl::MemoryBarrier(gl::TEXTURE_FETCH_BARRIER_BIT | gl::TEXTURE_UPDATE_BARRIER_BIT);
					}
				}

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

		void AOPass()
		{
			//compute ao
			{
				auto unbind_shader = ao_comp->Bind();
				auto unbind_fb = ao_fb->Bind();

				g_buffer->BindTextures(1, false);

				auto unbind_geom = FSQ->Bind();

				FSQ->Draw(PrimitiveTypes::Triangles, 6);

				g_buffer->UnbindTextures();
			}
			//blur
			{
				auto unbind_buffer = blur_weights->Bind(BufferTargets::ShaderStorage, 0);

				auto unbind3 = g_buffer->attachments[Buffers::Position]->BindImage(2, ImageAccessMode::ReadOnly);
				auto unbind4 = g_buffer->attachments[Buffers::Normal]->BindImage(3, ImageAccessMode::ReadOnly);
				{
					auto unbind1 = base_ao_map->BindImage(0, ImageAccessMode::ReadOnly);
					auto unbind2 = ao_horiz->BindImage(1, ImageAccessMode::WriteOnly);

					auto unbind_shader = bilateral_horiz->Bind();
					bilateral_horiz->SetUniform("filter_width", shadow_blur_width);

					bilateral_horiz->RunCompute(static_cast<int>(std::ceil(width / 128.f)), height);

					gl::MemoryBarrier(gl::SHADER_IMAGE_ACCESS_BARRIER_BIT);
				}
				{
					auto unbind1 = ao_horiz->BindImage(0, ImageAccessMode::ReadOnly);
					auto unbind2 = ao_final->BindImage(1, ImageAccessMode::WriteOnly);

					auto unbind_shader = bilateral_vert->Bind();
					bilateral_vert->SetUniform("filter_width", shadow_blur_width);

					bilateral_vert->RunCompute(width, static_cast<int>(std::ceil(height / 128.f)));

					gl::MemoryBarrier(gl::TEXTURE_FETCH_BARRIER_BIT | gl::TEXTURE_UPDATE_BARRIER_BIT);
				}
			}
		}

		void ScreenSpaceReflect()
		{
			//create Hi-Z


			//coverage (if cone trace)

			//raytrace

			//blur (if cone trace)

			//cone trace
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
				if (sky_sphere_img)
				{
					auto unbind_irradiance = sky_sphere_irradiance->Bind(6);
					auto unbind_skysphere = sky_sphere_img->Bind(7);
					auto unbind_ao = ao_final->Bind(8);
					ambient_shader->SetUniform("CamPos", owner.current_cam->owner_world_trans_.position);
					ambient_shader->SetUniform("NumSamples", num_samples);
					auto unbind_random = random_numbers->Bind(BufferTargets::ShaderStorage, 0);
					auto unbind_vao = FSQ->Bind();

					{
						auto unbind_shader = ambient_shader->Bind();
						FSQ->Draw(PrimitiveTypes::Triangles, 6);
					}
				}

				NonShadowLights(view, proj);
			}

			ShadowLights(view, proj);

			//SSR
			if (do_ssr)
				ScreenSpaceReflect();

			auto unbind_buffer = g_buffer->g_buff->Bind();
			g_buffer->g_buff->EnableAttachments({ Buffers::LightAccumulation, Buffers::Position, Buffers::Normal, Buffers::Diffuse,
				Buffers::Specular, Buffers::Alpha});

			gl::Disable(gl::BLEND);
		}

		void CopyBufferPass()
		{
			gl::Disable(gl::DEPTH_TEST);

			g_buffer->BindTextures(1);
			auto unbind_tex = base_ao_map->Bind(7);
			auto unbind_tex2 = ao_horiz->Bind(8);
			auto unbind_tex3 = ao_final->Bind(9);

			auto unbind_shader = buffer_copy->Bind();
			auto unbind_vao = FSQ->Bind();
			buffer_copy->SetUniform("exposure", exposure);
			buffer_copy->SetUniform("contrast", contrast);
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

		void RenderSkysphere()
		{
			gl::Enable(gl::DEPTH_TEST);
			gl::DepthFunc(gl::LEQUAL);
			
			glm::mat4 view, proj;
			view = owner.current_cam->GetViewMatrix();
			proj = owner.current_cam->GetProjectionMatrix();

			view = glm::mat4(glm::mat3(view));

			{
				auto unbind_fbo = g_buffer->g_buff->Bind();
				g_buffer->g_buff->EnableAttachments({ Buffers::LightAccumulation });
				sky_sphere_shader->SetUniform("MVP", proj * view);
				auto unbind_tex = sky_sphere_img->Bind(1);
				auto unbind_shader = sky_sphere_shader->Bind();

				sky_sphere->Draw();
			}

			gl::DepthFunc(gl::LESS);

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
		impl->SetupSkysphere();
		impl->InitSSR();
		SetNumSamples(30);
	}

	void GraphicsManager::Update()
	{
		//clear the default frame buffer
		gl::Clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);
		impl->GeometryPass();

		//AO
		impl->AOPass();

		impl->LightingPass();
		
		if (impl->sky_sphere_img)
			impl->RenderSkysphere();

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

	void GraphicsManager::SetShadowBlurWidth(int w)
	{
		if (w > max_blur_width)
			return;

		impl->shadow_blur_width = w;

		auto unbind = impl->blur_weights->Bind(BufferTargets::Vertex);
		float *weights = impl->blur_weights->Map(MapModes::Write);

		float s = static_cast<float>(w) / 3.f;
		float total = 0.f;
		for (int i = 0; i < 2 * w + 1; ++i)
		{
			float weight = std::exp(-0.5f * ((i - w) / s) * ((i - w) / s));
			total += weight;
		}

		for (int i = 0; i < 2 * w + 1; ++i)
			weights[i] = std::exp(-0.5f * ((i - w) / s) * ((i - w) / s)) / total;

		impl->blur_weights->Unmap();
	}

	void GraphicsManager::SetSkybox(const std::string & filename)
	{
		impl->sky_sphere_img = ResourceLoader::LoadHDRTexFromFile((filename + ".hdr").c_str());
		impl->sky_sphere_irradiance = ResourceLoader::LoadHDRTexFromFile((filename + ".irr.hdr").c_str());
		{
			auto unbind = impl->sky_sphere_img->Bind(1);
			impl->sky_sphere_img->SetParameter(TextureParameter::MagFilter, TextureParamValue::FilterLinear);
			impl->sky_sphere_img->SetParameter(TextureParameter::MinFilter, TextureParamValue::FilterLinearMipmapLinear);
		}
	}

	void GraphicsManager::SetExposure(float e)
	{
		impl->exposure = e;
	}

	void GraphicsManager::SetContrast(float c)
	{
		impl->contrast = c;
	}
	void GraphicsManager::SetNumSamples(int n)
	{
		impl->num_samples = n;

		auto unbind = impl->random_numbers->Bind(BufferTargets::Vertex);

		if (static_cast<int>(impl->random_numbers->GetSize()) < n)
		{
			impl->random_numbers->ResizeableStorage(n);
		}

		glm::vec2* data = impl->random_numbers->Map(MapModes::Write);

		float p, u;
		int kk;

		for (int i = 0; i < n; ++i)
		{
			for (p = 0.5f, kk = i, u = 0.0f; kk; p *= 0.5f, kk >>= 1)
				if (kk & 1)
					u += p;
			float v = (i + 0.5f) / n;
			data[i].x = u;
			data[i].y = v;
		}

		impl->random_numbers->Unmap();
	}

	void GraphicsManager::SetAORadius(float r)
	{
		impl->ao_comp->SetUniform("R", r);
		impl->ao_comp->SetUniform("c", r * 0.1f);
	}

	void GraphicsManager::SetAODelta(float d)
	{
		impl->ao_comp->SetUniform("delta", d);
	}

	void GraphicsManager::SetAOSamples(int n)
	{
		impl->ao_comp->SetUniform("n", n);
	}

	void GraphicsManager::SetAOScale(float s)
	{
		impl->ao_comp->SetUniform("s", s);
	}

	void GraphicsManager::SetAOContrast(float k)
	{
		impl->ao_comp->SetUniform("k", k);
	}
}
