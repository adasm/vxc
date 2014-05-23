#pragma once
#include "renderer.h"

struct Editor
{
	bool init();
	void loadKeywords();

	void loadFile(bool onlyToBuffer = false);
	void saveFile(bool previous = false);
	void closeFile();

	float getLinePos(int i);
	float toEdge(float y);
	void adjustPosition(float delta);
	void drawLine(std::string text, float x, float y, float alpha, int cursorPos = -1, bool forceColor = false, vec4 forcedColor = vec4(1, 0, 1, 1));
	void draw(float delta);

	void adjustCursor();
	void insert(char c, int space);
	void breakLine();
	void removeChar();
	void removeCharAfter();
	void keyPressed(unsigned char key, int x, int y);
	void keySpecial(int key, int x, int y);
	void parse(std::string cmd);

	void edit_start(std::string name);
	void edit_stop();

	bool visible;
	bool consoleMode;
	Shader* currentShader;
	bool fileLoaded;
	std::string consoleString;
	int consoleCursor;
	std::string fileBuffer;
	std::vector<std::string> lines;
	int currentLineEdit;
	int currentLineCursorPos;
	int targetCursorPos ;
	float posY;
	DWORD rebuildTime;
	bool rebuildFailed;

	std::map<std::string, vec3> keywords;
	std::map<std::string, vec3> consoleKeywords;
};


extern Editor gE;