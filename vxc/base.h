#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <windows.h>
#include <time.h>

#include <GL/glew.h>
#include <gl/glut.h>
#include <GL/GL.h>
#include <glm.hpp>

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#pragma comment(lib, "lua52.lib")

#include <IL\il.h>
#include <IL\ilu.h>
#include <IL\ilut.h>
#pragma comment(lib, "devil.lib")
#pragma comment(lib, "ilu.lib")
#pragma comment(lib, "ilut.lib")

using glm::vec2;
using glm::vec3;
using glm::vec4;

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef _DEBUG
	#pragma comment(lib, "freetype252_D.lib")
#else
	#pragma comment(lib, "freetype252.lib")
#endif