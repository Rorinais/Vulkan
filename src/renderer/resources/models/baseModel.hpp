#pragma once 
#include "../../../base.hpp"
#include "../../pipeline/materials/baseMaterial.hpp"
#include "mesh.hpp"

class BaseModel {
public:

private:
	std::vector<BaseMaterial> mBaseMaterial;

	std::vector<Mesh> mMesh;
};