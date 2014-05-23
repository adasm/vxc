#include "base.h"
#include "renderer.h"
#include "log.h"

std::string convStrForFont(const std::string &text)
{
	std::string str = "";
	for (unsigned int i = 0; i < text.length(); i++)
	{
		char c = text[i];
		if (c == '\t') str += "    ";
		else str += c;
	}
	return str;
}

float Font::getTextWidth(const std::string& text)
{
	std::string str = convStrForFont(text);
	float w = 0;
	for (int i = 0; i < str.length(); i++)
	{
		char a = str[i];
		if (c[a].exists) {
			w += c[a].ax;
		}
	}
	return w;
}

Shader::Shader(const std::string &_vertFile, const std::string &_fragFile)
{
	shaderProgram = 0;
	vertexShader = 0;
	fragmentShader = 0;
	vertFile = _vertFile;
	fragFile = _fragFile;
}

void Shader::release()
{
	if (shaderProgram != 0)
	{
		glDetachShader(shaderProgram, vertexShader);
		glDetachShader(shaderProgram, fragmentShader);
		glDeleteProgram(shaderProgram);
		shaderProgram = 0;
	}
	if (vertexShader != 0)
	{
		glDeleteShader(vertexShader);
		vertexShader = 0;
	}
	if (fragmentShader != 0)
	{
		glDeleteShader(fragmentShader);
		fragmentShader = 0;
	}
	vertBuff = "";
	fragBuff = "";
	errors = "";
}

bool Shader::rebuild()
{
	errors = "";
	std::string VertexShaderCode;
	std::string FragmentShaderCode;

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	VertexShaderCode = "";
	std::ifstream VertexShaderStream(vertFile.c_str(), std::ios::in);
	if (VertexShaderStream.is_open())
	{
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else
	{
		errors = "Shader rebuild failed! Couldn't open file " + vertFile;
		prn("Shader rebuild failed! Couldn't open file %s", vertFile.c_str());
		return false;
	}

	// Read the Fragment Shader code from the file
	fragBuff = FragmentShaderCode = "";
	std::ifstream FragmentShaderStream(fragFile.c_str(), std::ios::in);
	if (FragmentShaderStream.is_open()){
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}
	else
	{
		errors = "Shader rebuild failed! Couldn't open file " + fragFile;
		prn("Shader rebuild failed! Couldn't open file %s", fragFile.c_str());
		return false;
	}

	GLint result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	prn("Compiling shader : %s\n", vertFile.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> VertexShaderErrorMessage(InfoLogLength);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		prn("%s\n", &VertexShaderErrorMessage[0]);
		errors = "Shader rebuild failed!\n" + std::string(&VertexShaderErrorMessage[0]);
		return false;
	}


	// Compile Fragment Shader
	prn("Compiling shader : %s\n", fragFile.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		prn("%s\n", &FragmentShaderErrorMessage[0]);
		errors = "Shader rebuild failed!\n" + std::string(&FragmentShaderErrorMessage[0]);
		return false;
	}

	// Link the program
	prn("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		std::vector<char> ProgramErrorMessage(std::max(InfoLogLength, int(1)));
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		prn("%s\n", &ProgramErrorMessage[0]);
		errors = "Shader rebuild failed!\n" + std::string(&ProgramErrorMessage[0]);
		return false;
	}

	release();

	vertBuff = VertexShaderCode;
	fragBuff = FragmentShaderCode;

	vertexShader = VertexShaderID;
	fragmentShader = FragmentShaderID;
	shaderProgram = ProgramID;
	return vertexShader && fragmentShader && shaderProgram;
}

bool Shader::valid()
{
	return vertexShader && fragmentShader && shaderProgram;
}

void Shader::setUniform1i(const std::string &name, GLint v0)
{
	GLuint loc = glGetUniformLocation(shaderProgram, name.c_str());
	glUniform1i(loc, v0);
}

void Shader::setUniform1f(const std::string &name, float value)
{
	GLuint loc = glGetUniformLocation(shaderProgram, name.c_str());
	glUniform1f(loc, value);
}

void Shader::setUniform2f(const std::string &name, vec2 value)
{
	GLuint loc = glGetUniformLocation(shaderProgram, name.c_str());
	glUniform2f(loc, value.x, value.y);
}

void Shader::setUniform4fv(const std::string &name, GLsizei size, const GLfloat* value)
{
	GLuint loc = glGetUniformLocation(shaderProgram, name.c_str());
	glUniform4fv(loc, size, value);
}


Renderer gR;

void Renderer::init()
{
	int c = 0;
	glutInit(&c, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("vxc");
	glewInit();
	prn("OpenGL version: %s\n", (char*)glGetString(GL_VERSION));
	prn("OpenGL renderer: %s\n", (char*)glGetString(GL_RENDERER));

	ilInit();
	iluInit();
	ilutInit();
}

void Renderer::close()
{

}

void Renderer::loop()
{
	glutMainLoop();
}



void Renderer::load()
{
	glClearColor(0, 0, 0, 1.0);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	textShader = loadProgram("text", "text");
	quadShader = loadProgram("quad", "quad");
	demoShader = loadProgram("demo", "demo");
	texture = loadTexture("textures/demo512.png");
	uniform_coord = glGetAttribLocation(textShader->shaderProgram, "coord");
	uniform_atr = glGetAttribLocation(textShader->shaderProgram, "atr");

	GLfloat black[4] = { 0, 0, 0, 1 };
	textShader->setUniform4fv("color", 1, black);
	textShader->setUniform1i("texture1", 0);
	quadShader->setUniform1i("texture1", 0);
	demoShader->setUniform1i("texture1", 0);

	textVertexBufferSize = 6 * 1024;
	textVertexBuffer = new TextVertex[textVertexBufferSize];
	currentTextVertexBufferPos = 0;
	
	vboSize = sizeof(TextVertex)* textVertexBufferSize;
	glGenBuffersARB(1, &vbo);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, vboSize, 0, GL_DYNAMIC_DRAW_ARB);

	struct {
		vec2 pos;
	} quad[4] = { vec2(-1, 1), vec2(1, 1), vec2(-1, -1), vec2(1, -1) };

	glGenBuffersARB(1, &vboQuad);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboQuad);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 4 * sizeof(vec2), quad, GL_DYNAMIC_DRAW_ARB);

	if (FT_Init_FreeType(&ft)) {
		prn("Could not init freetype library\n");
	}

	currentFont = 0;
	defaultFont = loadFont("fonts/lucon.ttf", 16, 17);
	
}

Shader* Renderer::loadProgram(const std::string &vertex_file_path, const std::string &fragment_file_path)
{
	Shader *shader = new Shader("shaders/" + vertex_file_path + ".vert", "shaders/" + fragment_file_path + ".frag");
	shader->rebuild();
	shaders.insert(std::make_pair(vertex_file_path, shader));
	return shader;
}

Shader* Renderer::getShader(const std::string &name)
{
	std::map<std::string, Shader*>::iterator it = shaders.find(name);
	if (it != shaders.end())
		return it->second;
	else return 0;
}

GLuint Renderer::loadTexture(const std::string &fname)
{
	ILuint imageID;
	GLuint textureID;
	ILboolean success;
	ILenum error;
	ilGenImages(1, &imageID); 
	ilBindImage(imageID); 
	success = ilLoadImage(fname.c_str());
	if (success)
	{
		ILinfo ImageInfo;
		iluGetImageInfo(&ImageInfo);
		if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		{
			iluFlipImage();
		}
	
		success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);

		if (!success)
		{
			error = ilGetError();
			return 0;
		}

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_FORMAT), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());
	}
	else
	{
		error = ilGetError();
		return 0;
	}

	ilDeleteImages(1, &imageID);
	return textureID;
}

Font* Renderer::loadFont(const std::string &fname, unsigned int size, unsigned sizew)
{
	Font *font = new Font();
	font->fontFileName = fname;
	font->fontSize = size;
	if (FT_New_Face(ft, font->fontFileName.c_str(), 0, &font->face)) {
		prn("Could not open font\n");
		return 0;
	}

	FT_Set_Char_Size(font->face, sizew * 16, font->fontSize * 16, 300, 300);
	FT_GlyphSlot g = font->face->glyph;

	font->w = 0;
	font->h = 0;
	for (int i = 0; i < 128; i++)
		font->c[i].exists = false;

	for (int i = 32; i < 128; i++) {
		if (FT_Load_Char(font->face, i, FT_LOAD_RENDER)) {
			prn("Loading character %c failed!\n", i);
			continue;
		}
		font->w += g->bitmap.width;
		font->h = std::max(font->h, g->bitmap.rows);
	}

	
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &font->tex);
	glBindTexture(GL_TEXTURE_2D, font->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font->w, font->h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

	int x = 0, n = 0;
	for (int i = 32; i < 128; i++) {
		if (FT_Load_Char(font->face, i, FT_LOAD_RENDER))
			continue;
		n++;
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		//prn("Cache %c %i %i", i, g->bitmap.width, g->bitmap.rows);
		font->c[i].exists = true;
		font->c[i].ax = g->advance.x >> 6;
		font->c[i].ay = g->advance.y >> 6;

		font->c[i].bw = g->bitmap.width;
		font->c[i].bh = g->bitmap.rows;

		font->c[i].bl = g->bitmap_left;
		font->c[i].bt = g->bitmap_top;

		font->c[i].tx = (float)x / font->w;

		x += g->bitmap.width;
	}
	prn("Cached %i characters for font %s", n, font->fontFileName.c_str());
	return font;
}

bool Renderer::bindShader(Shader *shader)
{
	if (shader == 0 || !shader->valid())
		return false;

	glUseProgram(shader->shaderProgram);
	return true;
}

void check(const std::string &file, int line)
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		const GLubyte *errString;
		errString = gluErrorString(error);
		char buff[2048];
		sprintf_s(buff, "%s at %i", file.c_str(), line);
		MessageBox(0, (char*)errString, buff, 0);
	}
}

#define CHECK check(__FILE__, __LINE__);

void Renderer::pushTextVertex(TextVertex &vertex)
{
	if (currentTextVertexBufferPos >= textVertexBufferSize - 1)
		flushText();
	
	textVertexBuffer[currentTextVertexBufferPos++] = vertex;
}

void Renderer::flushText()
{
	if (!currentFont) return;
	if (!bindShader(textShader))
		return;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, currentFont->tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
	glEnableVertexAttribArrayARB(uniform_coord);
	glVertexAttribPointerARB(uniform_coord, 4, GL_FLOAT, GL_FALSE, sizeof(TextVertex), 0);
	glEnableVertexAttribArrayARB(uniform_atr);
	glVertexAttribPointerARB(uniform_atr, 4, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (const void *)(offsetof(TextVertex, c)));
	float* ptr = (float*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	if (ptr)
	{
		memcpy(ptr, textVertexBuffer, sizeof(TextVertex)* currentTextVertexBufferPos);
		glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
		glDrawArrays(GL_TRIANGLES, 0, currentTextVertexBufferPos);
	}

	currentTextVertexBufferPos = 0;
}

float Renderer::render_text(Font *font, const std::string &text, float _x, float _y, vec4 color)
{
	if (font == 0)
		font = defaultFont;
	if (font == 0)
		return 0;

	if (font != currentFont)
	{
		flushText();
		currentFont = font;
	}

	_x = (int)_x;
	_y = (int)_y;

	float sx = 2.0 / glutGet(GLUT_WINDOW_WIDTH);
	float sy = 2.0 / glutGet(GLUT_WINDOW_HEIGHT);
	float x = -1 + _x * sx;
	float y = 1 - (_y + font->fontSize + 1) * sy;
	std::string str = convStrForFont(text);

	

	int n = 0;

	float ret = 0;

	for (unsigned int i = 0; i < str.length() && n < 512; i++) {
		char c = str[i];

		if (c == '\n')
		{
			x = -1 + _x * sx;
			_y += font->fontSize + 1;
			y = 1 - (_y + font->fontSize + 1) * sy;
			continue;
		}

		if (c < 32 && c > 128) continue;
		if (font->c[c].exists == false) continue;

		
		float x2 = x + font->c[c].bl * sx;
		float y2 = -y - font->c[c].bt * sy;
		float w = font->c[c].bw * sx;
		float h = font->c[c].bh * sy;
		x += font->c[c].ax * sx;
		y += font->c[c].ay * sy;
		
		ret += font->c[c].ax;

		if (!w || !h)
			continue;

		if (-y2 - h > 1 || -y2 < -1) continue;

		pushTextVertex(TextVertex(x2, -y2, font->c[c].tx, 0, color));
		pushTextVertex(TextVertex(x2 + w, -y2, font->c[c].tx + font->c[c].bw / font->w, 0, color));
		pushTextVertex(TextVertex(x2, -y2 - h, font->c[c].tx, font->c[c].bh / font->h, color));
		pushTextVertex(TextVertex(x2 + w, -y2, font->c[c].tx + font->c[c].bw / font->w, 0, color));
		pushTextVertex(TextVertex(x2, -y2 - h, font->c[c].tx, font->c[c].bh / font->h, color));
		pushTextVertex(TextVertex(x2 + w, -y2 - h, font->c[c].tx + font->c[c].bw / font->w, font->c[c].bh / font->h, color));
	}

	return ret;
}

DWORD startTime = timeGetTime();

void Renderer::drawQuad(Shader *shader, vec4 color, GLuint texId)
{
	if (shader == 0)
		shader = quadShader;
	if (!bindShader(shader))
		return;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	float time = (timeGetTime() - startTime) / 10000.0f;

	float h = glutGet(GLUT_WINDOW_HEIGHT);
	float w = glutGet(GLUT_WINDOW_WIDTH);
	shader->setUniform1f("time", time);
	shader->setUniform2f("resolution", vec2(w, h));
	shader->setUniform4fv("color", 1, (float*)&color);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboQuad);
	GLuint uniform_pos = glGetAttribLocation(shader->shaderProgram, "position");
	glEnableVertexAttribArrayARB(uniform_pos);
	glVertexAttribPointerARB(uniform_pos, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	/*char c[200];
	sprintf_s(c, "%f", time);
	render_text(0, c, 0, 12, vec4(1, 1, 1, 10));*/
}