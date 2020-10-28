// Module:  2D graphics
// Company: RonIT
// Coder:   Adam Czupryna


#ifndef _AD2D_H
#define _AD2D_H

#define GL_GLEXT_PROTOTYPES

#include <windows.h>
#include <map>
#include <string>
#include <gd.h>

#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h>
#include "glext.h"

#include <d3d9.h>
#include <d3dx9tex.h>
#include <d3dx9core.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glaux.lib")
#pragma comment(lib,"libbgd.lib")

#pragma comment(lib, "d3d9.lib")


class AD2D_Window;
class AD2D_Viewport;
class AD2D_Image;


enum APIMode {
	API_OPENGL = 0,
	API_DIRECT3D = 1,
};


enum BLEND {
	BLEND_NONE = 0,
	BLEND_ZERO = 1,
	BLEND_ONE = 2,
	BLEND_SRCCOLOR = 3,
	BLEND_INVSRCCOLOR = 4,
	BLEND_SRCALPHA = 5,
	BLEND_INVSRCALPHA = 6,
	BLEND_DSTCOLOR = 7,
	BLEND_INVDSTCOLOR = 8,
	BLEND_DSTALPHA = 9,
	BLEND_INVDSTALPHA = 10,
};


struct COLOR {
	float red;
	float green;
	float blue;
	float alpha;

	COLOR() { red = 0.0f; green = 0.0f; blue = 0.0f; alpha = 0.0f; }
	COLOR(float r, float g, float b, float a = 1.0f) : red(r), green(g), blue(b), alpha(a) { };
};

struct D3DCUSTOMVERTEX {
    FLOAT x, y, z, rhw;
    DWORD color;
    FLOAT u, v;
};


class AD2D_Image {
private:
	unsigned short	realWidth;
	unsigned short	realHeight;
	unsigned short	virtualWidth;
	unsigned short	virtualHeight;

	float TexOffsetX;
	float TexOffsetY;

public:
	GLuint texture;
	LPDIRECT3DTEXTURE9 textureD3D;
	LPDIRECT3DSURFACE9 surfaceD3D;

public:
	AD2D_Image();
	~AD2D_Image();

	void AdjustSize(unsigned short width, unsigned short height);

	void Create(unsigned short width, unsigned short height, GLuint texdata);
	void Create(unsigned short width, unsigned short height, LPDIRECT3DTEXTURE9 texdata);
	void Create(unsigned short width, unsigned short height, unsigned char* data);
	void CreateBMP(const char* path);
	void CreateBMP_(unsigned char* data, long int size);
	void CreatePNG(const char* path);
	void CreatePNG_(unsigned char* data, long int size);
	void CreateGNP(const char* path);
	void CreateGNP_(unsigned char* data, long int size);
	void CreateAlphaMask(unsigned char r, unsigned char g, unsigned char b, unsigned char tolerance);
	void Update(unsigned char* data);
	unsigned char* GetData(unsigned char* data);

	unsigned short GetWidth();
	unsigned short GetHeight();

	friend class AD2D_Window;
	friend class AD2D_Font;
};

class AD2D_Font : public AD2D_Image {
private:
	unsigned short					realSize;
	std::map<unsigned char, int>	widthMap;

public:
	void ReadWidthMap(const char* path);
	void ReadWidthMap_(unsigned char* data, long int size);
	int GetCharWidth(unsigned char key);
	float GetCharWidth(unsigned char key, float fontSize);
	float GetTextWidth(std::string text, float fontSize);
	int GetTextWidthCharNumber(std::string text, float fontSize, float width);

	friend class AD2D_Window;
};


class AD2D_Viewport {
public:
	int	startX;
	int	startY;
	int	width;
	int	height;

public:
	AD2D_Viewport();
	void Create(int tX, int tY, int bX, int bY);

	bool operator==(AD2D_Viewport& vp);
	bool operator!=(AD2D_Viewport& vp);

	friend class AD2D_Window;
};


class AD2D_Window {
private:
	HWND*			_HWND;
	HDC				_HDC;
	HGLRC			_HRC;

	LPDIRECT3D9				_D3D;
	LPDIRECT3DDEVICE9		_D3DDEV;
	LPDIRECT3DVERTEXBUFFER9	_D3DVB;
	LPDIRECT3DSWAPCHAIN9 	_D3DSwapChain;
	LPDIRECT3DSURFACE9		_D3DBackBuffer;

	int				_PosX;
	int				_PosY;
	unsigned short	_ResX;
	unsigned short	_ResY;
	unsigned char	_BPP;

	bool			_minimized;
	bool			_maximized;

	AD2D_Viewport	_currentViewport;

public:
	static COLOR	_currentColor;

	static APIMode				_mode;
	static AD2D_Window*			_window;

public:
	AD2D_Window();
	AD2D_Window(HWND& hWnd, int PosX, int PosY, unsigned short ResX, unsigned short ResY, unsigned char BPP, APIMode mode = API_OPENGL);
	~AD2D_Window();

	void Create(HWND& hWnd, int PosX, int PosY, unsigned short ResX, unsigned short ResY, unsigned char BPP, APIMode mode = API_OPENGL);
	void Release();

	HDC GetHDC();
	HGLRC GetHRC();
	LPDIRECT3D9 GetD3D();
	LPDIRECT3DDEVICE9 GetD3DDEV();
	LPDIRECT3DVERTEXBUFFER9 GetD3DVB();
	LPDIRECT3DSURFACE9 GetD3DBackBuffer();

	void SetMaximized(bool state);
	bool GetMaximized();
	void SetMinimized(bool state);
	bool GetMinimized();

	static AD2D_Window* GetPointer();

	POINT GetWindowPos();
	POINT GetWindowSize();
	void MoveWindow(int PosX, int PosY);
	void ResizeWindow(unsigned short ResX, unsigned short ResY);

	void SetViewport(int startX, int startY, int width, int height);
	void SetViewport(AD2D_Viewport& viewport);
	AD2D_Viewport GetCurrentViewport();
	void PutPoint(float x, float y, float size = 1.0f);
	void PutLine(float x1, float y1, float x2, float y2, float size = 1.0f);
	void PutRect(float x1, float y1, float x2, float y2);
	void PutImage(int x, int y, AD2D_Image& image, unsigned int sampler = 0);
	void PutImage(float xs, float ys, float xf, float yf, AD2D_Image& image, unsigned int sampler = 0);
	void RotImage(float x, float y, float rad, float zoom, AD2D_Image& image, unsigned int sampler = 0);
	void Print(float x, float y, float size, AD2D_Font& font, const char* str, bool border = false);
	void PPrint(float x, float y, float size, AD2D_Font& font, const char* str, bool border = false, ...);
	void CPrint(float x, float y, float size, AD2D_Font& font, const char* str, const char* color, bool border = false);
	void ClearBuffers();
	void SwapBuffers();

	void ScreenShot(const char* path);

	static void SetBlendFunc(int BlendSrc, int BlendDst);
	static void SetColor(float R, float G, float B, float alpha = 1.0f);
	static COLOR GetColor();
	static void SetColorSTD(unsigned char color, float alpha = 1.0f);
	static COLOR ConvertColorSTD(unsigned char color);

	static COLOR STD2RGB(unsigned char color);
};


#endif
