#include "stl.h"

#include <float.h>
#include <stdlib.h>
#include <string.h>

uint32_t stl_get_num_triangles(FILE *file)
{
	err_assert(fseek(file, sizeof(uint8_t[80]), SEEK_SET) == 0, ERR_STL_IO_FP);
	uint32_t num_triangles;
	err_assert(fread(&num_triangles, sizeof(uint32_t), 1, file) == 1, ERR_STL_IO_READ);
	return num_triangles;
}

//assumes that file is at SEEK_SET
Mesh *stl_load(OBJECT_INIT_PARAMS, FILE *file, Vec3 position, Vec3 rot, float scale)
{
	//ensure that file is binary instead of ascii
	char header[5];
	err_assert(fread(header, sizeof(char), 5, file) == 5, ERR_STL_IO_READ);
	err_assert(strncmp("solid", header, 5), ERR_STL_ENCODING);

	float a = cosf(rot[Z]) * sinf(rot[Y]);
	float b = sinf(rot[Z]) * sinf(rot[Y]);
	Vec3 rotation_matrix[3] = {
		{
			cosf(rot[Z]) * cosf(rot[Y]),
			a * sinf(rot[X]) - sinf(rot[Z]) * cosf(rot[X]),
			a * cosf(rot[X]) + sinf(rot[Z]) * sinf(rot[X])
		}, {
			sinf(rot[Z]) * cosf(rot[Y]),
			b * sinf(rot[X]) + cosf(rot[Z]) * cosf(rot[X]),
			b * cosf(rot[X]) - cosf(rot[Z]) * sinf(rot[X])
		}, {
			- sinf(rot[Y]),
			cosf(rot[Y]) * sinf(rot[X]),
			cosf(rot[Y]) * cosf(rot[X])
		}};

	uint32_t num_triangles = stl_get_num_triangles(file);
	Mesh *mesh = init_mesh(OBJECT_INIT_VARS, num_triangles);
	uint32_t i;
	for (i = 0; i < num_triangles; i++) {
		STLTriangle stl_triangle;
		err_assert(fread(&stl_triangle, sizeof(STLTriangle), 1, file) == 1, ERR_STL_IO_READ);

		unsigned j;
		for (j = 0; j < 3; j++) {
			Vec3 rotated_vertex = {
				dot3(rotation_matrix[X], stl_triangle.vertices[j]),
				dot3(rotation_matrix[Y], stl_triangle.vertices[j]),
				dot3(rotation_matrix[Z], stl_triangle.vertices[j])};
			multiply3(rotated_vertex, scale, stl_triangle.vertices[j]);
			add3(stl_triangle.vertices[j], position, stl_triangle.vertices[j]);
		}
		mesh_set_triangle(mesh, i, stl_triangle.vertices);
	}
	mesh_generate_bounding_cuboid(mesh);
	return mesh;
}
