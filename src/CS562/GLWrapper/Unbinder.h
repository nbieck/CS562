////////////////////////////////////////////////////////////////////////////////////////////
//
//	Author: Niklas Bieck
//  Subject: CS562
//
////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CS350_UNBINDER_H_
#define CS350_UNBINDER_H_

namespace CS562
{
	/*!
		\brief
			This class will call the unbind function of the resource it references at end of scope
	*/
	template <typename Resource>
	class Unbinder
	{
	public:

		Unbinder(Resource& res)
			: res(res)
		{}

		//we want move semantics for this so we can return it
		Unbinder(Unbinder<Resource>&& rhs)
			:res(rhs.res)
		{}

		~Unbinder()
		{
			res.Unbind();
		}

	private:

		Resource& res;

	};
}

#endif
