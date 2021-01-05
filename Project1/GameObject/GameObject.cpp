#include "GameObject.h"
#include "../Rendering/Rendering.h"

GameObject::GameObject() : componets(), enabled(), transform(DBG_NEW Component::Transform()) {
	// this->addComponet(DBG_NEW Component::Transform());
	enabled.push_back(1);
}

void GameObject::addComponet(Component::ComponetBase* componet)
{
	componets.push_back(componet);
	enabled.push_back(1);
	componet->setParent(this);
	if (componet->getType() == Component::Type::Collider) {
		Component::RigidBody* rb = getRigidbody();
		if (!rb)
			return;
		rb->addCollider(reinterpret_cast<Physics::Collider*>(componet));
	}
	else if (componet->getType() == Component::Type::Rigidbody) {
		Component::RigidBody* rb = reinterpret_cast<Component::RigidBody*>(componet);
		for (Component::ComponetBase* comp : componets) {
			if (comp->getType() == Component::Type::Collider) {
				rb->addCollider(reinterpret_cast<Physics::Collider*>(comp));
			}
		}
	}
}

void GameObject::tick(short currentTick)
{
	for (unsigned i = 0; i < componets.size(); i++) {
		if (!enabled[i] OR componets[i]->getType() == Component::Type::RenderMesh OR componets[i]->getType() == Component::Type::Rigidbody) continue;
		Component::ComponetBase*& comp = componets[i];
		comp->update();
		if (currentTick == FIXED_UPDATE_RATE) {
			comp->fixedUpdate();
		}
	}
}

void GameObject::tryDraw()
{
	for (unsigned i = 0; i < componets.size(); i++) {
		if (enabled[i] AND componets[i]->getType() == Component::Type::RenderMesh) {
			Component::ComponetBase*& comp = componets[i];
			comp->update();
		}
	}
}

Component::RigidBody* GameObject::getRigidbody()
{
	for (Component::ComponetBase* comp : componets) {
		if (comp->getType() == Component::Type::Rigidbody)
			return reinterpret_cast<Component::RigidBody*>(comp);
	}
	return nullptr;
}

template<class T>
T* GameObject::getComponet()
{
	for (Component::ComponetBase* componet : componets) {
		T* cast = dynamic_cast<T*>(componet);
		if (cast)
			return cast;
	}
}

void GameObject::cleanUp()
{
	for (auto itt = componets.begin(); itt != componets.end();) {
		(*itt)->cleanUp();
		// delete* itt;
		itt = componets.erase(itt);
	}
	/*for (Component::ComponetBase*& componet : componets) {
		componet->cleanUp();
		delete componet;
		componet = nullptr;
	}*/
	componets.clear();
	transform->cleanUp();
	delete transform;
	transform = nullptr;
}
