////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "Geometry.h"

namespace CS562
{
	Geometry::Geometry(const std::vector<Vertex>& vertices,
					   const std::vector<unsigned>& indices, 
					   PrimitiveTypes::PrimitiveTypes type)
		: vertices(vertices), indices(indices), type_(type), num_indices_(static_cast<unsigned>(indices.size())),
		idx_buffer_(std::make_shared<Buffer<unsigned>>()),
		vtx_buffer_(std::make_shared<Buffer<Vertex>>()),
		vao_(std::make_unique<VertexArray>())
	{
		{
			auto unbind = idx_buffer_->Bind(BufferTargets::Vertex);
			idx_buffer_->SetUpStorage(static_cast<unsigned>(indices.size()), BufferCreateFlags::None, indices.data());
		}
		{
			auto unbind = vtx_buffer_->Bind(BufferTargets::Vertex);
			vtx_buffer_->SetUpStorage(static_cast<unsigned>(vertices.size()), BufferCreateFlags::None, vertices.data());
		}
		{
			auto unbind = vao_->Bind();
			vao_->AddIndexBuffer(idx_buffer_);
			unsigned buffer_idx = vao_->AddDataBuffer(vtx_buffer_, sizeof(Vertex));
			vao_->SetAttributeAssociation(0, buffer_idx, 3, DataTypes::Float, 0);
			vao_->SetAttributeAssociation(1, buffer_idx, 3, DataTypes::Float, sizeof(Vertex::Position));
			vao_->SetAttributeAssociation(2, buffer_idx, 2, DataTypes::Float, sizeof(Vertex::Position) + sizeof(Vertex::Normal));
		}
	}

	void Geometry::Draw()
	{
		auto unbind = vao_->Bind();
		vao_->Draw(type_, num_indices_);
	}

	void Geometry::DrawInstanced(unsigned num_instances)
	{
		auto unbind = vao_->Bind();
		vao_->DrawInstanced(type_, num_indices_, num_instances);
	}
}
