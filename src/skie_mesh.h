#include <glm/vec3.hpp>
#include "../utils/vkutils.h"
#include "vector"
#pragma once
namespace sk
{
	struct VertexInputDescription
	{
		std::vector<vk::VertexInputBindingDescription> bindings;
		std::vector<vk::VertexInputAttributeDescription> attributes;


	};
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec3 color;

		static VertexInputDescription get_vertex_description();
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;

		std::unique_ptr<VkAllocatedBuffer> vertex_buffer;
		bool from_obj(const char* filename);
	};

	class skie_mesh
	{

	};

}

