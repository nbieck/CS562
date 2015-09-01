////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_AABB_H_
#define CS350_AABB_H_

#include "IBoundingVolume.h"

#include "../Geometry/Geometry.h"
#include "../GLWrapper/ShaderProgram.h"
#include "../Transformation/Transformation.h"
#include "../GLWrapper/Buffer.h"
#include "../GLWrapper/VertexArray.h"

namespace CS350
{
	class AABB : public IBoundingVolume
	{
	public:

		AABB();
		AABB(const glm::vec3& min, const glm::vec3& max);
		AABB(const std::vector<Vertex>& vertices, const glm::mat4& model_matrix);
		AABB(std::shared_ptr<Geometry> geometry, const Transformation& world_trans);

		AABB(const AABB& other) = delete;
		AABB& operator=(const AABB& other) = delete;

		AABB(AABB&& rhs);
		AABB& operator=(AABB&& rhs);

		void Recompute();

		void Update() override;
		void Draw(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, const glm::vec4& color) override;

		void GetMinMax(glm::vec3& min, glm::vec3& max) const override;

		float SurfaceArea() const;

		static void GetExtents(const std::vector<Vertex>& vertices, const glm::mat4& model_matrix, glm::vec3& min, glm::vec3& max);

		glm::vec3 min() const;
		glm::vec3 max() const;

	private:

		void InitBuffers();
		void GenerateWireframeBox();

		glm::vec3 min_, max_;

		std::shared_ptr<Geometry> reference_geometry_;
		const Transformation* world_trans_;

		std::shared_ptr<VertexArray> vao_;
		std::shared_ptr<Buffer<glm::vec3>> vertices_;

	};
}

#endif
