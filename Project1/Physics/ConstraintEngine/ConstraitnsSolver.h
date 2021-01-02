#pragma once
#include "../../Utils/General.h"
class GameObject;
namespace Component {
	class RigidBody;
}
namespace Physics {
	namespace Constraints {
		class Constraint;
		class ConstraintsSolver
		{
		private:
			static std::list <Constraint*> constraints;
		public:
			static void cleanUp();
			static void addConstraint(Constraint* c);
			template<class T, typename... ARGS>
			static void addConstraint(ARGS&&... args);
			static void solveAll(const float& dt);
			static void preSolveAll();
		};
	};
};

