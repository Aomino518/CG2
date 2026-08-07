#include "Emitter2DManager.h"
#include "Logger.h"

Emitter2DManager* Emitter2DManager::GetInstance()
{
    static Emitter2DManager instance;
    return &instance;
}

void Emitter2DManager::Update()
{
    for (auto& emitter : emitters_) {
        auto& it = emitter.second;
        it->Update();
    }
}

void Emitter2DManager::Shutdown()
{
    emitters_.clear();
}

void Emitter2DManager::CreateEmitter(const std::string& name, const ParticleConfig& config, uint32_t count, float frequency)
{
    // 同名がないかチェック
    if (emitters_.find(name) != emitters_.end()) {
        Logger::Write(Logger::LogLevel::Warning, "Emitter2D already exists: " + name);
        return;
    }

    std::unique_ptr<Particle2DEmitter> emitter = std::make_unique<Particle2DEmitter>(name, config, count, frequency);

    emitters_.emplace(name, std::move(emitter));
}

void Emitter2DManager::RemoveEmitter(const std::string& name)
{
    auto it = emitters_.find(name);
    if (it == emitters_.end()) {
        return;
    }

    emitters_.erase(it);

    Logger::Write("Emitter2D removed: " + name);
}

void Emitter2DManager::DrawDebug()
{
#ifdef _DEBUG
    for (auto& emitter : emitters_) {
        emitter.second->DrawDebug();
    }
#endif
}

void Emitter2DManager::DrawImgui(const std::string& name)
{
#ifdef _DEBUG
    auto it = emitters_.find(name);
    if (it != emitters_.end()) {
        it->second->DrawImGui();
    }
#endif
}

Particle2DEmitter* Emitter2DManager::GetEmitter(const std::string& name)
{
    auto it = emitters_.find(name);
    assert(it != emitters_.end() && "Emitter2D not found");
    return it->second.get();
}

std::vector<Particle2DEmitter*> Emitter2DManager::GetAllEmitters() const
{
    std::vector<Particle2DEmitter*> result;
    result.reserve(emitters_.size());
    for (const auto& emitter : emitters_) {
        result.push_back(emitter.second.get());
    }
    return result;
}

const std::unordered_map<std::string, std::unique_ptr<Particle2DEmitter>>& Emitter2DManager::GetEmitterMap() const
{
    return emitters_;
}

uint32_t Emitter2DManager::GetEmitterCount() const
{
    return static_cast<uint32_t>(emitters_.size());
}

