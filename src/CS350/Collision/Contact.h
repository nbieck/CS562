#ifndef CS350_CONTACT_H_
#define CS350_CONTACT_H_

#include "../Scene/Object.h"

#include <memory>

namespace CS350
{
	struct Contact
	{
		std::shared_ptr<Object> obj1;
		std::shared_ptr<Object> obj2;
	};
}

#endif
