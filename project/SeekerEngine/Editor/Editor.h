#pragma once
#include <stdlib.h>
#include <string>
#include <memory>
#include <unordered_map>

enum class InspectorCategory {
    None,
    Sprite,
    Model,
    Particle,
    Particle2D,
    Camera,
    DirectionalLight,
    PointLight,
    SpotLight,
    WorldField,
    WorldField2D
};

struct InspectorSelection {
    InspectorCategory category = InspectorCategory::None;
    std::string name;
};

class Sprite;
class Entity3D;
class ParticleEmitter;
class Particle2DEmitter;
class Editor
{
public:
    static Editor* GetInstance();

	void Draw();

    // 登録
    void RegisterSprite(const std::string& name, Sprite* sprite);
    void RegisterModel(const std::string& name, Entity3D* model);
    void RegisterParticle(const std::string& name);
    void RegisterParticle2D(const std::string& name);

    // 保存と読み込み
    void SaveSceneJson(const std::string& path) const;
    void LoadSceneJson(const std::string& path);
    void ClearSceneJson(const std::string& path);

    void Clear();

private:
    Editor() = default;
    ~Editor() = default;
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

	void DrawHierarchy();
	void DrawInspector();

    InspectorSelection selection_;

    std::unordered_map<std::string, Sprite*> sprites_;
    std::unordered_map<std::string, Entity3D*> models_;
    std::unordered_map<std::string, ParticleEmitter*> particles_;
    std::unordered_map<std::string, Particle2DEmitter*> particles2D_;
};