#pragma once
#include <vector>
#include <string>
#include "../Rendering/Rendering.h"
using namespace std;

class Scene : Render::Rendering
{
private:
	vector<void*> loaded_objects;
public:
	Scene(const string requirments);
	~Scene();
};
