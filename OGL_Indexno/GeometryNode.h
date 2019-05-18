#pragma once

#include "Node.h"
#include "TransformNode.h"
#include "Model.h"
#include "BoundingObjects.h"
#include <glm\gtc\matrix_transform.hpp>

class GeometryNode : public Node
{
	Model model;
	Shader* shader;
	BoundingSphere* boundingSphere = NULL;

public:
	GeometryNode() :  Node()
	{
		type = nt_GeometryNode;
	}

	GeometryNode(const std::string& name) : Node(name, nt_GeometryNode)
	{

	}

	GeometryNode(const std::string& name, const std::string& path) : Node(name, nt_GeometryNode)
	{
		LoadFromFile(path);
	}

	~GeometryNode()
	{
		if (boundingSphere != NULL)
		{
			delete boundingSphere;
		}
	}

	void LoadFromFile(const std::string& path)
	{
		model.LoadModel(path);
		boundingSphere = new BoundingSphere(this, model);
	}

	const Model& GetModel() const
	{
		return model;
	}

	void SetShader(Shader* s)
	{
		shader = s;
	}

	const BoundingSphere& GetBoundingSphere()
	{
		return *boundingSphere;
	}

	void Traverse()
	{
		glm::mat4 transform = TransformNode::GetTransformMatrix();
		shader->setMat4("model", transform);
		glm::mat3 normalMat = glm::transpose(glm::inverse(transform));
		shader->setMat3("normalMat", normalMat);
		//boundingSphere->Transform(transform);
		model.Draw(*shader);
	}

	virtual void TraverseIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, vector<Intersection*>& hits)
	{
		Intersection* hit = new Intersection();

		glm::mat4 transform = TransformNode::GetTransformMatrix();
		boundingSphere->Transform(transform);

		if (boundingSphere->CollidesWithRay(rayOrigin, rayDirection, *hit))
		{
			hits.push_back(hit);
		}
	}
};
