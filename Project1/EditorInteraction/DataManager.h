#pragma once
#include <unordered_map>
#include "../Utils/General.h"

class GameScene;
class GameObject;
#define CC const Command&
#define CMD_RES const Command& command, Response& response
namespace Component {
	class ComponetBase;
	class Transform;
};
namespace EditorInteraction {
	class Command;
	class Response;
	class DataManager
	{
	private:
		static GameScene* scene;
		static short UID;
		static std::unordered_map<short, GameObject> gameObjects;
		static std::unordered_map<short, Component::ComponetBase*> components;
		static std::vector<short> transformUIDs;
		static short getUID();

		static bool resizeWindow(CMD_RES);

		// creation
		static bool createCommand(CMD_RES);
		static bool createGameObject(CMD_RES);
		static bool createComponent(CMD_RES);
		static bool createRenderMesh(CMD_RES);
		static bool createCamera(CMD_RES);

		// editing
		static bool editCommand(CMD_RES);
		static bool editComponent(CMD_RES);
		static bool editTransform(CMD_RES, Component::Transform* component);

		// calling functions
		static bool callFunction(CMD_RES);

		static void sendResponse(bool success, CMD_RES);
		static void clearBuffer();
	public:
		static void cleanUp();
		static void executeCommand(CC command);
		static void setScene(GameScene* scene);
	};
};