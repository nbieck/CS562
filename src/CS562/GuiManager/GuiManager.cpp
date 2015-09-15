////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "GuiManager.h"

#include "../ResourceLoader/ResourceLoader.h"
#include "../GLWrapper/ContextState.h"

#include <glm/gtc/type_ptr.hpp>

namespace CS562
{
	GuiManager* GuiManager::g_gui_ = nullptr;

	void GuiManager::RenderImGui(ImDrawList** const cmd_lists, int cmd_lists_count)
	{
		if (!g_gui_)
			return;

		if (cmd_lists_count == 0)
			return;

		gl::Enable(gl::BLEND);
		gl::BlendEquation(gl::FUNC_ADD);
		gl::BlendFuncSeparate(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA, gl::ONE, gl::ZERO);
		gl::Disable(gl::CULL_FACE);
		gl::Disable(gl::DEPTH_TEST);
		gl::Enable(gl::SCISSOR_TEST);
		
		// Setup orthographic projection matrix
		const float width = ImGui::GetIO().DisplaySize.x;
		const float height = ImGui::GetIO().DisplaySize.y;
		const float ortho_projection[] =
		{
			2.0f / width, 0.0f, 0.0f, 0.0f,
			0.0f, 2.0f / -height, 0.0f, 0.0f,
			0.0f, 0.0f, -1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 1.0f,
		};
		glm::mat4 proj = glm::make_mat4(ortho_projection);

		g_gui_->shader_->SetUniform("ProjMtx", proj);
		g_gui_->shader_->SetUniform("Texture", 0);

		size_t total_vtx_count = 0;
		for (int n = 0; n < cmd_lists_count; ++n)
			total_vtx_count += cmd_lists[n]->vtx_buffer.size();

		auto unbind_buffer = g_gui_->vbo_->Bind(BufferTargets::Vertex);
		if (total_vtx_count > g_gui_->vbo_->GetSize())
		{
			unsigned new_size = g_gui_->vbo_->GetSize() + 5000;
			g_gui_->vbo_->ResizeableStorage(new_size);
		}

		unsigned offset = 0;
		for (int i = 0; i < cmd_lists_count; ++i)
		{
			const ImDrawList *cmd_list = cmd_lists[i];
			g_gui_->vbo_->SendData(offset, static_cast<unsigned>(cmd_list->vtx_buffer.size()), &cmd_list->vtx_buffer[0]);
			offset += static_cast<unsigned>(cmd_list->vtx_buffer.size());
		}

		auto unbind_vao = g_gui_->vao_->Bind();
		auto unbind_shader = g_gui_->shader_->Bind();
		int cmd_offset = 0;
		for (int i = 0; i < cmd_lists_count; ++i)
		{
			const ImDrawList* cmd_list = cmd_lists[i];
			const ImDrawCmd* commands_end = cmd_list->commands.end();
			for (const ImDrawCmd* command = cmd_list->commands.begin(); command != commands_end; command++)
			{
				gl::Scissor((int)command->clip_rect.x, (int)(height - command->clip_rect.w), (int)(command->clip_rect.z - command->clip_rect.x), (int)(command->clip_rect.w - command->clip_rect.y));
				g_gui_->vao_->Draw(PrimitiveTypes::Triangles, command->vtx_count, cmd_offset);
				cmd_offset += command->vtx_count;
			}
		}

		gl::Disable(gl::SCISSOR_TEST);
		gl::Enable(gl::DEPTH_TEST);
		gl::Enable(gl::CULL_FACE);
		gl::Disable(gl::BLEND);
	}

	GuiManager::GuiManager(unsigned width, unsigned height, FrameTimer& time, InputManager& input)
		: time_(time), input_(input), vbo_(std::make_shared<Buffer<ImDrawVert>>()), vao_(std::make_unique<VertexArray>())
		, gui_visible_(false)
	{
		//setup graphics data necessary for drawing
		shader_ = ResourceLoader::LoadShaderProgramFromFile("shaders/editorgui.shader");

		{
			auto unbind_buffer = vbo_->Bind(BufferTargets::Vertex);
			vbo_->ResizeableStorage(1000);
		}

		auto unbind = vao_->Bind();
		unsigned buffer_idx = vao_->AddDataBuffer(vbo_, sizeof(ImDrawVert));
		vao_->SetAttributeAssociation(0, buffer_idx, 2, DataTypes::Float, 0);
		vao_->SetAttributeAssociation(1, buffer_idx, 2, DataTypes::Float, sizeof(ImVec2));
		vao_->SetAttributeAssociation(2, buffer_idx, 4, DataTypes::UnsignedByte, sizeof(ImVec2) * 2, true);
		
		//initial ImGui setup
		ImGuiIO& io = ImGui::GetIO();

		unsigned char *pixels;
		int tex_width, tex_height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &tex_width, &tex_height);

		gl::GenTextures(1, &gui_tex_);
		ContextState::SetActiveTextureUnit(0);
		gl::BindTexture(gl::TEXTURE_2D, gui_tex_);
		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR);
		gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR);
		gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, tex_width, tex_height, 0, gl::RGBA, gl::UNSIGNED_BYTE, pixels);
		//we have only the one texture, so don't unbind

		io.DisplaySize = { static_cast<float>(width), static_cast<float>(height) };
		io.RenderDrawListsFn = RenderImGui;

		ImGui::NewFrame();

		g_gui_ = this;
	}

	GuiManager::~GuiManager()
	{
		gl::BindTexture(gl::TEXTURE_2D, 0);
		gl::DeleteTextures(1, &gui_tex_);
	}

	void GuiManager::Update()
	{
		ImGuiIO& io = ImGui::GetIO();

		float dt = static_cast<float>(time_.dt());
		io.DeltaTime = (dt > 0) ? dt : std::numeric_limits<float>::min();
		
		ImGui::NewFrame();

		if (input_.isKeyTriggered('G'))
			gui_visible_ = !gui_visible_;

		DrawInfoOverlay();
	}

	void GuiManager::DrawInfoOverlay()
	{

		if (!ImGui::Begin("FrameRateDisplay", nullptr, ImVec2(0, 0), 0.3f, 
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::End();
			return;
		}

		ImGui::SetWindowPos(ImVec2(10, 10));
		ImGui::Text("FPS : %f", time_.fps());
		ImGui::Separator();
		ImGui::Text("Press 'G' to show the GUI.");
		ImGui::End();
	}

	void GuiManager::StartGuiWindow()
	{
		ImGui::Begin("CS562 Options");
		ImGui::Text("Mouse over for GUI instructions.");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::ShowUserGuide();
			ImGui::EndTooltip();
		}
		ImGui::Text("Mouse over for keyboard shortcuts.");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("WASD - Horizontal movement");
			ImGui::Text("QE   - Vertical movement");
			ImGui::Text("G    - Show/Hide GUI");
			ImGui::EndTooltip();
		}
		ImGui::Separator();
	}

	void GuiManager::EndGuiWindow()
	{
		ImGui::End();
	}

	bool GuiManager::GuiVisible() const
	{
		return gui_visible_;
	}
}
