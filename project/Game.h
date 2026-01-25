#pragma once
#include "SeekerEngine.h"
#include "SEFramework.h"
//#include "SceneManager.h"

class Game : public SEFramework
{
public:
	void Init() override;
	void Shutdown() override;
	void Update() override;
	void Draw() override;

private:
	//std::unique_ptr<SceneManager> sceneManager_ = nullptr;
};

