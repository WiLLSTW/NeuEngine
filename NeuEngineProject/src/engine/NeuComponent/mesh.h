#pragma once
#include <vector>
#include "../NeuDataStructure.h"
#include "../NeuTools.h"
#include "../NeuVulkan/NeuVulkanTools.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Neu {
	class Mesh {
		std::vector<Vertex> vertices;
		std::vector<uint16_t> indicies;
		BufferSet vertexBuffer;
		BufferSet indexBuffer;



	public:
		Mesh(std::vector<Vertex>v, std::vector<uint16_t>i) {
			init(v, i);
		}
		Mesh() {};

		void init(std::vector<Vertex>v, std::vector<uint16_t>i) {

			this->vertices = v;
			this->indicies = i;

			createVertexBuffer();
			createIndexBuffer();
		}

		


		void createVertexBuffer() {

			auto device = getVulkanSet()->Device;
			VkDeviceSize bufferSize = sizeof(vertices[0])*vertices.size();
			BufferSet stagingBuffer;

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);

			void* data;
			vkMapMemory(device, stagingBuffer.Memory, 0, bufferSize, 0, &data);
			memcpy(data, vertices.data(), (size_t)bufferSize);
			vkUnmapMemory(device, stagingBuffer.Memory);

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer);

			copyBuffer(stagingBuffer.Buffer, vertexBuffer.Buffer, bufferSize);

			vkDestroyBuffer(device, stagingBuffer.Buffer, nullptr);
			vkFreeMemory(device, stagingBuffer.Memory, nullptr);

		}

		void createIndexBuffer() {
			auto device = getVulkanSet()->Device;
			VkDeviceSize bufferSize = sizeof(indicies[0])*indicies.size();
			BufferSet stagingBuffer;

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer);

			void* data;
			vkMapMemory(device, stagingBuffer.Memory, 0, bufferSize, 0, &data);
			memcpy(data, indicies.data(), (size_t)bufferSize);
			vkUnmapMemory(device, stagingBuffer.Memory);

			createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer);

			copyBuffer(stagingBuffer.Buffer, indexBuffer.Buffer, bufferSize);

			vkDestroyBuffer(device, stagingBuffer.Buffer, nullptr);
			vkFreeMemory(device, stagingBuffer.Memory, nullptr);
		}

		BufferSet *getVertexBuffer() {
			return &vertexBuffer;
		}

		BufferSet *getIndexBuffer() {
			return &indexBuffer;
		}
		
		size_t getIndexSize() {
			return indicies.size();
		}

		void drawCmd(VkCommandBuffer commandBuffer) {
			VkBuffer vertexBuffers[] = {vertexBuffer.Buffer };
			VkDeviceSize offsets[] = { 0 };

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
			vkCmdBindIndexBuffer(commandBuffer,indexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indicies.size()), 1, 0, 0, 0);

			addTriangleCnt(static_cast<uint32_t>(indicies.size() / 3));
		}

	};


	class Model {

		std::string directory;
		std::vector<Mesh> meshes;

	public:
		void init(const std::string src) {
			Assimp::Importer importer;
			const aiScene *scene = importer.ReadFile(src, aiProcess_Triangulate | aiProcess_FlipUVs|aiProcess_GenNormals);
			if (!scene || scene->mFlags&AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
				printf("Error:Assimp %s", importer.GetErrorString());
				return;
			}
			directory = src.substr(0, src.find_last_of('/'));
			
			processNode(scene->mRootNode, scene);

		}
		

		void processNode(aiNode *node, const aiScene *scene)
		{
			for (uint16_t i = 0; i < node->mNumMeshes; i++) {
				aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
				processMesh(mesh, scene);
			}
			for (uint16_t i = 0; i < node->mNumChildren; i++) {
				processNode(node->mChildren[i],scene);
			}
		}

		void processMesh(aiMesh *mesh, const aiScene *scene) {
			std::vector<Vertex> verticies;
			std::vector<uint16_t>indicies;
			for (uint16_t i = 0; i < mesh->mNumVertices; i++) {
				Vertex vertex;
				glm::vec3 vector;
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.pos = vector;

				glm::vec3 normal;
				normal.x = mesh->mNormals[i].x;
				normal.y = mesh->mNormals[i].y;
				normal.z = mesh->mNormals[i].z;
				vertex.normal = normal;

				if (mesh->mTextureCoords[0]) {
					glm::vec2 coord;
					coord.x = mesh->mTextureCoords[0][i].x;
					coord.y = mesh->mTextureCoords[0][i].y;
					vertex.texcoord = coord;
				}
				else {
					vertex.texcoord = glm::vec2(0.0f, 0.0f);
				}

				verticies.emplace_back(vertex);

			}

			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					indicies.emplace_back(face.mIndices[j]);
				}
			}

			Mesh me;
			me.init(verticies, indicies);
			meshes.emplace_back(me);
		}

		void drawCmd(VkCommandBuffer commandBuffer) {
			for (uint16_t i = 0; i < meshes.size(); i++) {
				meshes[i].drawCmd(commandBuffer);
			}

			
		}

	};

}