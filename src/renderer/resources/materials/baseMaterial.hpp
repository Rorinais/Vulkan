#pragma once 
#include "../../../base.hpp"
#include "../../resources/textures/Texture.hpp"
#include "../shaders/baseShader.hpp"
#include "../pipelineStates/pipeline.hpp"

class BaseMaterial {
public:
	void addTexture(){}
	void setVertexShader(){}
	void setFragmentShader(){}
private:
	std::vector<Texture::Ptr> mTextures;
	BaseShader mBaseShader;
	Pipeline::Config mPipelineStageConfig;
	uint32_t mMaterialIndex = 0;

	bool IsDepth = true;
	std::string mMaterialName = "DefualtMaterial";
};