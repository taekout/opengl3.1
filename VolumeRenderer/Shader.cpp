#include "Shader.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <sstream>


Shader::Shader(void)
	: loc(0), v(0), f(0), f2(0), p(0)
{
}


Shader::~Shader(void)
{
}

char * Shader::textFileRead(char  *fn) {
	FILE *fp = NULL;
	char *content = NULL;

	int count=0;

	if (fn != NULL) {
		fopen_s(&fp, fn,"rt");

		if (fp != NULL) {
      
      fseek(fp, 0, SEEK_END);
      count = ftell(fp);
      rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
/*	fstream fp;
	char * content = NULL;
	int count = 0;

	if(fn != NULL)
	{
		fp.open(fn, fstream::in);
		if( fp.is_open() ) 
		{
			fp.seekg(0,	ios::end);
			count = fp.tellg();
			fp.seekg(0, ios::beg);

			if( count > 0 )
			{
				content = new char[count];
				fp.read(content, count );
				content[count] = '\0';
			}
			fp.close();
		}
	}
	return content;*/
}

string *Shader::ShaderFileRead(string filename, string shaderKind)
{
	string data = textFileRead((char *)filename.c_str());
	string *final = textProcess(data);
	if(shaderKind == "vertex shader") {
		vertFilename = filename;
	}
	else if(shaderKind == "fragment shader") {
		fragFilename = filename;
	}

	// Register the shader files.
	return final;
}

string *Shader::textProcess(string stringData)
{
	string *final = new string;
	size_t pos, filenamePos, filenamePos2;
	while(stringData.find("#include", 0) != string::npos)
	{	// Parse #include and get the file name
		pos = stringData.find("#include", 0);
		filenamePos = stringData.find("\"", pos);
		filenamePos ++;
		filenamePos2 = stringData.find("\"", filenamePos);
		string filename = stringData.substr(filenamePos, filenamePos2-filenamePos);
		*final += stringData.substr(0, pos);
		stringData.erase(0, filenamePos2+1);
		// Add the corresponding file content.
		string contentToAdd = textFileRead((char*)filename.c_str());
		stringData.insert(0, contentToAdd);
	}
	// There is still the remaining part after having found the tokens. So add it.
	*final += stringData;

	// Now step1(find #include and insert the file content after removing the include syntax) is done.
	// Step 2 - find @# and remove @ token.
	while(final ->find("@#", 0) != string::npos) {
		size_t pos = final ->find("@#", 0);
		final ->erase(pos, 1);
	}
	return final;
}

int Shader::textFileWrite(char *fn, char *s) {
	FILE *fp = NULL;
	int status = 0;

	if (fn != NULL) {
		fopen_s(&fp, fn,"w");

		if (fp != NULL) {
			
			if (fwrite(s,sizeof(char),strlen(s),fp) == strlen(s))
				status = 1;
			fclose(fp);
		}
	}
	return(status);
/*	fstream fp;
	int status = 0;
	if(fn != NULL)
	{
		fp.open(fn, fstream::out);
		if(fp.is_open())
		{
			status = !fp.write(s, strlen(s)).bad();
		}
	}
	return(status);*/
}

void Shader::printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);

		if(infologLength == 1)
			printf("shader fine.\n");
    }
}

void Shader::printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);

		if(infologLength == 1)
			printf("shader fine.\n");
    }
	
}

void Shader::ShaderFileChangeWatcher(void)
{
	static int count = 0;
	count ++;
	if(count != 100) {
		return;
	}
	count = 0;
	struct _stat fileinfo;
	// vertex shader file change detection
	if(vertFilename.empty() == true) return;
	if(_stat(vertFilename.c_str(), &fileinfo) != -1) {
		vertTimeStamp = fileinfo.st_mtime;
		if(vertTimeStamp.IsChanged()) {
			setShaders((char *)vertFilename.c_str(), (char *)fragFilename.c_str());
		}
	}
	// fragment shader file change detection
	if(fragFilename.empty() == true) return;
	if(_stat(fragFilename.c_str(), &fileinfo) != -1) {
		fragTimeStamp = fileinfo.st_mtime;
		if(fragTimeStamp.IsChanged())
			setShaders((char *)vertFilename.c_str(), (char *)fragFilename.c_str());
	}
}

void Shader::ReloadVertShader(string vertShader)
{
}

void Shader::setShaders(char *vertShader, char * fragShader) {
	if(p != 0)
	{
		glDetachShader(p, v);
		glDetachShader(p, f);
		glDeleteShader(v);
		glDeleteShader(f);
		glDeleteShader(f2);
		glDeleteProgram(p);
		p = v = f = f2 = 0;
	}
	char *vs = NULL,*fs = NULL,*fs2 = NULL;
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	//f2 = glCreateShader(GL_FRAGMENT_SHADER);

	string *vertData = ShaderFileRead(vertShader, "vertex shader");
	vs = new char[vertData ->length() + 1];
	strcpy_s( vs, vertData ->length() + 1, vertData ->c_str() );
	string *fragData = ShaderFileRead(fragShader, "fragment shader");
	fs = new char[fragData ->length() + 1];
	strcpy_s( fs, fragData ->length() + 1, fragData ->c_str() );

	const char * vv = vs;
	const char * ff = fs;
	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);
	free(vs);free(fs);

	glCompileShader(v);
	glCompileShader(f);
	printShaderInfoLog(v);
	printShaderInfoLog(f);
	//printShaderInfoLog(f2);
	p = glCreateProgram();
	glAttachShader(p,v);
	glAttachShader(p,f);

	
	delete vertData, fragData;
}

void Shader::LinkShaders()
{
	glLinkProgram(p);
	printProgramInfoLog(p);

	glUseProgram(p);
	printf("Use a new program.\n");
}