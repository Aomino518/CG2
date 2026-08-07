#pragma once
#include "SeekerEngine.h"
#include "SEFramework.h"

class Game : public SEFramework
{
public:
	void Init() override;
	void Shutdown() override;
	void Update() override;
	void Draw() override;

private:

};

