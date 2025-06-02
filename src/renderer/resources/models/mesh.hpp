#pragma once 
#include "../../../base.hpp"

class Mesh {
public:

private:
	std::vector<glm::vec3> mPositions;
	std::vector<uint32_t> mIndices;

	uint32_t mMaterialIndex = 0;

};