#pragma once
#include"../../core/base.hpp"

class Mesh{
public:
	Mesh() {}
	~Mesh() {}

	bool LoadMesh(const std::string& Filename);
	bool InitFromScene(const aiScene* pScene, const std::string& Filename);
	void InitMesh(unsigned int Index, const aiMesh* paiMesh);

	std::vector<glm::vec3> positions{};
	std::vector<glm::vec3> normals{};
	std::vector<glm::vec2> texCoords{};
	std::vector<uint32_t> indices{};
};


bool Mesh::LoadMesh(const std::string& Filename) {
    // 清空旧数据
    positions.clear();
    normals.clear();
    texCoords.clear();
    indices.clear();

    Assimp::Importer Importer;
    const aiScene* pScene = Importer.ReadFile(
        Filename.c_str(),
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs
    );

    if (!pScene) {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
        return false;
    }

    // 遍历场景中的所有网格
    for (unsigned int meshIndex = 0; meshIndex < pScene->mNumMeshes; meshIndex++) {
        const aiMesh* paiMesh = pScene->mMeshes[meshIndex];
        const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

        // 记录当前顶点起始位置
        unsigned int vertexStartIndex = positions.size();

        // 处理顶点数据
        for (unsigned int i = 0; i < paiMesh->mNumVertices; i++) {
            const aiVector3D* pPos = &(paiMesh->mVertices[i]);
            const aiVector3D* pNormal = &(paiMesh->mNormals[i]);
            const aiVector3D* pTexCoord = paiMesh->HasTextureCoords(0)
                ? &(paiMesh->mTextureCoords[0][i])
                : &Zero3D;

            positions.emplace_back(pPos->x, pPos->y, pPos->z);
            normals.emplace_back(pNormal->x, pNormal->y, pNormal->z);
            texCoords.emplace_back(pTexCoord->x, pTexCoord->y);
        }

        // 处理索引数据（需要偏移）
        for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
            const aiFace& face = paiMesh->mFaces[i];
            if (face.mNumIndices != 3) continue; // 跳过非三角形面

            indices.push_back(vertexStartIndex + face.mIndices[0]);
            indices.push_back(vertexStartIndex + face.mIndices[1]);
            indices.push_back(vertexStartIndex + face.mIndices[2]);
        }
    }

    return !positions.empty();
}



