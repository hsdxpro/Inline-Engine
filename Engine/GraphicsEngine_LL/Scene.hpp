#pragma once

#include "EntityCollection.hpp"
#include <string>

namespace inl {
namespace gxeng {

class MeshEntity;
class TerrainEntity;
class DirectionalLight;
class GraphicsEngine;


class Scene {
public:
	Scene() = default;
	Scene(std::string name);
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	virtual ~Scene();

	void SetName(std::string name);
	const std::string& GetName() const;
		
	EntityCollection<MeshEntity>& GetMeshEntities();
	const EntityCollection<MeshEntity>& GetMeshEntities() const;

	void SetSun(DirectionalLight* sun);
	const DirectionalLight& GetSun() const;

	//EntityCollection<TerrainEntity>& GetTerrainEntities();
	//const EntityCollection<TerrainEntity>& GetTerrainEntities() const;

	//EntityCollection<Light>& GetLights();
	//const EntityCollection<Light>& GetLights() const;

private:
	EntityCollection<MeshEntity> m_meshEntities;	
	DirectionalLight* m_sun = nullptr;
	//EntityCollection<TerrainEntity> m_terrainEntities;
	//EntityCollection<Light> m_lights;

	std::string m_name;
};



} // namespace gxeng
} // namespace inl
