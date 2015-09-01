////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "AABB.h"

#include <numeric>

namespace
{
	//bottom back left		0
	//bottom back right		1
	//top back left			2
	//top back right		3
	//bottom front left		4
	//bottom front right	5
	//top front left		6
	//top front right		7

	unsigned idx_data[] =
	{0,1,0,4,0,2,1,3,1,5,2,3,2,6,3,7,4,5,4,6,5,7,6,7};
}

namespace CS350
{
	AABB::AABB(std::shared_ptr<Geometry> geometry,
		const Transformation& world_trans)
		: reference_geometry_(geometry),
		world_trans_(&world_trans),
		vao_(std::make_shared<VertexArray>()),
		vertices_(std::make_shared<Buffer<glm::vec3>>())
	{
		InitBuffers();

		glm::mat4 model_mtx = world_trans_->GetMatrix();

		GetExtents(reference_geometry_->vertices, model_mtx, min_, max_);

		GenerateWireframeBox();
	}

	AABB::AABB(const std::vector<Vertex>& vertices, const glm::mat4& model_matrix)
		: vao_(std::make_shared<VertexArray>()),
		vertices_(std::make_shared<Buffer<glm::vec3>>())
	{
		InitBuffers();
		GetExtents(vertices, model_matrix, min_, max_);
		GenerateWireframeBox();
	}

	AABB::AABB(const glm::vec3& min, const glm::vec3& max)
		: min_(min), max_(max), 
		vao_(std::make_shared<VertexArray>()),
		vertices_(std::make_shared<Buffer<glm::vec3>>())
	{
		InitBuffers();
		GenerateWireframeBox();
	}

	AABB::AABB()
		: min_(0), max_(0)
	{}

	AABB::AABB(AABB&& rhs)
		: min_(rhs.min_), max_(rhs.max_), 
		reference_geometry_(rhs.reference_geometry_),
		world_trans_(rhs.world_trans_),
		vao_(std::move(rhs.vao_)),
		vertices_(std::move(rhs.vertices_))
	{}

	AABB& AABB::operator=(AABB&& rhs)
	{
		min_ = rhs.min_;
		max_ = rhs.max_;
		reference_geometry_ = rhs.reference_geometry_;
		world_trans_ = rhs.world_trans_;
		vao_ = std::move(rhs.vao_);
		vertices_ = std::move(rhs.vertices_);

		return *this;
	}

	void AABB::InitBuffers()
	{
		auto indices = std::make_shared<Buffer<unsigned>>();
		{
			auto unbind = indices->Bind(BufferTargets::Vertex);
			indices->SetUpStorage(24, BufferCreateFlags::None, idx_data);
		}
		{
			auto unbind = vertices_->Bind(BufferTargets::Vertex);
			vertices_->SetUpStorage(8, BufferCreateFlags::MapWrite);
		}
		{
			auto unbind = vao_->Bind();
			vao_->AddIndexBuffer(indices);
			unsigned buff_idx = vao_->AddDataBuffer(vertices_, sizeof(glm::vec3));
			vao_->SetAttributeAssociation(0, buff_idx, 3, DataTypes::Float, 0);
		}
	}

	void AABB::GenerateWireframeBox()
	{
		glm::vec3 corners[8] =
		{
			glm::vec3(min_.x, min_.y, min_.z), //bottom back left
			glm::vec3(max_.x, min_.y, min_.z), //bottom back right
			glm::vec3(min_.x, max_.y, min_.z), //top back left
			glm::vec3(max_.x, max_.y, min_.z), //top back right
			glm::vec3(min_.x, min_.y, max_.z), //bottom front left
			glm::vec3(max_.x, min_.y, max_.z), //bottom front right
			glm::vec3(min_.x, max_.y, max_.z), //top front left
			glm::vec3(max_.x, max_.y, max_.z), //top front right
		};

		auto unbind = vertices_->Bind(BufferTargets::Vertex);
		glm::vec3 *data = vertices_->Map(MapModes::Write);
		std::memcpy(data, corners, sizeof(corners));
		vertices_->Unmap();
	}

	void AABB::Recompute()
	{
		if (world_trans_ && reference_geometry_)
		{
			glm::mat4 model_mtx = world_trans_->GetMatrix();

			GetExtents(reference_geometry_->vertices, model_mtx, min_, max_);

			GenerateWireframeBox();
		}
	}

	void AABB::Update()
	{
		Recompute();
	}

	void AABB::Draw(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, const glm::vec4& color)
	{
		if (shader && vao_)
		{
			glm::mat4 MVP = projection * view;

			shader->SetUniform("MVP", MVP);
			shader->SetUniform("line_color", color);
			auto unbind_shader_ = shader->Bind();
			auto unbind_vao = vao_->Bind();
			vao_->Draw(PrimitiveTypes::Lines, 24);
		}
	}

	void AABB::GetExtents(const std::vector<Vertex>& vertices, const glm::mat4& model_matrix, glm::vec3& min, glm::vec3& max)
	{
		min = glm::vec3(std::numeric_limits<float>::max());
		max = glm::vec3(std::numeric_limits<float>::lowest());

		for (auto v : vertices)
		{
			glm::vec4 p_in_world = model_matrix * glm::vec4(v.pos, 1);

			if (p_in_world.x < min.x)
				min.x = p_in_world.x;
			if (p_in_world.y < min.y)
				min.y = p_in_world.y;
			if (p_in_world.z < min.z)
				min.z = p_in_world.z;

			if (p_in_world.x > max.x)
				max.x = p_in_world.x;
			if (p_in_world.y > max.y)
				max.y = p_in_world.y;
			if (p_in_world.z > max.z)
				max.z = p_in_world.z;
		}
	}

	glm::vec3 AABB::min() const
	{
		return min_;
	}

	glm::vec3 AABB::max() const
	{
		return max_;
	}

	void AABB::GetMinMax(glm::vec3& min, glm::vec3& max) const
	{
		min = min_;
		max = max_;
	}

	float AABB::SurfaceArea() const
	{
		auto extent = max_ - min_;

		float area = 2 * (extent.x * extent.y + extent.x * extent.z + extent.y * extent.z);

		return area;
	}
}
