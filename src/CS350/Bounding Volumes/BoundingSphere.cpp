////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#include "BoundingSphere.h"

#include "AABB.h"

#include <glm/gtc/constants.hpp>

#include <algorithm>

namespace CS350
{
	namespace
	{
		void GenerateSphere(unsigned stacks, unsigned slices, float radius, glm::vec3 center, unsigned& num_idx, std::shared_ptr<VertexArray>& vao)
		{
			std::vector<glm::vec3> vertices;
			std::vector<unsigned> indices;

			const float phi_slice = glm::pi<float>() / (stacks);
			const float theta_slice = glm::two_pi<float>() / slices;

			float phi = phi_slice;
			float theta = 0;

			//generate vertices for top triangle fan
			vertices.push_back(center + glm::vec3(0, radius, 0));
			for (unsigned i = 0; i < slices; ++i)
			{
				float cos_phi = cos(phi);
				float sin_phi = sin(phi);
				float cos_theta = cos(theta);
				float sin_theta = sin(theta);

				vertices.push_back(center + glm::vec3(radius * sin_phi * sin_theta, radius * cos_phi, radius * sin_phi * cos_theta));

				theta += theta_slice;
			}
			//indices for top triangle fan
			for (unsigned i = 0; i < slices; ++i)
			{
				indices.push_back(0);
				indices.push_back(i + 1);
				indices.push_back(i + 1);
				indices.push_back(1 + (i + 1) % slices);
			}

			//generate all the rows along the side
			for (unsigned j = 0; j < stacks - 2; ++j)
			{
				phi += phi_slice;
				float cos_phi = cos(phi);
				float sin_phi = sin(phi);

				theta = 0.f;
				for (unsigned i = 0; i < slices; ++i)
				{
					float cos_theta = cos(theta);
					float sin_theta = sin(theta);

					vertices.push_back(center + glm::vec3(radius * sin_phi * sin_theta, radius * cos_phi, radius * sin_phi * cos_theta));

					theta += theta_slice;
				}
			}
			//indices for the rows
			for (unsigned j = 1; j < stacks - 1; ++j)
			{
				for (unsigned i = 0; i < slices; ++i)
				{
					indices.push_back(1 + (j - 1) * slices + i);
					indices.push_back(1 + j * slices + i);
					indices.push_back(1 + j * slices + i);
					indices.push_back(1 + j * slices + (i + 1) % slices);
				}
			}

			//add bottom center point
			vertices.push_back(center + glm::vec3(0, -radius, 0));

			//indices for bottom triangle fan
			for (unsigned i = 0; i < slices; ++i)
			{
				indices.push_back(static_cast<unsigned>(vertices.size() - 1));
				indices.push_back(static_cast<unsigned>(vertices.size() - 1 - i));
			}

			auto vtx_buffer = std::make_shared<Buffer<glm::vec3>>();
			auto idx_buffer = std::make_shared<Buffer<unsigned>>();

			{
				auto unbind = vtx_buffer->Bind(BufferTargets::Vertex);
				vtx_buffer->SetUpStorage(static_cast<unsigned>(vertices.size()), 0, &vertices[0]);
			}
			{
				auto unbind = idx_buffer->Bind(BufferTargets::Vertex);
				idx_buffer->SetUpStorage(static_cast<unsigned>(indices.size()), 0, &indices[0]);
			}

			vao = std::make_shared<VertexArray>();
			{
				auto unbind = vao->Bind();
				vao->AddIndexBuffer(idx_buffer);
				vao->AddDataBuffer(vtx_buffer, sizeof(glm::vec3));
				vao->SetAttributeAssociation(0, 0, 3, DataTypes::Float, 0);
			}

			num_idx = static_cast<unsigned>(indices.size());
		}
	}

	BoundingSphere::BoundingSphere(const glm::vec3& center, float radius, const Transformation& worl_trans)
		: center_(center), radius_(radius), world_trans_(worl_trans)
	{
		GenerateSphere(20, 12, radius, center, num_idx_, vao_);
	}

	void BoundingSphere::Update()
	{}

	void BoundingSphere::Draw(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, const glm::vec4& color)
	{
		shader->SetUniform("MVP", projection * view * world_trans_.GetMatrix());
		auto unbind = shader->Bind();
		auto vao_unbind = vao_->Bind();
		vao_->Draw(PrimitiveTypes::Lines, num_idx_);
	}

	std::shared_ptr<BoundingSphere> BoundingSphere::CentroidMethod(std::shared_ptr<Geometry> geometry, const Transformation& world_trans, std::shared_ptr<ShaderProgram> shader)
	{
		glm::vec3 center;
		float radius = 1.f;

		glm::vec3 min, max;
		AABB::GetExtents(geometry->vertices, glm::mat4(), min, max);

		center = (max + min) / 2.f;
		radius = glm::length(max - center);

		return std::make_shared<BoundingSphere>(center, radius, world_trans);
	}

	std::shared_ptr<BoundingSphere> BoundingSphere::Ritters(std::shared_ptr<Geometry> geometry, const Transformation& world_trans, std::shared_ptr<ShaderProgram> shader)
	{
		glm::vec3 center;
		float radius;

		RittersSphere(center, radius, geometry->vertices);

		return std::make_shared<BoundingSphere>(center, radius, world_trans);
	}

	std::shared_ptr<BoundingSphere> BoundingSphere::IterativeRefinement(std::shared_ptr<Geometry> geometry, const Transformation& world_trans, std::shared_ptr<ShaderProgram> shader, unsigned num_iterations)
	{
		glm::vec3 center;
		float radius;

		std::vector<Vertex> vertices(geometry->vertices);

		RittersSphere(center, radius, vertices);

		for (; num_iterations > 0; num_iterations--)
		{
			std::random_shuffle(vertices.begin(), vertices.end());

			float new_radius = radius * 0.95f;
			glm::vec3 new_center = center;

			for (auto v : vertices)
				GrowSphereIfNeeded(new_center, new_radius, v.pos);

			if (new_radius < radius)
			{
				radius = new_radius;
				center = new_center;
			}
		}

		return std::make_shared<BoundingSphere>(center, radius, world_trans);
	}

	void BoundingSphere::RittersSphere(glm::vec3& center, float &radius, const std::vector<Vertex>& vertices)
	{
		glm::vec3 min, max;
		FindMaxSeparatedPointsOnAxis(vertices, min, max);

		center = (max + min) / 2.f;
		radius = glm::length(max - center);

		for (auto v : vertices)
			GrowSphereIfNeeded(center, radius, v.pos);

	}

	void BoundingSphere::FindMaxSeparatedPointsOnAxis(const std::vector<Vertex>& vertices, glm::vec3& min, glm::vec3& max)
	{
		int minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0;
		for (unsigned i = 1; i < vertices.size(); ++i)
		{
			if (vertices[i].pos.x < vertices[minx].pos.x) minx = i;
			if (vertices[i].pos.x > vertices[maxx].pos.x) maxx = i;
			if (vertices[i].pos.y < vertices[miny].pos.y) miny = i;
			if (vertices[i].pos.y > vertices[maxy].pos.y) maxy = i;
			if (vertices[i].pos.z < vertices[minz].pos.z) minz = i;
			if (vertices[i].pos.z > vertices[maxz].pos.z) maxz = i;
		}

		float dist_x, dist_y, dist_z;
		dist_x = glm::distance(vertices[minx].pos, vertices[maxx].pos);
		dist_y = glm::distance(vertices[miny].pos, vertices[maxy].pos);
		dist_z = glm::distance(vertices[minz].pos, vertices[maxz].pos);

		max = vertices[maxx].pos;
		min = vertices[minx].pos;

		if (dist_y > dist_x && dist_y > dist_z)
		{
			max = vertices[maxy].pos;
			min = vertices[miny].pos;
		}
		else if (dist_z > dist_x && dist_z > dist_y)
		{
			max = vertices[maxz].pos;
			min = vertices[minz].pos;
		}
	}

	void BoundingSphere::GrowSphereIfNeeded(glm::vec3& center, float& radius, const glm::vec3& new_point)
	{
		//taken from the textbook
		glm::vec3 separation = new_point - center;
		float d_2 = glm::dot(separation, separation);

		//update sphere if necessary
		if (d_2 > radius * radius)
		{
			float d = glm::sqrt(d_2);
			float new_radius = (radius + d) * 0.5f;
			float k = (new_radius - radius) / d;
			radius = new_radius;
			center += separation * k;
		}
	}

	void BoundingSphere::GetMinMax(glm::vec3& min, glm::vec3& max) const
	{
		glm::vec3 center_world = glm::vec3(world_trans_.GetMatrix() * glm::vec4(center_, 1));
		min = center_world - glm::vec3(radius_);
		max = center_world + glm::vec3(radius_);
	}
}
