#include "std_util.h"
#include "oglt_util.h"

#include "oglt_freetypefont.h"

#include <glm/gtc/matrix_transform.hpp>

#ifdef _DEBUG
#pragma comment(lib, "freetype261_D.lib")
#else
#pragma comment(lib, "freetype261.lib")
#endif

using namespace oglt;

FreeTypeFont::FreeTypeFont()
{
	loaded = false;
}

/*-----------------------------------------------

Name:	CreateChar

Params:	iIndex - character index in Unicode.

Result:	Creates one single character (its
texture).

/*---------------------------------------------*/

inline int next_p2(int n) { int res = 1; while (res < n)res <<= 1; return res; }

void FreeTypeFont::createChar(int iIndex)
{
	FT_Load_Glyph(ftFace, FT_Get_Char_Index(ftFace, iIndex), FT_LOAD_DEFAULT);

	FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);
	FT_Bitmap* pBitmap = &ftFace->glyph->bitmap;

	int iW = pBitmap->width, iH = pBitmap->rows;
	int iTW = next_p2(iW), iTH = next_p2(iH);

	GLubyte* bData = new GLubyte[iTW*iTH];
	// Copy glyph data and add dark pixels elsewhere
	FOR(ch, iTH)FOR(cw, iTW)
		bData[ch*iTW + cw] = (ch >= iH || cw >= iW) ? 0 : pBitmap->buffer[(iH - ch - 1)*iW + cw];

	// And create a texture from it

	tCharTextures[iIndex].createFromData(bData, iTW, iTH, 8, GL_DEPTH_COMPONENT, false);
	tCharTextures[iIndex].setFiltering(TEXTURE_FILTER_MAG_BILINEAR, TEXTURE_FILTER_MIN_BILINEAR);

	tCharTextures[iIndex].setSamplerParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	tCharTextures[iIndex].setSamplerParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Calculate glyph data
	iAdvX[iIndex] = ftFace->glyph->advance.x >> 6;
	iBearingX[iIndex] = ftFace->glyph->metrics.horiBearingX >> 6;
	iCharWidth[iIndex] = ftFace->glyph->metrics.width >> 6;

	iAdvY[iIndex] = (ftFace->glyph->metrics.height - ftFace->glyph->metrics.horiBearingY) >> 6;
	iBearingY[iIndex] = ftFace->glyph->metrics.horiBearingY >> 6;
	iCharHeight[iIndex] = ftFace->glyph->metrics.height >> 6;

	iNewLine = max(iNewLine, int(ftFace->glyph->metrics.height >> 6));

	// Rendering data, texture coordinates are always the same, so now we waste a little memory
	glm::vec2 vQuad[] =
	{
		glm::vec2(0.0f, float(-iAdvY[iIndex] + iTH)),
		glm::vec2(0.0f, float(-iAdvY[iIndex])),
		glm::vec2(float(iTW), float(-iAdvY[iIndex] + iTH)),
		glm::vec2(float(iTW), float(-iAdvY[iIndex]))
	};
	glm::vec2 vTexQuad[] = { glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f) };

	// Add this char to VBO
	FOR(i, 4)
	{
		vboData.addData(&vQuad[i], sizeof(glm::vec2));
		vboData.addData(&vTexQuad[i], sizeof(glm::vec2));
	}
	delete[] bData;
}

/*-----------------------------------------------

Name:	LoadFont

Params:	sFile - path to font file
iPXSize - desired font pixel size

Result:	Loads whole font.

/*---------------------------------------------*/

bool FreeTypeFont::loadFont(string sFile, int iPXSize)
{
	bool bError = FT_Init_FreeType(&ftLib);

	bError = FT_New_Face(ftLib, sFile.c_str(), 0, &ftFace);
	if (bError)return false;
	FT_Set_Pixel_Sizes(ftFace, iPXSize, iPXSize);
	iLoadedPixelSize = iPXSize;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	vboData.createVBO();
	vboData.bindVBO();

	FOR(i, 128)createChar(i);
	loaded = true;

	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLib);

	vboData.uploadDataToGPU(GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2) * 2, (void*)(sizeof(glm::vec2)));
	return true;
}

/*-----------------------------------------------

Name:	LoadSystemFont

Params:	sName - system font name
iPXSize - desired font pixel size

Result:	Loads system font (from system Fonts
directory).

/*---------------------------------------------*/

bool FreeTypeFont::loadSystemFont(string sName, int iPXSize)
{
	char buf[512]; GetWindowsDirectoryA(buf, 512);
	string sPath = buf;
	sPath += "\\Fonts\\";
	sPath += sName;

	return loadFont(sPath, iPXSize);
}

/*-----------------------------------------------

Name:	GetTextWidth

Params:	sText - text to get width of
iPXSize - it's printed size

Result:	Returns width as number of pixels the
text will occupy.

/*---------------------------------------------*/

int FreeTypeFont::getTextWidth(string sText, int iPXSize)
{
	int iResult = 0;
	FOR(i, ESZ(sText))iResult += iAdvX[sText[i]];

	return iResult*iPXSize / iLoadedPixelSize;
}

/*-----------------------------------------------

Name:	Print

Params:	sText - text to print
x, y - 2D position
iPXSize - printed text size

Result:	Prints text at specified position
with specified pixel size.

/*---------------------------------------------*/

void FreeTypeFont::print(string sText, int x, int y, int iPXSize)
{
	TextField textField;
	textField.text = sText;
	textField.size = iPXSize;
	textField.x = x;
	textField.y = y;
	textFields.push_back(textField);
}

/*-----------------------------------------------

Name:	PrintFormatted

Params:	x, y - 2D position
iPXSize - printed text size
sText - text to print

Result:	Prints formatted text at specified position
with specified pixel size.

/*---------------------------------------------*/

void FreeTypeFont::printFormatted(int x, int y, int iPXSize, char* sText, ...)
{
	char buf[512];
	va_list ap;
	va_start(ap, sText);
	vsprintf(buf, sText, ap);
	va_end(ap);
	print(buf, x, y, iPXSize);
}

/*-----------------------------------------------

Name:	DeleteFont

Params:	none

Result:	Deletes all font textures.

/*---------------------------------------------*/

void FreeTypeFont::deleteFont()
{
	FOR(i, 128)tCharTextures[i].deleteTexture();
	vboData.deleteVBO();
	glDeleteVertexArrays(1, &vao);
}

void FreeTypeFont::render(int renderType)
{
	if (!loaded)return;

	glBindVertexArray(vao);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	FOR(i, ESZ(textFields)) {
		int iCurX = textFields[i].x, iCurY = textFields[i].y;
		int iPXSize = textFields[i].size;
		string sText = textFields[i].text;
		if (iPXSize == -1)iPXSize = iLoadedPixelSize;
		float fScale = float(iPXSize) / float(iLoadedPixelSize);
		FOR(j, ESZ(sText))
		{
			if (sText[j] == '\n')
			{
				iCurX = textFields[i].x;
				iCurY -= iNewLine*iPXSize / iLoadedPixelSize;
				continue;
			}
			int iIndex = int(sText[j]);
			iCurX += iBearingX[iIndex] * iPXSize / iLoadedPixelSize;
			if (sText[j] != ' ')
			{
				tCharTextures[iIndex].bindTexture();
				glm::mat4 mModelView = glm::translate(glm::mat4(1.0f), glm::vec3(float(iCurX), float(iCurY), 0.0f));
				mModelView = glm::scale(mModelView, glm::vec3(fScale));
				shaderProgram->setUniform("matrices.modelViewMatrix", mModelView);
				// Draw character
				glDrawArrays(GL_TRIANGLE_STRIP, iIndex * 4, 4);
			}

			iCurX += (iAdvX[iIndex] - iBearingX[iIndex])*iPXSize / iLoadedPixelSize;
		}
	}
	glDisable(GL_BLEND);
	textFields.clear();
}
