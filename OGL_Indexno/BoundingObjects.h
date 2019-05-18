#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
//#include "Node.h"

class Node;
class GeometryNode;

struct Intersection
{
	//the intersection point in world space
	glm::vec3 point;
	//the distance between the the intersection ray origin and the intersection point
	float distance;
	GeometryNode* intersectedNode;
	vector<Node*> Route;

	//GeometryNode* Route;
};

class IBoundingVolume
{
public:
	virtual bool CollidesWithRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, Intersection& intersection) = 0;
};

class BoundingSphere : public IBoundingVolume
{
	//local center
	glm::vec3 center;
	float radius;
	//center in world coordinates after model transformation
	glm::vec3 worldCenter;
	//the radius after applied model scale transformations
	float worldRadius;

protected:
	//the node the bounding volume belongs to
	GeometryNode* node;

public:
	BoundingSphere(GeometryNode* gn, const Model& model)
	{
		node = gn;
		float xmin, ymin, zmin, xmax, ymax, zmax;
		xmin = xmax = model.meshes[0].vertices[0].Position.x;
		ymin = ymax = model.meshes[0].vertices[0].Position.y;
		zmin = zmax = model.meshes[0].vertices[0].Position.z;
		for (unsigned int m = 0; m < model.meshes.size(); m++)
		{
			const Mesh& mesh = model.meshes[m];
			for (unsigned int i = 1; i < mesh.vertices.size(); i++) //popalva masiva s vertexite
			{
				if (mesh.vertices[i].Position.x < xmin)
					xmin = mesh.vertices[i].Position.x;
				if (mesh.vertices[i].Position.x > xmax)
					xmax = mesh.vertices[i].Position.x;
				if (mesh.vertices[i].Position.y < ymin)
					ymin = mesh.vertices[i].Position.y;
				if (mesh.vertices[i].Position.y > ymax)
					ymax = mesh.vertices[i].Position.y;
				if (mesh.vertices[i].Position.z < zmin)
					zmin = mesh.vertices[i].Position.z;
				if (mesh.vertices[i].Position.z > zmax)
					zmax = mesh.vertices[i].Position.z;
			}
		}
		center.x = (xmin + xmax) / 2;
		center.y = (ymin + ymax) / 2;
		center.z = (zmin + zmax) / 2;
		radius = 0;
		for (unsigned int m = 0; m < model.meshes.size(); m++)
		{
			const Mesh& mesh = model.meshes[m];
			for (unsigned int i = 0; i < mesh.vertices.size(); i++)
			{
				float r = glm::length(mesh.vertices[i].Position - center);
				if (r > radius)
					radius = r;
			}
		}
	}

	const glm::vec3& GetCenter() const
	{
		return center;
	}

	const glm::vec3 GetWorldCenter() const
	{
		return worldCenter;
	}

	const float GetRadius() const
	{
		return radius;
	}

	void Transform(const glm::mat4& model)
	{
		worldCenter = model * glm::vec4(center, 1);

		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;

		glm::decompose(model, scale, rotation, translation, skew, perspective);
		//multiply the radius with the largest scale (in case of non-uniform scale)
		if (scale.x > scale.y && scale.x > scale.z)
		{
			worldRadius = radius * scale.x;
		}
		else if (scale.y > scale.x && scale.y > scale.z)
		{
			worldRadius = radius * scale.y;
		}
		else
		{
			worldRadius = radius * scale.z;
		}
	}

	bool CollidesWithRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, Intersection& hit)
	{
		glm::vec3 originToCenter = worldCenter - rayOrigin;
		float radiusSqr = worldRadius * worldRadius;
		float lengthOCSqr = pow(glm::length(originToCenter), 2);
		bool  startsOutside = lengthOCSqr > radiusSqr;
		if (!startsOutside)
		{
			return false; //don't handle intersections from inside the bounding sphere
		}

		// 'distance' from ray origin to the perpendicular from the sphere's center to the ray
		float dLen = glm::length(rayDirection);
		float distOF = glm::dot(originToCenter, rayDirection) / dLen;

		// ray starts outside the sphere
		if (startsOutside && distOF < 0)
			return false;

		// 'distance' between the perpendicular and the intersection
		float distFS = radiusSqr - (lengthOCSqr - distOF*distOF);

		// the ray doesn't intersect the sphere
		if (distFS < 0)
			return false;

		// calculate the parameter for the line equation
		float t;
		t = (distOF - (float)sqrt(distFS)) / dLen; //this is the nearer intersection from the two solutions

		hit.point = rayOrigin + rayDirection * t;
		hit.intersectedNode = node;
		hit.distance = t;

		//hit.Route.push_back(node);
		//hit.Route = node;

	}



};
