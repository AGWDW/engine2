#include "ConstraintsBased.h"
#include "../../Componets/Rigidbody.h"

Physics::ConstraintsBased::ConstraintsBased() : Resolution(), constraints()
{
}

void Physics::ConstraintsBased::resolve(Component::RigidBody* a, Component::RigidBody* b, Physics::CollisionManfold& manafold)
{
	/*auto& c_1 = a->getConstraints();
	auto& c_2 = b->getConstraints();
	for (Constraint* c : c_1) {
		// Constraint* c = Utils::elementAt(c_1, i);
		if (Utils::find(c_2, c)) {
			// std::cout << "collision detected: " << i++ << "\r";
			Utils::BigMaths::Vector12 V;
			Utils::BigMaths::Matrix12 M;

			V = Utils::BigMaths::combine(a->getVelocities(), b->getVelocities());
			M = Utils::BigMaths::combine(a->getMassMatrix(), b->getMassMatrix());
			c->solve(V, M, manafold);
			// const Utils::BigMaths::Vector12 delta = c->solve(V, M, manafold);
			// a->applyForce({ delta[0], delta[1], delta[2] }, { delta[3], delta[4], delta[5] });
			// b->applyForce({ delta[6], delta[7], delta[8] }, { delta[9], delta[10], delta[11] });
		}
	}*/
}

void Physics::ConstraintsBased::cleanUp()
{
	/*for (auto itt = constraints.begin(); itt != constraints.end();) {
		(*itt)->cleanUp();
		delete* itt;
		itt = constraints.erase(itt);
	}*/
}
