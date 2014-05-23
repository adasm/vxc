#include "base.h"
#include "editor.h"
#include "log.h"
#include "vm.h"
#include <boost\tokenizer.hpp>

Editor gE;

int func_editFile(lua_State *L)
{
	int argc = lua_gettop(L);
	gE.edit_start(argc < 1 ? "" : lua_tostring(L, 1));
	return 0;
}

int func_closeFile(lua_State *L)
{
	int argc = lua_gettop(L);
	gE.edit_stop();
	return 0;
}

bool Editor::init()
{
	currentLineEdit = 0;
	currentLineCursorPos = 0;
	targetCursorPos = 0;
	consoleCursor = 0;
	posY = 0;
	rebuildTime;
	rebuildFailed = false;
	visible = true;
	fileLoaded = false;
	currentShader = 0;
	consoleMode = true;
	loadKeywords();
	consoleKeywords.insert(std::make_pair("edit", vec3(1, 0, 0)));
	consoleKeywords.insert(std::make_pair("close", vec3(1, 0, 0)));
	gVM.reg("edit", func_editFile);
	gVM.reg("close", func_closeFile);
	return true;
}

void Editor::loadKeywords()
{
	std::ifstream fin("editor/keywords", std::ios::in);
	if (fin.is_open())
	{
		vec3 col = vec3(1, 0, 1);
		std::string tok;
		while (fin >> tok)
		{
			if (tok == ":")
			{
				fin >> col.r >> col.g >> col.b;
			}
			else
			{
				keywords.insert(std::make_pair(tok, col));
			}
		}
	}
	prn("Loaded %u editor keywords", keywords.size());
}

void Editor::loadFile(bool onlyToBuffer)
{
	if (currentShader == 0) return;
	std::ifstream fin(currentShader->fragFile, std::ios::in);
	if (fin.is_open())
	{
		if (!onlyToBuffer)
			lines.clear();
		std::string line = "";
		while (getline(fin, line)) {
			fileBuffer += line + '\n';
			if (!onlyToBuffer)
				lines.push_back(line);
		}
		fin.close();

		if (!onlyToBuffer)
		{
			currentLineCursorPos = 0;
			currentLineEdit = 0;
			fileLoaded = true;
		}
	}
	else fileLoaded = false;
}

void Editor::saveFile(bool previous)
{
	if (currentShader == 0) return;
	std::ofstream fout(currentShader->fragFile);
	if (fout.is_open())
	{
		if (previous)
			fout << fileBuffer;
		else {
			for (int i = 0; i < lines.size(); i++)
				fout << lines[i] << std::endl;
		}

		fout.close();
	}
}

void Editor::closeFile()
{
	consoleMode = true;
	lines.clear();
	fileBuffer = "";
	currentLineCursorPos = 0;
	currentLineEdit = 0;
	targetCursorPos = 0;
	posY = 0;
	currentShader = 0;
}

float Editor::getLinePos(int i)
{
	float fh = (gR.defaultFont->fontSize + 1);
	return posY + fh * i;
}

float Editor::toEdge(float y)
{
	float fh = gR.defaultFont->fontSize + 2;
	float h = glutGet(GLUT_WINDOW_HEIGHT) - fh;
	
	if (y > 0 && y < h)
		return std::min(glm::length(y), glm::length(h - y));
	else
		return -std::min(glm::length(y), glm::length(h - y));
}

void Editor::adjustPosition(float delta)
{
	float span = glutGet(GLUT_WINDOW_WIDTH) / 6;
	float center = glutGet(GLUT_WINDOW_HEIGHT) / 2;
	float y = getLinePos(currentLineEdit);
	if (y < center - span)
		posY += delta * 5 * glm::length(y - center + span);
	else if (y > center + span)
		posY -= delta * 5 * glm::length(y - center - span);
}

std::vector<std::string> tokenize(std::string str)
{
	std::string white = " \t\r";
	std::string delim = "-=[];',./+{}:\"<>?\|!@$%^&*()";
	std::string whitedelim = white + delim;
	std::vector<std::string> out;
	while (str.length() > 0)
	{
		if (delim.find(str[0]) != std::string::npos)
		{
			if (str[0] == '\"')
			{
				int end = str.find_first_of("\"", 1);
				if (end != std::string::npos)
				{
					out.push_back(str.substr(0, end + 1));
					str = str.substr(end + 1);
				}
				else
				{
					out.push_back(str);
					str = "";
				}
			}
			else if (str[0] == '#')
			{
				out.push_back(str);
				str = "";
			}
			else if (str.length() > 1 && str.substr(0, 2) == "//")
			{
				out.push_back(str);
				str = "";
			}
			else
			{
				out.push_back(str.substr(0, 1));
				str = str.substr(1, str.length() - 1);
			}
			
		}
		else if (white.find(str[0]) != std::string::npos)
		{
			int end = str.find_first_not_of(white);
			if (end != std::string::npos)
			{
				out.push_back(str.substr(0, end));
				str = str.substr(end);
			}
			else
			{
				out.push_back(str);
				str = "";
			}
			
		}
		else
		{
			int end = str.find_first_of(whitedelim);
			if (end != std::string::npos)
			{
				out.push_back(str.substr(0, end));
				str = str.substr(end);
			}
			else
			{
				out.push_back(str);
				str = "";
			}			
		}
	}
	return out;
}

void Editor::drawLine(std::string text, float _x, float _y, float alpha, int cursorPos, bool forceColor, vec4 forcedColor)
{
	float cs = sin(timeGetTime()*0.005);
	cs = cs*cs;

	std::vector<std::string> tokens = tokenize(text);
	
	float x = _x;
	for (int i = 0; i < tokens.size(); i++)
	{
		std::string numbers = "0123456789.";
		std::string delim = "-=[];',./+{}:\"<>?\|!@#$%^&*()";
		std::string curr = tokens[i];

		vec4 color = vec4(0, 1, 0, alpha);
		if (curr.find_first_of(delim) != std::string::npos)
		{
			color.b += 0.1;
			if (curr.length() > 1)
			{
				if (curr[0] == '#') color = vec4(0.5, 0.5, 0.5, color.a);
				else if (curr[0] == '/') color = vec4(0.1, 0.5, 0.1, color.a);
				else if (curr[0] == '\"') color = vec4(1, 0.5, 0.3, color.a);
				else if (curr[0] == '\'') color = vec4(1, 0.5, 0.3, color.a);
			}
		}
		else
		{
			if (numbers.find(curr[0]) != std::string::npos)
			{
				color = vec4(1, 1, 0, color.a);
			}
			else
			{
				std::map<std::string, vec3>::iterator it = keywords.find(curr);
				if (it != keywords.end())
					color = vec4(it->second, color.a);
				else if ((it = consoleKeywords.find(curr)) != consoleKeywords.end())
					color = vec4(it->second, color.a);
				else
					color = vec4(0.7, 0.7, 1, color.a);
			}
		}

		if (cursorPos >= 0) {
			color.a = alpha;
		}

		if (forceColor)
			color = color * (forcedColor.a) + (1 - forcedColor.a) * vec4(forcedColor.r, forcedColor.g, forcedColor.b, alpha);

		x += gR.render_text(0, curr, x, _y, color);
	}

	//draw cursor
	if (cursorPos >= 0) {
		std::string prev = text.substr(0, cursorPos);
		std::string after = text.substr(cursorPos);
		float prevWidth = gR.defaultFont->getTextWidth(prev);
		float cursorWidth = gR.defaultFont->getTextWidth("|");
		gR.render_text(0, "|", _x - cursorWidth/2 + prevWidth, _y, vec4(1, 1, 1, cs));
	}
}

void Editor::draw(float delta)
{
	if (!visible) return;

	float s = sin(timeGetTime()*0.005);
	s = s * s * 0.25 + 0.75;

	if (consoleMode)
	{
		gR.drawQuad(0, vec4(0, 0, 0, 0.5));
		gR.render_text(0, "> ", 10, 5, vec4(1, 0, 0, 1));
		float sw = gR.defaultFont->getTextWidth(">");
		drawLine(consoleString, 10 + sw + 5, 5, 1, consoleCursor);
		//
		/*std::string prev = consoleString.substr(0, consoleCursor);
		std::string after = consoleString.substr(consoleCursor);
		float prevWidth = gR.defaultFont->getTextWidth(prev);
		gR.render_text(0, consoleString, 10 + sw, 5, vec4(1, 0.7, 0.6, s));
		gR.render_text(0, "|", 7 + prevWidth + sw, 5, vec4(1, 1, 1, s));*/
	}
	else
	{
		gR.drawQuad(0, vec4(0, 0, 0, 0.9));

		if (lines.size() <= 0)
			return;

		adjustPosition(delta);

		float w = glutGet(GLUT_WINDOW_WIDTH);
		float x = 10;

		bool rebuildColor = false;
		float deltaColor = rebuildFailed ? 3000 : 500;
		if (timeGetTime() - rebuildTime < deltaColor) {
			rebuildColor = true;
			deltaColor = (timeGetTime() - rebuildTime) / deltaColor;
			deltaColor = pow(deltaColor, 3);
		}

		for (int i = 0; i < lines.size(); i++)
		{
			float y = getLinePos(i);
			if (toEdge(y) < -10) continue;

			float sy = 2.0 / glutGet(GLUT_WINDOW_HEIGHT);
			float fade = (1 - glm::length(1 - y * sy) / sqrt(2));
			fade = pow(fade, 2) - 0.1;

			if (rebuildColor)
				drawLine(lines[i], x, y, i == currentLineEdit ? s * fade : fade, i == currentLineEdit ? currentLineCursorPos : -1, true, rebuildFailed ? vec4(1, 0, 0, deltaColor) : vec4(0, 1, 0, deltaColor));
			else
				drawLine(lines[i], x, y, i == currentLineEdit ? s * fade : fade, i == currentLineEdit ? currentLineCursorPos : -1);
		}

		vec4 infoc = vec4(1, 1, 1, 0.5);
		if (rebuildColor)
			infoc = deltaColor * infoc + (1 - deltaColor) * (rebuildFailed ? vec4(1, 0, 0, 1) : vec4(0, 1, 0, 1));
		gR.render_text(0, currentShader->fragFile, 10, 5, infoc);
		if (rebuildColor)
			gR.render_text(0, currentShader->errors, 10, 25, (rebuildFailed ? vec4(1, 0, 0, pow(1 - deltaColor, 0.25)) : vec4(0, 1, 0, pow(1 - deltaColor, 0.25))));
	}

	gR.flushText();
}



void Editor::insert(char c, int space)
{
	if (consoleMode)
	{
		std::string prev = consoleString.substr(0, consoleCursor);
		std::string after = consoleString.substr(consoleCursor);
		consoleString = prev + (char)c + after;
		consoleCursor += space;
	}
	else
	{
		std::string prev = lines[currentLineEdit].substr(0, currentLineCursorPos);
		std::string after = lines[currentLineEdit].substr(currentLineCursorPos);
		lines[currentLineEdit] = prev + (char)c + after;
		currentLineCursorPos += space;
	}
}

void Editor::breakLine()
{
	if (consoleMode)
	{
		//parse(consoleString);
		gVM.proc(consoleString);

		consoleString = "";
		consoleCursor = 0;
	}
	else
	{
		std::string prev = lines[currentLineEdit].substr(0, currentLineCursorPos);
		std::string after = lines[currentLineEdit].substr(currentLineCursorPos);
		lines[currentLineEdit] = prev;
		std::string prefix = lines[currentLineEdit].substr(0, lines[currentLineEdit].find_first_not_of(" \t"));
		lines.insert(lines.begin() + currentLineEdit + 1, prefix + after);
		currentLineCursorPos = prefix.length();
		currentLineEdit++;
	}
}

void Editor::removeChar()
{
	if (consoleMode)
	{
		if (consoleCursor > 0)
		{
			if (consoleCursor < consoleString.length())
			{
				char cp = consoleString[consoleCursor - 1], ca = consoleString[consoleCursor];
				if (cp == '(' && ca == ')')removeCharAfter();
				if (cp == '[' && ca == ']')removeCharAfter();
				if (cp == '{' && ca == '}')removeCharAfter();
				if (cp == '\"' && ca == '\"')removeCharAfter();
				if (cp == '\'' && ca == '\'')removeCharAfter();
			}

			std::string prev = consoleString.substr(0, consoleCursor - 1);
			std::string after = consoleString.substr(consoleCursor);
			consoleString = prev + after;
			consoleCursor--;
		}
	}
	else
	{
		if (currentLineCursorPos > 0)
		{
			if (currentLineCursorPos < lines[currentLineEdit].length())
			{
				char cp = lines[currentLineEdit][currentLineCursorPos - 1], ca = lines[currentLineEdit][currentLineCursorPos];
				if (cp == '(' && ca == ')')removeCharAfter();
				if (cp == '[' && ca == ']')removeCharAfter();
				if (cp == '{' && ca == '}')removeCharAfter();
				if (cp == '\"' && ca == '\"')removeCharAfter();
				if (cp == '\'' && ca == '\'')removeCharAfter();
			}
			std::string prev = lines[currentLineEdit].substr(0, currentLineCursorPos - 1);
			std::string after = lines[currentLineEdit].substr(currentLineCursorPos);
			lines[currentLineEdit] = prev + after;
			currentLineCursorPos--;
		}
		else if (currentLineEdit > 0)
		{
			int len = lines[currentLineEdit - 1].length();
			lines[currentLineEdit - 1] += lines[currentLineEdit];
			lines.erase(lines.begin() + currentLineEdit);
			currentLineEdit--;
			currentLineCursorPos = len;
		}
	}
}

void Editor::removeCharAfter()
{
	if (consoleMode)
	{
		std::string prev = consoleString.substr(0, consoleCursor);
		std::string after = consoleString.substr(consoleCursor + 1);
		consoleString = prev + after;
	}
	else
	{
		if (currentLineCursorPos < lines[currentLineEdit].length())
		{
			std::string prev = lines[currentLineEdit].substr(0, currentLineCursorPos);
			std::string after = lines[currentLineEdit].substr(currentLineCursorPos + 1);
			lines[currentLineEdit] = prev + after;
		}
		else if (currentLineEdit < lines.size())
		{
			int len = lines[currentLineEdit].length();
			lines[currentLineEdit] += lines[currentLineEdit + 1];
			lines.erase(lines.begin() + currentLineEdit + 1);
		}
	}
}

void Editor::keyPressed(unsigned char key, int x, int y)
{
	if (key == '`')
	{
		if (!visible)
		{
			visible = true;
			consoleMode = true;
		}
		else
		{
			consoleMode = !consoleMode;
			if (currentShader == 0 || fileLoaded == false)
				consoleMode = true;
		}
		return;
	}

	if (!visible) return;
	if (key >= 32 && key < 127) {
		if (key == '{') {
			insert('{', 1);
			insert('}', 0);
		}
		else if (key == '(') {
			insert('(', 1);
			insert(')', 0);
		}
		else if (key == '[') {
			insert('[', 1);
			insert(']', 0);
		}
		else if (key == '\"') {
			insert('\"', 1);
			insert('\"', 0);
		}
		else if (key == '\'') {
			insert('\'', 1);
			insert('\'', 0);
		}
		else {
			insert(key, 1);
		}
	}
	if (key == '\t')
		insert('\t', 1);
	if (key == 13)
		breakLine();
	if (key == 8)
		removeChar();
	if (key == 127)
		removeCharAfter();

}

void Editor::adjustCursor()
{
	if (consoleMode)
	{
	}
	else
	{
		if (currentLineCursorPos > lines[currentLineEdit].length())
			currentLineCursorPos = lines[currentLineEdit].length();
		if (currentLineCursorPos < 0)
			currentLineCursorPos = 0;
	}	
}

void Editor::keySpecial(int key, int x, int y) {
	if (key == GLUT_KEY_F1)
		visible = !visible;

	if (!visible) return;

	float fh = (gR.defaultFont->fontSize + 1);

	if (key == GLUT_KEY_LEFT) {
		if (consoleMode)
		{
			if (consoleCursor > 0)
				consoleCursor--;
		}
		else
		{
			if (currentLineCursorPos > 0)
				currentLineCursorPos--;
			else if (currentLineEdit > 0) {
				currentLineEdit--;
				currentLineCursorPos = lines[currentLineEdit].length();
			}
			targetCursorPos = currentLineCursorPos;
		}
	}

	if (key == GLUT_KEY_RIGHT) {
		if (consoleMode)
		{
			if (consoleCursor < consoleString.length())
				consoleCursor++;
		}
		else
		{
			if (currentLineCursorPos < lines[currentLineEdit].length())
				currentLineCursorPos++;
			else if (currentLineEdit < lines.size() - 1) {
				currentLineEdit++;
				currentLineCursorPos = 0;
			}
			targetCursorPos = currentLineCursorPos;
		}
	}

	if (key == GLUT_KEY_UP) {
		if (consoleMode)
		{
		}
		else
		{
			if (currentLineEdit > 0)
				currentLineEdit--;

			currentLineCursorPos = targetCursorPos;
			adjustCursor();
		}
	}
	if (key == GLUT_KEY_DOWN) {
		if (consoleMode)
		{
		}
		else
		{
			if (currentLineEdit < lines.size() - 1)
				currentLineEdit++;

			currentLineCursorPos = targetCursorPos;
			adjustCursor();
		}
	}

	if (key == GLUT_KEY_F5) {
		if (consoleMode)
		{
		}
		else
		{
			if (currentShader != 0 && fileLoaded)
			{
				std::string fragFile = currentShader->fragFile;
				currentShader->fragFile = "editor/frag.temp";
				saveFile();
				rebuildFailed = !currentShader->rebuild();
				currentShader->fragFile = fragFile;
				if (!rebuildFailed)
					saveFile();


				rebuildTime = timeGetTime();
			}
		}
	}

	if (key == GLUT_KEY_PAGE_DOWN)
	{
		if (consoleMode)
		{
		}
		else
		{
			float h = glutGet(GLUT_WINDOW_HEIGHT);
			int sl = (h / (gR.defaultFont->fontSize + 1)) - 1;
			currentLineEdit += sl;
			if (currentLineEdit > lines.size() - 1)
				currentLineEdit = lines.size() - 1;
		}
	}

	if (key == GLUT_KEY_PAGE_UP)
	{
		if (consoleMode)
		{
		}
		else
		{
			float h = glutGet(GLUT_WINDOW_HEIGHT);
			int sl = (h / (gR.defaultFont->fontSize + 1)) - 1;
			currentLineEdit -= sl;
			if (currentLineEdit < 0)
				currentLineEdit = 0;
		}
	}

	if (key == GLUT_KEY_HOME)
	{

		if (consoleMode)
		{
		}
		else
		{
			currentLineCursorPos = 0;
		}
	}

	if (key == GLUT_KEY_END)
	{
		if (consoleMode)
		{
		}
		else
		{
			currentLineCursorPos = lines[currentLineEdit].length();
		}
	}
}

void Editor::edit_start(std::string name)
{
	if (name.length() < 1)
	{
		if (currentShader != 0 && fileLoaded)
			consoleMode = false;
	}
	else
	{
		closeFile();
		currentShader = gR.getShader(name);
		loadFile();
		if (fileLoaded)
			consoleMode = false;
	}
}

void Editor::edit_stop()
{
	closeFile();
}

//void Editor::parse(std::string cmd)
//{
//	boost::tokenizer<> tok(cmd);
//	boost::tokenizer<>::iterator iter = tok.begin();
//	std::vector<std::string> tokens;
//	while (iter != tok.end()) {
//		prn((*iter).c_str());
//		tokens.push_back(*(iter++));
//	}
//
//	if (tokens.size() < 1)
//		return;
//	int numarg = tokens.size() - 1;
//	std::string command = tokens[0];
//	if (command == "close") {
//		closeFile();
//	}
//	if (command == "edit") {
//		if (numarg < 1)
//		{
//			if (currentShader != 0 && fileLoaded)
//				consoleMode = false;
//		}
//		else
//			edit(tokens[1]);
//
//	}
//}