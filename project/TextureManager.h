#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <vector>
#include <cassert>
#include <memory>
#include "Logger.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

class TextureManager
{
public:
	static void Init();

	static void Shutdown();

private:
	
};

