#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "GroupNode.h"

class TransformNode : public GroupNode
{
	glm::vec3 translation;
	glm::vec3 rotation;
	glm::vec3 scale;
	float angle;

	static glm::mat4 transformMatrix;

public:
	TransformNode(const std::string& name) : GroupNode(name)
	{
		type = nt_TransformNode;
		translation = glm::vec3(0.0f);
		rotation = glm::vec3(0.0f);
		scale = glm::vec3(1.0f);
	}

	void SetTranslation(const glm::vec3& tr)
	{
		translation = tr;
	}

	glm::vec3 getTranslation() {
		return translation;
	}

	void SetScale(const glm::vec3& sc)
	{
		scale = sc;
	}

	void SetRotation(float an, const glm::vec3& rt)
	{
		angle = an;
		rotation = rt;
	}

	void Traverse()
	{
		//push
		glm::mat4 matCopy = transformMatrix;

		
		
		transformMatrix = glm::translate(transformMatrix, translation);
		transformMatrix = glm::scale(transformMatrix, scale);
		transformMatrix = glm::rotate(transformMatrix, angle, rotation);
		for (unsigned int i = 0; i < children.size(); i++)
		{
			children[i]->Traverse();
		}

		//pop
		transformMatrix = matCopy;
	}

	virtual void TraverseIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, vector<Intersection*>& hits)
	{
		//push
		glm::mat4 matCopy = transformMatrix;
		//push path

		
		transformMatrix = glm::translate(transformMatrix, translation);
		transformMatrix = glm::scale(transformMatrix, scale);
		transformMatrix = glm::rotate(transformMatrix, angle, rotation);
		for (unsigned int i = 0; i < children.size(); i++)
		{
			children[i]->TraverseIntersection(rayOrigin, rayDirection, hits);
		}

		//pop
		transformMatrix = matCopy;

		//pop path

	}

	static const glm::mat4 GetTransformMatrix()
	{
		return transformMatrix;
	}
};