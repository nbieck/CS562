////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS350
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_BOUNDING_SPHERE_H_
#define CS350_BOUNDING_SPHERE_H_

#include "IBoundingVolume.h"

#include "../GLWrapper/ShaderProgram.h"
#include "../Geometry/Geometry.h"
#include "../Transformation/Transformation.h"

#include <memory>

namespace CS350
{
	class BoundingSphere : public IBoundingVolume
	{
	public:
		
		static std::shared_ptr<BoundingSphere> CentroidMethod(std::shared_ptr<Geometry> geometry, const Transformation& world_trans, std::shared_ptr<ShaderProgram> shader);

		static std::shared_ptr<BoundingSphere> Ritters(std::shared_ptr<Geometry> geometry, const Transformation& world_trans, std::shared_ptr<ShaderProgram> shader);

		static std::shared_ptr<BoundingSphere> IterativeRefinement(std::shared_ptr<Geometry> geometry, const Transformation& world_trans, std::shared_ptr<ShaderProgram> shader, unsigned num_iterations);

		void Update() override;
		void Draw(const glm::mat4& view, const glm::mat4& projection, std::shared_ptr<ShaderProgram> shader, const glm::vec4& color) override;
		
		void GetMinMax(glm::vec3& min, glm::vec3& max) const override;

		BoundingSphere(const glm::vec3& center, float radius, const Transformation& world_trans);

	private:

		static void RittersSphere(glm::vec3& center, float &radius, const std::vector<Vertex>& vertices);
		static void FindMaxSeparatedPointsOnAxis(const std::vector<Vertex>& vertices, glm::vec3& min, glm::vec3& max);
		static void GrowSphereIfNeeded(glm::vec3& center, float& radius, const glm::vec3& new_point);

		glm::vec3 center_;
		float radius_;

		std::shared_ptr<VertexArray> vao_;
		const Transformation& world_trans_;
		unsigned num_idx_;
	};
}

#endif
