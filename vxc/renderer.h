#pragma once

#pragma pack(push, 0)
struct TextVertex {
	TextVertex(float _x = 0, float _y = 0, float _s = 0, float _t = 0, vec4 _c = vec4())
	: x(_x), y(_y), s(_s), t(_t), c(_c) {}
	float x;
	float y;
	float s;
	float t;
	vec4  c;
};
#pragma pack(pop)


struct Font
{
	std::string fontFileName;
	unsigned int fontSize;

	struct character_info {
		float ax; // advance.x
		float ay; // advance.y
		float bw; // bitmap.width;
		float bh; // bitmap.rows;
		float bl; // bitmap_left;
		float bt; // bitmap_top;
		float tx; // x offset of glyph in texture coordinates
		bool  exists;
	} c[128];

	float getTextWidth(const std::string& text);

	FT_Face face;
	GLuint tex;
	int w, h;
};

struct Shader
{
	Shader(const std::string &_vertFile, const std::string &_fragFile);

	void release();
	bool rebuild();
	bool valid();

	void setUniform1i(const std::string &name, int i);
	void setUniform1f(const std::string &name, float value);
	void setUniform2f(const std::string &name, vec2 value);
	void setUniform4fv(const std::string &name, GLsizei size, const GLfloat* value);


	std::string vertFile;
	std::string fragFile;

	std::string vertBuff;
	std::string fragBuff;

	std::string errors;

	GLint vertexShader;
	GLint fragmentShader;
	GLint shaderProgram;
};

struct Renderer
{
	Renderer()
	: currentShader(0) {}

	void init();
	void close();
	void loop();
	void load();

	Shader* loadProgram(const std::string &vert, const std::string &frag);
	Shader* getShader(const std::string &name);
	GLuint loadTexture(const std::string &fname);
	Font* loadFont(const std::string &fname, unsigned int size, unsigned sizew = 0);
	float render_text(Font *font, const std::string &text, float _x, float _y, vec4 color);
	void drawQuad(Shader *shader, vec4 color = vec4(1, 0, 1, 1), GLuint texId = 0);

	bool bindShader(Shader *shader);

	FT_Library ft;
	Font* defaultFont;
	Font* currentFont;
	Shader* textShader;
	Shader* quadShader;
	Shader* demoShader;
	Shader* currentShader;
	GLuint tex;
	GLuint texture;
	GLuint vbo, vboQuad;
	int vboSize;
	GLuint uniform_coord, uniform_atr, uniform_tex, uniform_color;

	int textVertexBufferSize;
	int currentTextVertexBufferPos;
	TextVertex *textVertexBuffer;

	void pushTextVertex(TextVertex &vertex);
	void flushText();

	std::map<std::string, Shader*> shaders;
};

extern Renderer gR;