#ifndef CS350_NARROW_PHASE_H_
#define CS350_NARROW_PHASE_H_

#include "../Scene/Object.h"
#include "Contact.h"

#include <memory>
#include <unordered_map>

namespace CS350
{
	struct SeparatingPlane
	{
		glm::vec3 p;
		glm::vec3 n;
	};

	class SeparatingAxisTest
	{
	public:

		bool operator()(std::shared_ptr<Object> obj1, std::shared_ptr<Object> obj2);

		SeparatingPlane GetLastSeparatingPlane() const;

	private:

		struct Edge
		{
			glm::vec3 vector;
			glm::vec3 p0;
			glm::vec3 p1;
		};

		std::vector<glm::vec3> ExtractNormals(const std::vector<glm::vec3>& positions, 
			const std::vector<unsigned>& indices);

		std::vector<Edge> ExtractEdges(const std::vector<glm::vec3>& positions, 
			const std::vector<unsigned>& indices);

		void ComputeInterval(const std::vector<glm::vec3>& positions, glm::vec3 axis,
			float& min, float& max);

		bool IsZeroVector(glm::vec3 vec);

		bool CheckAxis(const std::vector<glm::vec3>& pos1, const std::vector<glm::vec3>& pos2,
			glm::vec3 axis, float& middle_point);
		
		void CompSepPlane(glm::vec3 obj_pos, glm::vec3 axis, float middle_point);

		using ObjectPair = std::pair < std::shared_ptr<Object>, std::shared_ptr<Object> > ;

		struct HashObjPair
		{
			size_t operator()(const ObjectPair& pair);
		};

		struct EquObjPair
		{
			bool operator()(const ObjectPair& left, const ObjectPair& right);
		};

		std::unordered_map<ObjectPair, glm::vec3, HashObjPair, EquObjPair> last_separating_axis_;

		SeparatingPlane last_sep_plane_;

	};
}

#endif
