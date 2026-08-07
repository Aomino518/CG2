#pragma once
#include "SeekerEngine.h"
#include "AbstractSceneFactory.h"

class SEFramework
{
public:
	virtual void Init();
	virtual void Shutdown();
	virtual void Update();
	virtual void Draw() = 0;
	virtual bool IsEndRequst() { return endRequst_; }
	virtual ~SEFramework() = default;

	void Run();

private:
	bool endRequst_ = false;
	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;
protected:
	SeekerEngine engine_;
	void SetSceneFactory(std::unique_ptr<AbstractSceneFactory> sceneFactory);
};

