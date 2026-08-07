#include "EmitterManager.h"

EmitterManager* EmitterManager::GetInstance()
{
	static EmitterManager instance;
	return &instance;
}

void EmitterManager::Update() {
	for (auto& emitter : emitters_) {
		auto& it = emitter.second;
		it->Update();
	}
}

void EmitterManager::Shutdown()
{
	emitters_.clear();
}

void EmitterManager::CreateEmitter(const std::string& name, const ParticleConfig& config, uint32_t count, float frequency)
{
	// 同名がないかチェック
	if (emitters_.find(name) != emitters_.end()) {
		Logger::Write(Logger::LogLevel::Warning, "Emitter already exists: " + name);
		return;
	}

	std::unique_ptr<ParticleEmitter> emitter = std::make_unique<ParticleEmitter>(name, config, count, frequency);
	
	emitters_.emplace(name, std::move(emitter));
}

void EmitterManager::RemoveEmitter(const std::string& name)
{
	auto it = emitters_.find(name);
	if (it == emitters_.end()) {
		return;
	}

	emitters_.erase(it);

	Logger::Write("Emitter removed: " + name);
}

ParticleEmitter* EmitterManager::GetEmitter(const std::string& name)
{
	auto it = emitters_.find(name);
	assert(it != emitters_.end() && "Emitter not found");
	return it->second.get();
}

std::vector<ParticleEmitter*> EmitterManager::GetAllEmitters() const
{
	std::vector<ParticleEmitter*> result;
	result.reserve(emitters_.size());
	for (const auto& emitter : emitters_) {
		result.push_back(emitter.second.get());
	}
	return result;
}

const std::unordered_map<std::string, std::unique_ptr<ParticleEmitter>>& EmitterManager::GetEmitterMap() const
{
	return emitters_;
}

uint32_t EmitterManager::GetEmitterCount() const
{
	return static_cast<uint32_t>(emitters_.size());
}

void EmitterManager::DrawDebug() {
#ifdef _DEBUG
	for (auto& emitter : emitters_) {
		emitter.second->DrawDebug();
	}
#endif
}

void EmitterManager::DrawImgui(const std::string& name)
{
#ifdef _DEBUG
	auto it = emitters_.find(name);
	if (it != emitters_.end()) {
		it->second->DrawImGui();
	}
#endif
}
