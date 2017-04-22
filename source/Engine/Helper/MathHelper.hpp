#pragma once

string to_string(vec3 in);

struct Plane
{
	Plane()
	{
		n = glm::vec3(0, 1, 0);
		d = glm::vec3(0, 0, 0);
	}
	Plane(glm::vec3 normal, glm::vec3 det)
	{
		n = normal;
		d = det;
	}
	Plane(glm::vec3 a, glm::vec3 b, glm::vec3 c)
	{
		d = a;
		n = glm::normalize(glm::cross(-b + a, c - a));
	}
	glm::vec3 n;
	glm::vec3 d;
};
struct Sphere
{
	Sphere()
	{
		radius = 1;
		pos = glm::vec3(0, 0, 0);
	}
	Sphere(glm::vec3 position, float size)
	{
		pos = position;
		radius = size;
	}
	glm::vec3 pos;
	float radius;
};