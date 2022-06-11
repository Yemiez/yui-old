#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/freetype.h>
#include "Util.h" 

#ifdef _DEBUG
#include <iostream>

#define LOG() std::cout

inline const char* get_gl_msg(GLenum error)
{
#define casify(x) case x: return #x;
	switch(error) {
		casify(GL_INVALID_VALUE)
		casify(GL_INVALID_ENUM)
		casify(GL_INVALID_OPERATION)
	}
#undef casify
	return "Unknown error";
}

#define CHECK_GL_CONTEXT() GLenum error = GL_NO_ERROR;

#define CHECK_GL_ERROR(reason) if ((error = glGetError()) != GL_NO_ERROR) { \
	yui::Application::the().report_error(yui::fmt("GL Error at line %d: %d %s", __LINE__, error, get_gl_msg(error))); \
	yui::Application::the().report_error(yui::fmt("Reason: %s", reason)); \
} \

#else
#define LOG() ;
#define CHECL_GL_CONTEXT() ;
#define CHECK_GL_ERROR(reason) ;
#endif