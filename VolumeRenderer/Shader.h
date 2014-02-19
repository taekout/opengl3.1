#pragma once
#include <iostream>
#include <sys/stat.h>
#include <stdlib.h>
#include <GL/glew.h>
#include "glut.h"

using namespace std;

class ShaderFileTimeStampDiff
{
	__time64_t prevShaderFileTimeStamp;
	__time64_t shaderFileTimeStamp;
public:
	ShaderFileTimeStampDiff() {
		prevShaderFileTimeStamp = -1;
		shaderFileTimeStamp = -1;
	}
	ShaderFileTimeStampDiff(__time64_t prev, __time64_t current) {
		prevShaderFileTimeStamp = prev;
		shaderFileTimeStamp = current;
	}

	inline __time64_t GetCurTime() {
		return shaderFileTimeStamp;
	}
	bool IsChanged()
	{
		return prevShaderFileTimeStamp != shaderFileTimeStamp;
	}

	ShaderFileTimeStampDiff & operator = (__time64_t newTimeStamp)
	{
		prevShaderFileTimeStamp = shaderFileTimeStamp;
		shaderFileTimeStamp = newTimeStamp;
		return *this;
	}
};

class Shader
{
private:
	string vertFilename;
	string fragFilename;

	ShaderFileTimeStampDiff vertTimeStamp;
	ShaderFileTimeStampDiff fragTimeStamp;
protected:
	char *textFileRead(char *fn);
	string *textProcess(string data);
public:
	Shader(void);
	~Shader(void);

	virtual void setShaders(char * vertShader, char *fragShader);
	virtual void LinkShaders();
	int textFileWrite(char *fn, char *s);
	string *ShaderFileRead(string filename, string shaderKind);
	void ShaderFileChangeWatcher(void);
	void ReloadVertShader(string vertShader);
	// Get Functions
	GLuint GetProgram(void) { return p; }
	// Log Functions
	void printShaderInfoLog(GLuint obj);
	void printProgramInfoLog(GLuint obj);

	// Update GL Variable Functions
	void UpdateUniform4fv(char *varName, float data1, float data2, float data3, float data4)
	{
		GLint loc;
		loc = glGetUniformLocation(p, varName);
		if(loc == -1)
			return;
		GLfloat dataToUpdate[4] = {data1, data2, data3, data4};
		glUniform4fv(loc, 1, dataToUpdate);
	}
	void UpdateUniform1f(char *varName, float data)
	{
		GLint loc;
		loc = glGetUniformLocation(p, varName);
		if(loc == -1)
			return;
		glUniform1f(loc, data);
	}
	void UpdateUniform3fv(char *varName, float data1, float data2, float data3)
	{
		GLint loc;
		loc = glGetUniformLocation(p, varName);
		if(loc == -1)
			return;
		GLfloat dataToUpdate[3] = {data1, data2, data3};
		glUniform3fv(loc, 1, dataToUpdate);
	}

	void UpdateUniformMat4(char *varName, float * data)
	{
		GLint loc;
		loc = glGetUniformLocation(p, varName);
		if(loc == -1)
			return;
		glUniformMatrix4fv(loc, 1, false, data);
	}

private:
	GLint loc;
	GLuint v,f,f2,p;
};