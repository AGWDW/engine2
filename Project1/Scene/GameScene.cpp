#include "GameScene.h"
#include "../Rendering/Shading/Manager.h"
#include "SkyBox.h"
#include  "../Context.h"
#include "../Rendering/Rendering.h"
#include "../Componets/Camera.h"
#include "../GameObject/Terrain.h"
#include "../GameObject/GameObject.h"
#include "../Utils/ResourceLoader.h"
#include "../Primatives/Buffers/FrameBuffer.h"
#include "../Primatives/Buffers/VertexBuffer.h"
#include "../Primatives/Buffers/UniformBuffer.h"
#include "../EventSystem/Handler.h"
#include "../Gizmos/GizmoRenderer.h"
#include "../UI/UIRenderer.h"
#include "../UI/Panes/Canvas.h"
#include "../ParticleSystem/ParticleEmmiter.h"
#include "../Componets/Lights/LightBase.h"


std::vector<GameEventsTypes> GameScene::getCurrentEvents() const
{
	std::vector<GameEventsTypes> res = {
		GameEventsTypes::Update
	};
	if (isFirstLoop) 
		res.insert(res.begin(), GameEventsTypes::Start);
	if (Events::Handler::buttonDown)
		res.push_back(GameEventsTypes::MouseToggle);
	if (Events::Handler::mouseMove)
		res.push_back(GameEventsTypes::MouseMove);
	if (Events::Handler::keyDown)
		res.push_back(GameEventsTypes::KeyToggle);
	return res;
}

GameScene::GameScene() : objects(), preProcessingLayers(), currentTick(0), postProcShaderId(0), FBOs(), backgroundColour(0),
							skybox(nullptr), mainContext(nullptr), opaque(), transparent(), mainCamera(nullptr), terrain(), 
							quadModel(), isFirstLoop(false), closing(false), uiStuff(), screenDimentions(0), emmiters(), lightSources(), USE_DEFFERED(1)
{
	quadModel = ResourceLoader::getModel("plane.dae");
}

void GameScene::drawOpaque()
{
	for (Component::RenderMesh* mesh : opaque) {
		if(mesh->getParent()->isAlive())
			mesh->render(mainContext->getTime().deltaTime);
	}
}

void GameScene::drawTransparent()
{
	/*glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);*/
	Render::Shading::Manager::setActive(ResourceLoader::getShader("TransparentShader")); 
	bindLights();
	assert(mainCamera);
	if (NOT transparent.size()) {
		return;
	}
	mainContext->enable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	std::map<float, Component::RenderMesh*> sorted;
	for (auto itt = transparent.rbegin(); itt != transparent.rend(); itt++) {
		float dist = (*itt).first;
		Component::RenderMesh* mesh = (*itt).second;
		if (mesh->getParent()->isAlive())
			mesh->render(mainContext->getTime().deltaTime);
		dist = glm::length2(mesh->getParent()->getTransform()->Position - mainCamera->getPos());
		sorted[dist] = mesh;
	}
	transparent.clear();
	transparent = sorted;
}

void GameScene::drawTerrain()
{
	for (Terrain* terrain : this->terrain) {
		terrain->draw(mainContext->getTime().deltaTime);
	}
}

void GameScene::drawUI()
{
	for (UI::Canvas* canvas : uiStuff) {
		canvas->render(mainContext->getTime().deltaTime);
	}
}

void GameScene::drawParticles()
{
	for (Component::ParticleEmmiter* part : emmiters) {
		part->update(mainContext->getTime().deltaTime);
	}
}

void GameScene::addObject(GameObject* obj)
{
	objects.push_back(obj);
	obj->raiseEvents({ GameEventsTypes::Awake }, 0);
	obj->setScene(this);

	obj->processComponets();
}

void GameScene::addTerrain(Terrain* terrain)
{
	this->terrain.push_back(terrain);
}

void GameScene::processComponet(Component::ComponetBase* comp)
{

	Component::RenderMesh* mesh = dynamic_cast<Component::RenderMesh*>(comp);
	if (mesh) {
		if (mesh->getTransparent()) {
			transparent[rand()] = mesh;
		}
		else {
			opaque.push_back(mesh);
		}
		return;
	}
	UI::Canvas* canv = dynamic_cast<UI::Canvas*>(comp);
	if (canv) {
		uiStuff.push_back(canv);
		return;
	}
	Component::ParticleEmmiter* part = dynamic_cast<Component::ParticleEmmiter*>(comp);
	if (part) {
		emmiters.push_back(part);
		return;
	}
	Component::LightBase* light = dynamic_cast<Component::LightBase*>(comp);
	if (light) {
		lightSources.push_back(light);
	}
}

void GameScene::setBG(Vector3 col) 
{ 
	backgroundColour = col; 
};

void GameScene::preProcess()
{
	for (const auto& layer : preProcessingLayers) {
		String name = layer.first;
		Unsigned shaderId = layer.second;
		Primative::Buffers::FrameBuffer& fbo = getFBO(name);
		fbo.bind();
		fbo.clearBits();
		Render::Shading::Manager::setActive(shaderId);

		if (name == "shadows" AND NOT USE_DEFFERED) {
			glCullFace(GL_FRONT);
			drawOpaque();
			glCullFace(GL_BACK);
		}
		else if(name == "G-Buffer")
		{
			drawOpaque();
			drawTerrain();
			drawParticles();
		}
		else if (name == "LightingPass") {
			int unit = 0;
			bindLights();
			Primative::Buffers::FrameBuffer& gBuffer = FBOs["G-Buffer"];
			gBuffer.activateColourTextures(unit, { "positionTex", "albedoTex", "normalTex", "MetRouAOTex" });

			const Primative::Buffers::VertexBuffer& buffer = ResourceLoader::getBuffer(quadModel.getBuffers()[0]);
			buffer.render();

			gBuffer.bind(GL_READ_FRAMEBUFFER);
			fbo.bind(GL_DRAW_FRAMEBUFFER);
			glBlitFramebuffer(0, 0, gBuffer.getDimentions().x, gBuffer.getDimentions().y, 0, 0, screenDimentions.x, screenDimentions.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST); 
			fbo.unBind();
			gBuffer.unBind(GL_READ_FRAMEBUFFER);
			fbo.unBind(GL_DRAW_FRAMEBUFFER);
			fbo.bind();


			drawSkyBox();
			drawTransparent();
			drawUI();
		}
		/*else if(NOT USE_DEFFERED) {
			drawOpaque();
			/*drawSkyBox();
			drawObjects(shaderId);
			drawUI();
			drawParticles();
		}*/
		fbo.unBind();
	}
}

void GameScene::postProcess()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // default fbo
	clearFBO();


	Render::Shading::Manager::setActive(postProcShaderId);
	
	glActiveTexture(GL_TEXTURE0);

	// if(USE_DEFFERED)
		glBindTexture(GL_TEXTURE_2D, FBOs["LightingPass"].getTexture("col0"));
	/*else
		glBindTexture(GL_TEXTURE_2D, FBOs["final"].getTexture("col0"));*/

	Render::Shading::Manager::setValue("tex", 0);
	
	const Primative::Buffers::VertexBuffer& buffer = ResourceLoader::getBuffer(quadModel.getBuffers()[0]);
	buffer.render();
}

void GameScene::updateObjects()
{
	const auto events = getCurrentEvents();
	for (GameObject*& obj : objects) {
		if (obj->isAlive()) {
			//obj->tick(currentTick++ % FIXED_UPDATE_RATE, mainContext->getTime().deltaTime);
			obj->raiseEvents(events,  mainContext->getTime().deltaTime);
		}
	}
}

void GameScene::clearFBO() const 
{
	glClearColor(backgroundColour.x, backgroundColour.y, backgroundColour.z, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screenDimentions.x, screenDimentions.y);
}

void GameScene::drawObjects(Unsigned shaderId)
{
	Render::Shading::Manager::setActive(shaderId);
	drawOpaque();
	drawTerrain();
	Render::Shading::Manager::setActive(shaderId);
	drawTransparent();
}

void GameScene::drawSkyBox()
{
	if (!skybox)
		return;
	mainContext->disable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	skybox->draw();
	glDepthFunc(GL_LESS);
	mainContext->enable(GL_CULL_FACE);
}

Primative::Buffers::FrameBuffer& GameScene::getFBO(const std::string& name)
{
	if (name == "any") {
		return (*FBOs.begin()).second;
	}
	const unsigned& s = FBOs.size();
	auto& r = FBOs[name];
	if (s < FBOs.size()) {
		FBOs.erase(name);
		return (*(FBOs.begin()++)).second;
	}
	return r;
}

void GameScene::initalize()
{
	Primative::Buffers::FrameBuffer g_buffer_FBO({ "col0", "col1", "col2", "col3" }, screenDimentions, { 0, 0, 0 });
	Primative::Buffers::FrameBuffer lighting_FBO({ "col0" }, screenDimentions, { 0, 0, 0 });

	addFBO("G-Buffer", g_buffer_FBO);
	addPreProcLayer("G-Buffer", ResourceLoader::getShader("DeferredOpaque"));

	addFBO("LightingPass", lighting_FBO);
	addPreProcLayer("LightingPass", ResourceLoader::getShader("DeferredFinal"));
	
	/*if(NOT USE_DEFFERED OR true) {
		Primative::Buffers::FrameBuffer shadowFBO({ "depth" }, screenDimentions, { 1, 0, 1 });
		Primative::Buffers::FrameBuffer finalFBO({ "col0" }, screenDimentions, { 0, 0, 0 });
		addFBO("shadows", shadowFBO);
		addPreProcLayer("shadows", ResourceLoader::getShader("ShadowShader"));

		addFBO("final", finalFBO);
		addPreProcLayer("final", ResourceLoader::getShader("PBRShader"));
	}*/

	setPostProcShader(ResourceLoader::getShader("PostShader")); // PBRShader  PostShader
	


	Primative::Buffers::StaticBuffer mainBuffer("m4, m4, v3, f", 0);
	// view    | matrix 4
	// proj    | matrix 4
	// viewPos | vector 3
	// gamma   | scalar f

	glm::mat4 projection = glm::perspective(glm::radians(mainCamera->getFOV()), static_cast<float>(screenDimentions.x) / static_cast<float>(screenDimentions.y), 0.01f, 5000.0f);

	mainBuffer.fill(1, glm::value_ptr(projection));
	float gamma = 2.2f;
	mainBuffer.fill(3, &gamma);

	Primative::Buffers::StaticBuffer lsUBO("m4", 1);
	// lspaceM | matrix 4

	glm::mat4 lightProjection = glm::ortho<float>(-10, 10, -10, 10, 1, 50);
	glm::mat4 lightView = glm::lookAt(glm::vec3(2), glm::vec3(0), glm::vec3(0, 1, 0));
	lightProjection *= lightView;
	lsUBO.fill(0, glm::value_ptr(lightProjection));

	uniformBuffers.push_back(mainBuffer);
	uniformBuffers.push_back(lsUBO);
}

void GameScene::gameLoop()
{
	// float counter = 0;
	isFirstLoop = true;
	while (NOT (closing OR mainContext->shouldClose()))
	{
		// FPS--------------------------------------------------------------------------------------------------------------------------------------------------

		// tb.setText("FPS: " + std::to_string(main.getTime().avgFPS));
		// const float w = tb.getWidth() / 2.0f;
		// const glm::vec2& p = tb.getPos();
		// float diff = p.x - w; // left edge
		// diff = 5 - diff;
		// tb.setPos({ p.x + diff, p.y });

		// UPDATES----------------------------------------------------------------------------------------------------------------------------------------------
		updateObjects();

		// counter += main.getTime().deltaTime;

		// RENDERING--------------------------------------------------------------------------------------------------------------------------------------------
		auto& mainBuffer = uniformBuffers[0];
		mainBuffer.fill(0, glm::value_ptr(mainCamera->getView()));
		mainBuffer.fill(2, glm::value_ptr(mainCamera->getPos()));

		preProcess(); // shadows and scene quad
		postProcess();// render to screen
		Gizmos::GizmoRenderer::drawAll();

		// sound->update();

		mainContext->update();
		Events::Handler::pollEvents();
		isFirstLoop = false;
	}
}

void GameScene::cleanUp()
{
	for (auto& obj : objects) {
		obj->cleanUp();
		//delete obj;
		obj = nullptr;
	}
	objects.clear();
	for (auto& pair : FBOs) {
		Primative::Buffers::FrameBuffer& fbo = pair.second;
		fbo.cleanUp();
	}
	for (auto itt = uniformBuffers.begin(); itt != uniformBuffers.end();) {
		(*itt).cleanUp();
		itt = uniformBuffers.erase(itt);
	}
	for (auto itt = terrain.begin(); itt != terrain.end();) {
		(*itt)->cleanUp();
		itt = terrain.erase(itt);
	}
	for (auto itt = uiStuff.begin(); itt != uiStuff.end();) {
		(*itt)->cleanUp();
		itt = uiStuff.erase(itt);
	}
	for (auto itt = emmiters.begin(); itt != emmiters.end();) {
		(*itt)->cleanUp();
		itt = emmiters.erase(itt);
	}
	for (auto itt = lightSources.begin(); itt != lightSources.end();) {
		(*itt)->cleanUp();
		itt = lightSources.erase(itt);
	}

	FBOs.clear();
	if(skybox)
		skybox->cleanUp();
	skybox = nullptr;
	mainCamera = nullptr;
	opaque.clear();
	transparent.clear();
	quadModel.cleanUp();
	mainContext->cleanUp();
}

void GameScene::close()
{
	closing = true;
}

void GameScene::bindLights()
{
	int directional = 0;
	int point = 0;
	int spot = 0;
	for (Component::LightBase* light : lightSources) {
		int count = -1;
		std::string type = "";
		switch (light->getLightType())
		{
		case Component::LightType::Directional:
			count = directional++;
			type = "directional";
			break;
		case Component::LightType::Point:
			count = point++;
			type = "point";
			break;
		case Component::LightType::Spot:
			count = spot++;
			type = "spot";
			break;
		}
		Render::Shading::Manager::setValue(type, light, count);
	}
	Render::Shading::Manager::setValue("numberOfDirectionalLights", directional);
	Render::Shading::Manager::setValue("numberOfPointLights", point);
	Render::Shading::Manager::setValue("numberOfSpotLights", spot);
}

void GameScene::addPreProcLayer(const std::string& name, const unsigned& shaderId)
{
	this->preProcessingLayers[name] = shaderId;	
}

void GameScene::addFBO(const std::string& layerName, Primative::Buffers::FrameBuffer fbo)
{
	FBOs.insert({ layerName, fbo });
};

void GameScene::setPostProcShader(const unsigned& shaderId)
{
	postProcShaderId = shaderId;
};

void GameScene::setSkyBox(SkyBox* sb)
{
	skybox = sb;
}

void GameScene::setContext(Context* context)
{
	mainContext = context;
	screenDimentions = context->getDimentions();
}

void GameScene::setMainCamera(Component::Camera* camera)
{
	mainCamera = camera;
}

const glm::ivec2& GameScene::getScreenDimentions() const
{
	return screenDimentions;
}

GameObject* GameScene::getObject(String name)
{
	for (GameObject* obj : objects) {
		if (obj->getName() == name) {
			return obj;
		}
	}
	return nullptr;
}
