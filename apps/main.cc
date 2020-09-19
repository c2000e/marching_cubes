/*
	Marching Cubes Implementation - Christian Ermann
	Version 1.0  May 30, 2020

	A simple implementation of marching cubes to be used as a reference for more complex implementations or
	implementations embedded within other algorithms.

	Based on the implementation at http://paulbourke.net/geometry/polygonise/.
*/

#include "marching_cubes/shader.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "SDL.h"
#include "GL/glew.h"
#include "SDL_opengl.h"

#include <iostream>
#include <array>
#include <vector>

// OpenGL window size
const GLint WIDTH = 512, HEIGHT = 512;

// Parameters for the marching cubes algorithm.
const glm::vec3 MIN = glm::vec3(-1.0f, -1.0f, -1.0f);
const glm::vec3 MAX = glm::vec3(1.0f, 1.0f, 1.0f);
const int RESOLUTION = 10;

// Lookup tables taken from http://paulbourke.net/geometry/polygonise/.
const std::array<int, 256> edgeTable = {
		0x0  , 0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c,
		0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09, 0xf00,
		0x190, 0x99 , 0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c,
		0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93, 0xf99, 0xe90,
		0x230, 0x339, 0x33 , 0x13a, 0x636, 0x73f, 0x435, 0x53c,
		0xa3c, 0xb35, 0x83f, 0x936, 0xe3a, 0xf33, 0xc39, 0xd30,
		0x3a0, 0x2a9, 0x1a3, 0xaa , 0x7a6, 0x6af, 0x5a5, 0x4ac,
		0xbac, 0xaa5, 0x9af, 0x8a6, 0xfaa, 0xea3, 0xda9, 0xca0,
		0x460, 0x569, 0x663, 0x76a, 0x66 , 0x16f, 0x265, 0x36c,
		0xc6c, 0xd65, 0xe6f, 0xf66, 0x86a, 0x963, 0xa69, 0xb60,
		0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff , 0x3f5, 0x2fc,
		0xdfc, 0xcf5, 0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0,
		0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55 , 0x15c,
		0xe5c, 0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950,
		0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc ,
		0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0,
		0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5, 0xfcc,
		0xcc , 0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0,
		0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f, 0xf55, 0xe5c,
		0x15c, 0x55 , 0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650,
		0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6, 0xfff, 0xcf5, 0xdfc,
		0x2fc, 0x3f5, 0xff , 0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0,
		0xb60, 0xa69, 0x963, 0x86a, 0xf66, 0xe6f, 0xd65, 0xc6c,
		0x36c, 0x265, 0x16f, 0x66 , 0x76a, 0x663, 0x569, 0x460,
		0xca0, 0xda9, 0xea3, 0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac,
		0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa , 0x1a3, 0x2a9, 0x3a0,
		0xd30, 0xc39, 0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c,
		0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33 , 0x339, 0x230,
		0xe90, 0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c,
		0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99 , 0x190,
		0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c,
		0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109, 0x0 };

const std::array<std::array<int, 16>, 256> triTable = {{
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  8,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  8,  3,  9,  8,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  8,  3,  1,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  2, 10,  0,  2,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  8,  3,  2, 10,  8, 10,  9,  8, -1, -1, -1, -1, -1, -1, -1},
	{ 3, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0, 11,  2,  8, 11,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  9,  0,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1, 11,  2,  1,  9, 11,  9,  8, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 3, 10,  1, 11, 10,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0, 10,  1,  0,  8, 10,  8, 11, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  9,  0,  3, 11,  9, 11, 10,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  8, 10, 10,  8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  3,  0,  7,  3,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  1,  9,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  1,  9,  4,  7,  1,  7,  3,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  2, 10,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  4,  7,  3,  0,  4,  1,  2, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  2, 10,  9,  0,  2,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 2, 10,  9,  2,  9,  7,  2,  7,  3,  7,  9,  4, -1, -1, -1, -1},
	{ 8,  4,  7,  3, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{11,  4,  7, 11,  2,  4,  2,  0,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  0,  1,  8,  4,  7,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  7, 11,  9,  4, 11,  9, 11,  2,  9,  2,  1, -1, -1, -1, -1},
	{ 3, 10,  1,  3, 11, 10,  7,  8,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 1, 11, 10,  1,  4, 11,  1,  0,  4,  7, 11,  4, -1, -1, -1, -1},
	{ 4,  7,  8,  9,  0, 11,  9, 11, 10, 11,  0,  3, -1, -1, -1, -1},
	{ 4,  7, 11,  4, 11,  9,  9, 11, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  5,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  5,  4,  0,  8,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  5,  4,  1,  5,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  5,  4,  8,  3,  5,  3,  1,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  2, 10,  9,  5,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  0,  8,  1,  2, 10,  4,  9,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  2, 10,  5,  4,  2,  4,  0,  2, -1, -1, -1, -1, -1, -1, -1},
	{ 2, 10,  5,  3,  2,  5,  3,  5,  4,  3,  4,  8, -1, -1, -1, -1},
	{ 9,  5,  4,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0, 11,  2,  0,  8, 11,  4,  9,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  5,  4,  0,  1,  5,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  1,  5,  2,  5,  8,  2,  8, 11,  4,  8,  5, -1, -1, -1, -1},
	{10,  3, 11, 10,  1,  3,  9,  5,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  9,  5,  0,  8,  1,  8, 10,  1,  8, 11, 10, -1, -1, -1, -1},
	{ 5,  4,  0,  5,  0, 11,  5, 11, 10, 11,  0,  3, -1, -1, -1, -1},
	{ 5,  4,  8,  5,  8, 10, 10,  8, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  7,  8,  5,  7,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  3,  0,  9,  5,  3,  5,  7,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  7,  8,  0,  1,  7,  1,  5,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  5,  3,  3,  5,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  7,  8,  9,  5,  7, 10,  1,  2, -1, -1, -1, -1, -1, -1, -1},
	{10,  1,  2,  9,  5,  0,  5,  3,  0,  5,  7,  3, -1, -1, -1, -1},
	{ 8,  0,  2,  8,  2,  5,  8,  5,  7, 10,  5,  2, -1, -1, -1, -1},
	{ 2, 10,  5,  2,  5,  3,  3,  5,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  9,  5,  7,  8,  9,  3, 11,  2, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  5,  7,  9,  7,  2,  9,  2,  0,  2,  7, 11, -1, -1, -1, -1},
	{ 2,  3, 11,  0,  1,  8,  1,  7,  8,  1,  5,  7, -1, -1, -1, -1},
	{11,  2,  1, 11,  1,  7,  7,  1,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  5,  8,  8,  5,  7, 10,  1,  3, 10,  3, 11, -1, -1, -1, -1},
	{ 5,  7,  0,  5,  0,  9,  7, 11,  0,  1,  0, 10, 11, 10,  0, -1},
	{11, 10,  0, 11,  0,  3, 10,  5,  0,  8,  0,  7,  5,  7,  0, -1},
	{11, 10,  5,  7, 11,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{10,  6,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  8,  3,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  0,  1,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  8,  3,  1,  9,  8,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  6,  5,  2,  6,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  6,  5,  1,  2,  6,  3,  0,  8, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  6,  5,  9,  0,  6,  0,  2,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  9,  8,  5,  8,  2,  5,  2,  6,  3,  2,  8, -1, -1, -1, -1},
	{ 2,  3, 11, 10,  6,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{11,  0,  8, 11,  2,  0, 10,  6,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  1,  9,  2,  3, 11,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 5, 10,  6,  1,  9,  2,  9, 11,  2,  9,  8, 11, -1, -1, -1, -1},
	{ 6,  3, 11,  6,  5,  3,  5,  1,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  8, 11,  0, 11,  5,  0,  5,  1,  5, 11,  6, -1, -1, -1, -1},
	{ 3, 11,  6,  0,  3,  6,  0,  6,  5,  0,  5,  9, -1, -1, -1, -1},
	{ 6,  5,  9,  6,  9, 11, 11,  9,  8, -1, -1, -1, -1, -1, -1, -1},
	{ 5, 10,  6,  4,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  3,  0,  4,  7,  3,  6,  5, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  9,  0,  5, 10,  6,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1},
	{10,  6,  5,  1,  9,  7,  1,  7,  3,  7,  9,  4, -1, -1, -1, -1},
	{ 6,  1,  2,  6,  5,  1,  4,  7,  8, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  2,  5,  5,  2,  6,  3,  0,  4,  3,  4,  7, -1, -1, -1, -1},
	{ 8,  4,  7,  9,  0,  5,  0,  6,  5,  0,  2,  6, -1, -1, -1, -1},
	{ 7,  3,  9,  7,  9,  4,  3,  2,  9,  5,  9,  6,  2,  6,  9, -1},
	{ 3, 11,  2,  7,  8,  4, 10,  6,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 5, 10,  6,  4,  7,  2,  4,  2,  0,  2,  7, 11, -1, -1, -1, -1},
	{ 0,  1,  9,  4,  7,  8,  2,  3, 11,  5, 10,  6, -1, -1, -1, -1},
	{ 9,  2,  1,  9, 11,  2,  9,  4, 11,  7, 11,  4,  5, 10,  6, -1},
	{ 8,  4,  7,  3, 11,  5,  3,  5,  1,  5, 11,  6, -1, -1, -1, -1},
	{ 5,  1, 11,  5, 11,  6,  1,  0, 11,  7, 11,  4,  0,  4, 11, -1},
	{ 0,  5,  9,  0,  6,  5,  0,  3,  6, 11,  6,  3,  8,  4,  7, -1},
	{ 6,  5,  9,  6,  9, 11,  4,  7,  9,  7, 11,  9, -1, -1, -1, -1},
	{10,  4,  9,  6,  4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4, 10,  6,  4,  9, 10,  0,  8,  3, -1, -1, -1, -1, -1, -1, -1},
	{10,  0,  1, 10,  6,  0,  6,  4,  0, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  3,  1,  8,  1,  6,  8,  6,  4,  6,  1, 10, -1, -1, -1, -1},
	{ 1,  4,  9,  1,  2,  4,  2,  6,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  0,  8,  1,  2,  9,  2,  4,  9,  2,  6,  4, -1, -1, -1, -1},
	{ 0,  2,  4,  4,  2,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  3,  2,  8,  2,  4,  4,  2,  6, -1, -1, -1, -1, -1, -1, -1},
	{10,  4,  9, 10,  6,  4, 11,  2,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  8,  2,  2,  8, 11,  4,  9, 10,  4, 10,  6, -1, -1, -1, -1},
	{ 3, 11,  2,  0,  1,  6,  0,  6,  4,  6,  1, 10, -1, -1, -1, -1},
	{ 6,  4,  1,  6,  1, 10,  4,  8,  1,  2,  1, 11,  8, 11,  1, -1},
	{ 9,  6,  4,  9,  3,  6,  9,  1,  3, 11,  6,  3, -1, -1, -1, -1},
	{ 8, 11,  1,  8,  1,  0, 11,  6,  1,  9,  1,  4,  6,  4,  1, -1},
	{ 3, 11,  6,  3,  6,  0,  0,  6,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 6,  4,  8, 11,  6,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 7, 10,  6,  7,  8, 10,  8,  9, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  7,  3,  0, 10,  7,  0,  9, 10,  6,  7, 10, -1, -1, -1, -1},
	{10,  6,  7,  1, 10,  7,  1,  7,  8,  1,  8,  0, -1, -1, -1, -1},
	{10,  6,  7,  10, 7,  1,  1,  7,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  2,  6,  1,  6,  8,  1,  8,  9,  8,  6,  7, -1, -1, -1, -1},
	{ 2,  6,  9,  2,  9,  1,  6,  7,  9,  0,  9,  3,  7,  3,  9, -1},
	{ 7,  8,  0,  7,  0,  6,  6,  0,  2, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  3,  2,  6,  7,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  3, 11, 10,  6,  8, 10,  8,  9,  8,  6,  7, -1, -1, -1, -1},
	{ 2,  0,  7,  2,  7, 11,  0,  9,  7,  6,  7, 10,  9, 10,  7, -1},
	{ 1,  8,  0,  1,  7,  8,  1, 10,  7,  6,  7, 10,  2,  3, 11, -1},
	{11,  2,  1, 11,  1,  7, 10,  6,  1,  6,  7,  1, -1, -1, -1, -1},
	{ 8,  9,  6,  8,  6,  7,  9,  1,  6, 11,  6,  3,  1,  3,  6, -1},
	{ 0,  9,  1, 11,  6,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  8,  0,  7,  0,  6,  3, 11,  0, 11,  6,  0, -1, -1, -1, -1},
	{ 7, 11,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  0,  8, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  1,  9, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  1,  9,  8,  3,  1, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1},
	{10,  1,  2,  6, 11,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  2, 10,  3,  0,  8,  6, 11,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  9,  0,  2, 10,  9,  6, 11,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 6, 11,  7,  2, 10,  3, 10,  8,  3, 10,  9,  8, -1, -1, -1, -1},
	{ 7,  2,  3,  6,  2,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 7,  0,  8,  7,  6,  0,  6,  2,  0, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  7,  6,  2,  3,  7,  0,  1,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  6,  2,  1,  8,  6,  1,  9,  8,  8,  7,  6, -1, -1, -1, -1},
	{10,  7,  6,  10, 1,  7,  1,  3,  7, -1, -1, -1, -1, -1, -1, -1},
	{10,  7,  6,  1,  7, 10,  1,  8,  7,  1,  0,  8, -1, -1, -1, -1},
	{ 0,  3,  7,  0,  7, 10,  0, 10,  9,  6, 10,  7, -1, -1, -1, -1},
	{ 7,  6, 10,  7, 10,  8,  8, 10,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 6,  8,  4, 11,  8,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  6, 11,  3,  0,  6,  0,  4,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  6, 11,  8,  4,  6,  9,  0,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  4,  6,  9,  6,  3,  9,  3,  1, 11,  3,  6, -1, -1, -1, -1},
	{ 6,  8,  4,  6, 11,  8,  2, 10,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  2, 10,  3,  0, 11,  0,  6, 11,  0,  4,  6, -1, -1, -1, -1},
	{ 4, 11,  8,  4,  6, 11,  0,  2,  9,  2, 10,  9, -1, -1, -1, -1},
	{10,  9,  3, 10,  3,  2,  9,  4,  3, 11,  3,  6,  4,  6,  3, -1},
	{ 8,  2,  3,  8,  4,  2,  4,  6,  2, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  4,  2,  4,  6,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  9,  0,  2,  3,  4,  2,  4,  6,  4,  3,  8, -1, -1, -1, -1},
	{ 1,  9,  4,  1,  4,  2,  2,  4,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  1,  3,  8,  6,  1,  8,  4,  6,  6, 10,  1, -1, -1, -1, -1},
	{10,  1,  0, 10,  0,  6,  6,  0,  4, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  6,  3,  4,  3,  8,  6, 10,  3,  0,  3,  9, 10,  9,  3, -1},
	{10,  9,  4,  6, 10,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  9,  5,  7,  6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  8,  3,  4,  9,  5, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  0,  1,  5,  4,  0,  7,  6, 11, -1, -1, -1, -1, -1, -1, -1},
	{11,  7,  6,  8,  3,  4,  3,  5,  4,  3,  1,  5, -1, -1, -1, -1},
	{ 9,  5,  4, 10,  1,  2,  7,  6, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 6, 11,  7,  1,  2, 10,  0,  8,  3,  4,  9,  5, -1, -1, -1, -1},
	{ 7,  6, 11,  5,  4, 10,  4,  2, 10,  4,  0,  2, -1, -1, -1, -1},
	{ 3,  4,  8,  3,  5,  4,  3,  2,  5, 10,  5,  2, 11,  7,  6, -1},
	{ 7,  2,  3,  7,  6,  2,  5,  4,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  5,  4,  0,  8,  6,  0,  6,  2,  6,  8,  7, -1, -1, -1, -1},
	{ 3,  6,  2,  3,  7,  6,  1,  5,  0,  5,  4,  0, -1, -1, -1, -1},
	{ 6,  2,  8,  6,  8,  7,  2,  1,  8,  4,  8,  5,  1,  5,  8, -1},
	{ 9,  5,  4, 10,  1,  6,  1,  7,  6,  1,  3,  7, -1, -1, -1, -1},
	{ 1,  6, 10,  1,  7,  6,  1,  0,  7,  8,  7,  0,  9,  5,  4, -1},
	{ 4,  0, 10,  4, 10,  5,  0,  3, 10,  6, 10,  7,  3,  7, 10, -1},
	{ 7,  6, 10,  7, 10,  8,  5,  4, 10,  4,  8, 10, -1, -1, -1, -1},
	{ 6,  9,  5,  6, 11,  9, 11,  8,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  6, 11,  0,  6,  3,  0,  5,  6,  0,  9,  5, -1, -1, -1, -1},
	{ 0, 11,  8,  0,  5, 11,  0,  1,  5,  5,  6, 11, -1, -1, -1, -1},
	{ 6, 11,  3,  6,  3,  5,  5,  3,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  2, 10,  9,  5, 11,  9, 11,  8, 11,  5,  6, -1, -1, -1, -1},
	{ 0, 11,  3,  0,  6, 11,  0,  9,  6,  5,  6,  9,  1,  2, 10, -1},
	{11,  8,  5, 11,  5,  6,  8,  0,  5, 10,  5,  2,  0,  2,  5, -1},
	{ 6, 11,  3,  6,  3,  5,  2, 10,  3, 10,  5,  3, -1, -1, -1, -1},
	{ 5,  8,  9,  5,  2,  8,  5,  6,  2,  3,  8,  2, -1, -1, -1, -1},
	{ 9,  5,  6,  9,  6,  0,  0,  6,  2, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  5,  8,  1,  8,  0,  5,  6,  8,  3,  8,  2,  6,  2,  8, -1},
	{ 1,  5,  6,  2,  1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  3,  6,  1,  6, 10,  3,  8,  6,  5,  6,  9,  8,  9,  6, -1},
	{10,  1,  0, 10,  0,  6,  9,  5,  0,  5,  6,  0, -1, -1, -1, -1},
	{ 0,  3,  8,  5,  6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{10,  5,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{11,  5, 10,  7,  5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{11,  5, 10, 11,  7,  5,  8,  3,  0, -1, -1, -1, -1, -1, -1, -1},
	{ 5, 11,  7,  5, 10, 11,  1,  9,  0, -1, -1, -1, -1, -1, -1, -1},
	{10,  7,  5, 10, 11,  7,  9,  8,  1,  8,  3,  1, -1, -1, -1, -1},
	{11,  1,  2, 11,  7,  1,  7,  5,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  8,  3,  1,  2,  7,  1,  7,  5,  7,  2, 11, -1, -1, -1, -1},
	{ 9,  7,  5,  9,  2,  7,  9,  0,  2,  2, 11,  7, -1, -1, -1, -1},
	{ 7,  5,  2,  7,  2, 11,  5,  9,  2,  3,  2,  8,  9,  8,  2, -1},
	{ 2,  5, 10,  2,  3,  5,  3,  7,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  2,  0,  8,  5,  2,  8,  7,  5, 10,  2,  5, -1, -1, -1, -1},
	{ 9,  0,  1,  5, 10,  3,  5,  3,  7,  3, 10,  2, -1, -1, -1, -1},
	{ 9,  8,  2,  9,  2,  1,  8,  7,  2, 10,  2,  5,  7,  5,  2, -1},
	{ 1,  3,  5,  3,  7,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  8,  7,  0,  7,  1,  1,  7,  5, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  0,  3,  9,  3,  5,  5,  3,  7, -1, -1, -1, -1, -1, -1, -1},
	{ 9,  8,  7,  5,  9,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  8,  4,  5, 10,  8, 10, 11,  8, -1, -1, -1, -1, -1, -1, -1},
	{ 5,  0,  4,  5, 11,  0,  5, 10, 11, 11,  3,  0, -1, -1, -1, -1},
	{ 0,  1,  9,  8,  4, 10,  8, 10, 11, 10,  4,  5, -1, -1, -1, -1},
	{10, 11,  4, 10,  4,  5, 11,  3,  4,  9,  4,  1,  3,  1,  4, -1},
	{ 2,  5,  1,  2,  8,  5,  2, 11,  8,  4,  5,  8, -1, -1, -1, -1},
	{ 0,  4, 11,  0, 11,  3,  4,  5, 11,  2, 11,  1,  5,  1, 11, -1},
	{ 0,  2,  5,  0,  5,  9,  2, 11,  5,  4,  5,  8, 11,  8,  5, -1},
	{ 9,  4,  5,  2, 11,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  5, 10,  3,  5,  2,  3,  4,  5,  3,  8,  4, -1, -1, -1, -1},
	{ 5, 10,  2,  5,  2,  4,  4,  2,  0, -1, -1, -1, -1, -1, -1, -1},
	{ 3, 10,  2,  3,  5, 10,  3,  8,  5,  4,  5,  8,  0,  1,  9, -1},
	{ 5, 10,  2,  5,  2,  4,  1,  9,  2,  9,  4,  2, -1, -1, -1, -1},
	{ 8,  4,  5,  8,  5,  3,  3,  5,  1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  4,  5,  1,  0,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 8,  4,  5,  8,  5,  3,  9,  0,  5,  0,  3,  5, -1, -1, -1, -1},
	{ 9,  4,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4, 11,  7,  4,  9, 11,  9, 10, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  8,  3,  4,  9,  7,  9, 11,  7,  9, 10, 11, -1, -1, -1, -1},
	{ 1, 10, 11,  1, 11,  4,  1,  4,  0,  7,  4, 11, -1, -1, -1, -1},
	{ 3,  1,  4,  3,  4,  8,  1, 10,  4,  7,  4, 11, 10, 11,  4, -1},
	{ 4, 11,  7,  9, 11,  4,  9,  2, 11,  9,  1,  2, -1, -1, -1, -1},
	{ 9,  7,  4,  9, 11,  7,  9,  1, 11,  2, 11,  1,  0,  8,  3, -1},
	{11,  7,  4, 11,  4,  2,  2,  4,  0, -1, -1, -1, -1, -1, -1, -1},
	{11,  7,  4, 11,  4,  2,  8,  3,  4,  3,  2,  4, -1, -1, -1, -1},
	{ 2,  9, 10,  2,  7,  9,  2,  3,  7,  7,  4,  9, -1, -1, -1, -1},
	{ 9, 10,  7,  9,  7,  4, 10,  2,  7,  8,  7,  0,  2,  0,  7, -1},
	{ 3,  7, 10,  3, 10,  2,  7,  4, 10,  1, 10,  0,  4,  0, 10, -1},
	{ 1, 10,  2,  8,  7,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  9,  1,  4,  1,  7,  7,  1,  3, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  9,  1,  4,  1,  7,  0,  8,  1,  8,  7,  1, -1, -1, -1, -1},
	{ 4,  0,  3,  7,  4,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 4,  8,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 9, 10,  8, 10, 11,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  0,  9,  3,  9, 11, 11,  9, 10, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  1, 10,  0, 10,  8,  8, 10, 11, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  1, 10, 11,  3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  2, 11,  1, 11,  9,  9, 11,  8, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  0,  9,  3,  9, 11,  1,  2,  9,  2, 11,  9, -1, -1, -1, -1},
	{ 0,  2, 11,  8,  0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 3,  2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  3,  8,  2,  8, 10, 10,  8,  9, -1, -1, -1, -1, -1, -1, -1},
	{ 9, 10,  2,  0,  9,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 2,  3,  8,  2,  8, 10,  0,  1,  8,  1, 10,  8, -1, -1, -1, -1},
	{ 1, 10,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,  3,  8,  9,  1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  9,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,  3,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}};

/*
	Returns the value of a function representing a 3d volume (in this case, a sphere with radius 1).
*/
float density(glm::vec3 p)
{ 
	return sqrtf(p.x * p.x + p.y * p.y + p.z * p.z) - 1.0f;
}

/*
	Returns the interpolated value for a vertex lying between two sample points (corners of a cube).
	Based on implementation at: http://paulbourke.net/geometry/polygonise/.
*/
glm::vec3 interpolateVertex(glm::vec4 corner1, glm::vec4 corner2, float isolevel)
{
	if (abs(isolevel - corner1.w) < 0.00001f)
	{
		return corner1;
	}
	if (abs(isolevel - corner2.w) < 0.00001f)
	{
		return corner2;
	}
	if (abs(corner1.w - corner2.w) < 0.00001f)
	{
		return corner1;
	}

	float midpointDistance = (isolevel - corner1.w) / (corner2.w - corner1.w);
	return glm::vec3(
		corner1.x + midpointDistance * (corner2.x - corner1.x),
		corner1.y + midpointDistance * (corner2.y - corner1.y),
		corner1.z + midpointDistance * (corner2.z - corner1.z)
	);
}

/*
	Returns a numerical approximation of a vertex's normal using the tetrahedron technique described at
	https://www.iquilezles.org/www/articles/normalsSDF/normalsSDF.htm.
*/
glm::vec3 vertexNormal(glm::vec3 vertex)
{
	const float h = 0.0001;
	return glm::normalize(
		glm::vec3(1.0f, -1.0f, -1.0f) * density(vertex + glm::vec3(1.0f, -1.0f, -1.0f) * h) +
		glm::vec3(-1.0f, -1.0f, 1.0f) * density(vertex + glm::vec3(-1.0f, -1.0f, 1.0f) * h) +
		glm::vec3(-1.0f, 1.0f, -1.0f) * density(vertex + glm::vec3(-1.0f, 1.0f, -1.0f) * h) +
		glm::vec3(1.0f, 1.0f, 1.0f) * density(vertex + glm::vec3(1.0f, 1.0f, 1.0f) * h)
	);
}

/*
	Calculates and stores the vertices generated for a single cube for rendering using SDL/OpenGL.
	Based on implementation at: http://paulbourke.net/geometry/polygonise/
*/
void polygonize(glm::vec3 center, glm::vec3 radius, float isolevel, std::vector<glm::vec3> &vertexBufferData)
{
	// Calculates the corners of the cube and the value of the sampling function at each corner.
	std::array<glm::vec4, 8> corners;
	for (int n = 0, i = -1; i < 2; i += 2)
	{
		for (int j = -1; j < 2; j += 2)
		{
			for (int k = -1; k < 2; k += 2, n++)
			{
				corners[n] = glm::vec4(
					center.x + j * radius.x,
					center.y + i * radius.y,
					center.z - j * k * radius.z,
					0.0f);
				corners[n].w = density(corners[n]);
			}
		}
	}

	// Calculates the index into the edgeTable and triTable based on which corners are inside our desired surface.
	int cubeIndex = 0;
	for (int i = 0; i < 8; i++)
	{
		if (corners[i].w < isolevel)
		{
			cubeIndex |= 1 << i;
		}
	}

	// Generate the vertices for this cube from its index.
	std::array<glm::vec3, 12> vertices;
	if ((edgeTable[cubeIndex] && (1 << 0)) != 0)
	{
		vertices[0] = interpolateVertex(corners[0], corners[1], isolevel);
	}
	if ((edgeTable[cubeIndex] && (1 << 1)) != 0)
	{
		vertices[1] = interpolateVertex(corners[1], corners[2], isolevel);
	}
	if ((edgeTable[cubeIndex] && (1 << 2)) != 0)
	{
		vertices[2] = interpolateVertex(corners[2], corners[3], isolevel);
	}
	if ((edgeTable[cubeIndex] && (1 << 3)) != 0)
	{
		vertices[3] = interpolateVertex(corners[3], corners[0], isolevel);
	}
	if ((edgeTable[cubeIndex] && (1 << 4)) != 0)
	{
		vertices[4] = interpolateVertex(corners[4], corners[5], isolevel);
	}
	if ((edgeTable[cubeIndex] && (1 << 5)) != 0)
	{
		vertices[5] = interpolateVertex(corners[5], corners[6], isolevel);
	}
	if ((edgeTable[cubeIndex] && (1 << 6)) != 0)
	{
		vertices[6] = interpolateVertex(corners[6], corners[7], isolevel);
	}
	if ((edgeTable[cubeIndex] && (1 << 7)) != 0)
	{
		vertices[7] = interpolateVertex(corners[7], corners[4], isolevel);
	}
	if ((edgeTable[cubeIndex] && (1 << 8)) != 0)
	{
		vertices[8] = interpolateVertex(corners[0], corners[4], isolevel);
	}
	if ((edgeTable[cubeIndex] && (1 << 9)) != 0)
	{
		vertices[9] = interpolateVertex(corners[1], corners[5], isolevel);
	}
	if ((edgeTable[cubeIndex] && (1 << 10)) != 0)
	{
		vertices[10] = interpolateVertex(corners[2], corners[6], isolevel);
	}
	if ((edgeTable[cubeIndex] && (1 << 11)) != 0)
	{
		vertices[11] = interpolateVertex(corners[3], corners[7], isolevel);
	}

	// Store the vertices and their normals for rendering with SDL/OpenGL.
	for (int i = 0; triTable[cubeIndex][i] != -1; i += 3)
	{
		vertexBufferData.push_back(vertices[triTable[cubeIndex][i]]);
		vertexBufferData.push_back(vertexNormal(vertexBufferData.back()));
		vertexBufferData.push_back(vertices[triTable[cubeIndex][i + 1]]);
		vertexBufferData.push_back(vertexNormal(vertexBufferData.back()));
		vertexBufferData.push_back(vertices[triTable[cubeIndex][i + 2]]);
		vertexBufferData.push_back(vertexNormal(vertexBufferData.back()));
	}
}

/*
	Runs the marching cubes algorithm on a bounded grid with a specified resolution.
*/
void marchingCubes(glm::vec3 min, glm::vec3 max, int resolution, std::vector<glm::vec3> &vertexBufferData)
{
	glm::vec3 step = (max - min) / (float)resolution;
	glm::vec3 radius = step * 0.5f;

	for (int i = 0; i <= resolution; i++)
	{
		for (int j = 0; j <= resolution; j++)
		{
			for (int k = 0; k <= resolution; k++)
			{
				glm::vec3 center = glm::vec3(min.x + step.x * i, min.y + step.y * j, min.z + step.z * k);
				polygonize(center, radius, 0.0f, vertexBufferData);
			}
		}
	}
}

/*
	Returns a vector representing the position of the camera as a function of time.
*/
glm::vec3 viewPos(float t)
{
	return glm::vec3(5.0f * glm::sin(t), 3.0f * glm::sin(t), 5.0f * glm::cos(t));
}

/*
	Returns the combined model-view-projection matrix as a function of time. A model matrix is not included yet.
*/
glm::mat4 mvp(float t)
{
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 1.0f, 10.0f);
	glm::mat4 view = glm::lookAt(
		viewPos(t),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	return proj * view;
}

/*
	Takes care of rendering everything using SDL/OpenGL. Mostly boilerplate code.
*/
int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_Window *window = SDL_CreateWindow("Marching Cubes", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW!" << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, WIDTH, HEIGHT);

	glEnable(GL_DEPTH_TEST);

	Shader shader("marching_cubes.vs", "marching_cubes.fs");

	std::vector<glm::vec3> vertexBufferData;
	marchingCubes(MIN, MAX, RESOLUTION, vertexBufferData);

	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferData.size() * sizeof(glm::vec3), &vertexBufferData.front(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Uncomment for wireframe view.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	SDL_Event windowEvent;
	while (true)
	{
		if (SDL_PollEvent(&windowEvent))
		{
			if (windowEvent.type == SDL_QUIT)
			{
				break;
			}
            else if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_ESCAPE)
            {
                break;
            }
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		GLuint viewPosLocation = glGetUniformLocation(shader.program, "viewPos");
		glm::vec3 _viewPos = viewPos(SDL_GetTicks() / 1000.0f);
		glUniform3fv(viewPosLocation, 1, &_viewPos[0]);

		GLuint mvpLocation = glGetUniformLocation(shader.program, "mvp");
		glm::mat4 _mvp = mvp(SDL_GetTicks() / 1000.0f);
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &_mvp[0][0]);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertexBufferData.size());
		glBindVertexArray(0);

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
