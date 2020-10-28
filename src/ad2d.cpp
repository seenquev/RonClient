// Module:  2D graphics
// Company: RonIT
// Coder:   Adam Czupryna


#include "ad2d.h"

#include <iostream>
#include <algorithm>


AD2D_Window* AD2D_Window::_window = NULL;
APIMode AD2D_Window::_mode = API_OPENGL;

COLOR AD2D_Window::_currentColor = COLOR(1.0f, 1.0f, 1.0f, 1.0f);

// ---- AD2D_Window ---- //

/** \brief WINAPI window constructor (with default values)
 */
AD2D_Window::AD2D_Window() {
	_HWND = NULL;
	_HDC = NULL;
	_HRC = NULL;
	_D3D = NULL;
	_D3DDEV = NULL;
	_D3DVB = NULL;
	_D3DSwapChain = NULL;
	_D3DBackBuffer = NULL;

	_ResX = 0;
	_ResY = 0;
	_BPP = 0;

	_minimized = false;
	_maximized = false;

	_currentColor = COLOR(1, 1, 1, 1);
}

/** \brief WINAPI 2D window constructor
 *
 * \param hWnd HWND& - WINAPI window handler
 * \param PosX int - Absolute X position
 * \param PosY int - Absolute Y position
 * \param ResX unsigned short - Horizontal resolution
 * \param ResY unsigned short - Vertical resolution
 * \param BPP unsigned char - Bits per pixel
 * \param mode APIMode - API mode (API_OPENGL / API_DIRECT3D)
 *
 */
AD2D_Window::AD2D_Window(HWND& hWnd, int PosX, int PosY, unsigned short ResX, unsigned short ResY, unsigned char BPP, APIMode mode) {
	Create(hWnd, PosX, PosY, ResX, ResY, BPP, mode);
}

/** \brief WINAPI 2D window destructor (Releases 2D window memory)
 */
AD2D_Window::~AD2D_Window() {
	Release();
}

/** \brief WINAPI 2D window creation
 *
 * \param hWnd HWND& - WINAPI window handler
 * \param PosX int - Absolute X position
 * \param PosY int - Absolute Y position
 * \param ResX unsigned short - Horizontal resolution
 * \param ResY unsigned short - Vertical resolution
 * \param BPP unsigned char - Bits per pixel
 * \param mode APIMode - API mode (API_OPENGL / API_DIRECT3D)
 * \return void
 *
 */
void AD2D_Window::Create(HWND& hWnd, int PosX, int PosY, unsigned short ResX, unsigned short ResY, unsigned char BPP, APIMode mode) {
	_PosX = PosX;
	_PosY = PosY;
	_ResX = ResX;
	_ResY = ResY;
	_BPP = BPP;

	_minimized = false;
	_maximized = false;

	_mode = mode;

	RECT rcClient, rcWindow;
	POINT ptDiff;
	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	::MoveWindow(hWnd, PosX, PosY, ResX + ptDiff.x, ResY + ptDiff.y, true);

	_HWND = &hWnd;

	_window = this;

	if (_mode == API_OPENGL) {
		_HDC = GetDC(*_HWND);

		GLuint pixFormat;
		static PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),
			0,
			PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA,
			BPP,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			BPP,
			0, 0,
			PFD_MAIN_PLANE,
			0, 0, 0, 0
		};

		pixFormat = ChoosePixelFormat(_HDC, &pfd);
		SetPixelFormat(_HDC, pixFormat, &pfd);

		_HRC = wglCreateContext(_HDC);
		wglMakeCurrent(_HDC, _HRC);

		glShadeModel(GL_SMOOTH);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);

		glDepthFunc(GL_LEQUAL);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
	}
	else if (_mode == API_DIRECT3D) {
		D3DPRESENT_PARAMETERS d3dpp;

		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.BackBufferCount = 1;
		d3dpp.BackBufferWidth = ResX;
		d3dpp.BackBufferHeight = ResY;
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		d3dpp.hDeviceWindow = hWnd;

		_D3D = Direct3DCreate9(D3D_SDK_VERSION);
		_D3D->CreateDevice(D3DADAPTER_DEFAULT,
						  D3DDEVTYPE_HAL,
						  hWnd,
						  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
						  &d3dpp,
						  &_D3DDEV);

		_D3DDEV->CreateAdditionalSwapChain(&d3dpp, &_D3DSwapChain);
		_D3DSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &_D3DBackBuffer);
		_D3DDEV->SetRenderTarget(0, _D3DBackBuffer);

		_D3DDEV->CreateVertexBuffer(65536 * sizeof(D3DCUSTOMVERTEX), 0, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, D3DPOOL_MANAGED, &_D3DVB, NULL);
		_D3DDEV->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		_D3DDEV->SetStreamSource(0, _D3DVB, 0, sizeof(D3DCUSTOMVERTEX));

		_D3DDEV->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		_D3DDEV->SetRenderState(D3DRS_LIGHTING, false);
		_D3DDEV->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		_D3DDEV->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		_D3DDEV->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		_D3DDEV->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    	_D3DDEV->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		_D3DDEV->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		_D3DDEV->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

		_D3DDEV->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		_D3DDEV->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		_D3DDEV->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		_D3DDEV->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		_D3DDEV->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		_D3DDEV->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

		_D3DDEV->BeginScene();
	}
}

/** \brief Releases WINAPI 2D window memory
 *
 * \return void
 *
 */
void AD2D_Window::Release() {
	if (_mode == API_OPENGL) {
		if (_HDC)
			ReleaseDC(*_HWND, _HDC);
	}
	else if (_mode == API_DIRECT3D) {
		_D3DDEV->EndScene();

		if (_D3DBackBuffer)
			_D3DBackBuffer->Release();
		if (_D3DSwapChain)
			_D3DSwapChain->Release();
		if (_D3DVB)
			_D3DVB->Release();
		if (_D3DDEV)
			_D3DDEV->Release();
		if (_D3D)
			_D3D->Release();
	}
}

HDC AD2D_Window::GetHDC() {
	return _HDC;
}

HGLRC AD2D_Window::GetHRC() {
	return _HRC;
}

LPDIRECT3D9 AD2D_Window::GetD3D() {
	return _D3D;
}

LPDIRECT3DDEVICE9 AD2D_Window::GetD3DDEV() {
	return _D3DDEV;
}

LPDIRECT3DVERTEXBUFFER9 AD2D_Window::GetD3DVB() {
	return _D3DVB;
}

LPDIRECT3DSURFACE9 AD2D_Window::GetD3DBackBuffer() {
	return _D3DBackBuffer;
}

/** \brief Set WINAPI 2D window maximized state
 *
 * \param state bool
 * \return void
 *
 */
void AD2D_Window::SetMaximized(bool state) {
	_maximized = state;
}

/** \brief Get WINAPI 2D window maximized state
 *
 * \return bool
 *
 */
bool AD2D_Window::GetMaximized() {
	return _maximized;
}

/** \brief Set WINAPI 2D window minimize state
 *
 * \param state bool
 * \return void
 *
 */
void AD2D_Window::SetMinimized(bool state) {
	_minimized = state;
}

/** \brief Get WINAPI 2Dwindow minimize state
 *
 * \return bool
 *
 */
bool AD2D_Window::GetMinimized() {
	return _minimized;
}


/** \brief Get current WINAPI 2D window pointer
 *
 * \return AD2D_Window*
 *
 */
AD2D_Window* AD2D_Window::GetPointer() {
	return _window;
}

/** \brief Get current WINAPI 2D window position
 *
 * \return POINT
 *
 */
POINT AD2D_Window::GetWindowPos() {
	POINT pos = {_PosX, _PosY};
	return pos;
}

/** \brief Get current WINAPI 2D window size
 *
 * \return POINT
 *
 */
POINT AD2D_Window::GetWindowSize() {
	POINT size = {_ResX, _ResY};
	return size;
}

/** \brief Set WINAPI 2D window position (use only after WINAPI window moveing message)
 *
 * \param PosX int
 * \param PosY int
 * \return void
 *
 */
void AD2D_Window::MoveWindow(int PosX, int PosY) {
	_PosX = PosX;
	_PosY = PosY;
}

/** \brief Set WINAPI 2D window size (use only after WINAPI window resize message)
 *
 * \param ResX unsigned short
 * \param ResY unsigned short
 * \return void
 *
 */
void AD2D_Window::ResizeWindow(unsigned short ResX, unsigned short ResY) {
	_ResX = ResX;
	_ResY = ResY;
	_currentViewport.Create(0, 0, ResX - 1, ResY - 1);
	SetViewport(_currentViewport);

	if (_mode == API_DIRECT3D) {
		_D3DDEV->EndScene();

		D3DPRESENT_PARAMETERS d3dpp;

		_D3DSwapChain->GetPresentParameters(&d3dpp);
		d3dpp.BackBufferWidth = ResX;
		d3dpp.BackBufferHeight = ResY;

		_D3DBackBuffer->Release();
		_D3DBackBuffer = NULL;
		_D3DSwapChain->Release();
		_D3DSwapChain = NULL;

		_D3DDEV->CreateAdditionalSwapChain(&d3dpp, &_D3DSwapChain);
		_D3DSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &_D3DBackBuffer);
		_D3DDEV->SetRenderTarget(0, _D3DBackBuffer);

		_D3DDEV->BeginScene();
		//_D3DDEV->Reset(&d3dpp);
	}
}

/** \brief Set active 2D viewport inside WINAPI window
 *
 * \param startX int
 * \param startY int
 * \param width int
 * \param height int
 * \return void
 *
 */
void AD2D_Window::SetViewport(int startX, int startY, int width, int height) {
	AD2D_Viewport viewport;
	viewport.Create(startX, startY, startX + width - 1, startY + height - 1);
	SetViewport(viewport);
}

/** \brief Set active 2D viewport inside WINAPI window
 *
 * \param viewport AD2D_Viewport&
 * \return void
 *
 */
void AD2D_Window::SetViewport(AD2D_Viewport& viewport) {
	_currentViewport = viewport;

	if (_mode == API_OPENGL) {
		glViewport(viewport.startX, _ResY - viewport.height - viewport.startY, viewport.width, viewport.height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		gluOrtho2D(0.0f, (GLfloat)viewport.width, (GLfloat)viewport.height, 0.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	else if (_mode == API_DIRECT3D) {
		int posX = viewport.startX;
		int posY = viewport.startY;
		int width = viewport.width;
		int height = viewport.height;
		if (posX < 0) { width += posX; posX = 0; }
		if (posY < 0) { height += posY; posY = 0; }
		if (width < 0) width = 0;
		if (height < 0) height = 0;

		D3DVIEWPORT9 viewData = {posX, posY, width, height, 0.0f, 0.0f};
		_D3DDEV->SetViewport(&viewData);
	}
}

/** \brief Get active 2D viewport from WINAPI window
 *
 * \return AD2D_Viewport
 *
 */
AD2D_Viewport AD2D_Window::GetCurrentViewport() {
	return _currentViewport;
};

/** \brief Draw point at active viewport
 *
 * \param x float
 * \param y float
 * \param size float
 * \return void
 *
 */
void AD2D_Window::PutPoint(float x, float y, float size) {
	if (_mode == API_OPENGL) {
		glPointSize(size);
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_POINT_SMOOTH);
		glBegin(GL_POINTS);
			glVertex2d(x, y);
		glEnd();
		glDisable(GL_POINT_SMOOTH);
		glEnable(GL_TEXTURE_2D);
	}
	else if (_mode == API_DIRECT3D) {
		DWORD color = D3DCOLOR_COLORVALUE(_currentColor.red, _currentColor.green, _currentColor.blue, _currentColor.alpha);
		D3DCUSTOMVERTEX vertices[] = {
				{_currentViewport.startX + x, _currentViewport.startY + y, 0.0f, 1.0f, color, 0.0f, 0.0f, } };
		void* pVertices;
		_D3DVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0);
		memcpy(pVertices, vertices, sizeof(vertices));
		_D3DVB->Unlock();

		//_D3DDEV->BeginScene();
		_D3DDEV->SetTexture(0, NULL);
		_D3DDEV->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&size));
		_D3DDEV->DrawPrimitive(D3DPT_POINTLIST, 0, 1);
		//_D3DDEV->EndScene();
	}
}

/** \brief Draw line at active viewport
 *
 * \param x1 float
 * \param y1 float
 * \param x2 float
 * \param y2 float
 * \param size float
 * \return void
 *
 */
void AD2D_Window::PutLine(float x1, float y1, float x2, float y2, float size) {
	if (_mode == API_OPENGL) {
		glLineWidth(size);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
			glVertex2d(x1, y1);
			glVertex2d(x2, y2);
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
	else if (_mode == API_DIRECT3D) {
		LPD3DXLINE line;
		D3DXVECTOR2 points[2];
		points[0] = D3DXVECTOR2(x1, y1);
		points[1] = D3DXVECTOR2(x2, y2);
		D3DXCreateLine(_D3DDEV, &line);
		line->SetWidth(size);
		line->Begin();
		line->Draw(points, 2, D3DCOLOR_COLORVALUE(_currentColor.red, _currentColor.green, _currentColor.blue, _currentColor.alpha));
		line->End();
		line->Release();
	}
}

/** \brief Draw rectangle at active viewport
 *
 * \param x1 float
 * \param y1 float
 * \param x2 float
 * \param y2 float
 * \return void
 *
 */
void AD2D_Window::PutRect(float x1, float y1, float x2, float y2) {
	if (_mode == API_OPENGL) {
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glVertex2d(x1, y1);
			glVertex2d(x2, y1);
			glVertex2d(x2, y2);
			glVertex2d(x1, y2);
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
	else if (_mode == API_DIRECT3D) {
		DWORD color = D3DCOLOR_COLORVALUE(_currentColor.red, _currentColor.green, _currentColor.blue, _currentColor.alpha);
		D3DCUSTOMVERTEX vertices[] = {
				{_currentViewport.startX + x1, _currentViewport.startY + y1, 0.0f, 1.0f, color, 0.0f, 0.0f, },
				{_currentViewport.startX + x2, _currentViewport.startY + y1, 0.0f, 1.0f, color, 0.0f, 0.0f, },
				{_currentViewport.startX + x2, _currentViewport.startY + y2, 0.0f, 1.0f, color, 0.0f, 0.0f, },
				{_currentViewport.startX + x1, _currentViewport.startY + y2, 0.0f, 1.0f, color, 0.0f, 0.0f, } };
		void* pVertices;
		_D3DVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0);
		memcpy(pVertices, vertices, sizeof(vertices));
		_D3DVB->Unlock();

		//_D3DDEV->BeginScene();
		_D3DDEV->SetTexture(0, NULL);
		_D3DDEV->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
		//_D3DDEV->EndScene();
	}
}

/** \brief Draw image at active viewport
 *
 * \param x int
 * \param y int
 * \param image AD2D_Image&
 * \param sampler unsigned int
 * \return void
 *
 */
void AD2D_Window::PutImage(int x, int y, AD2D_Image& image, unsigned int sampler) {
	PutImage(x, y, x + image.GetWidth(), y + image.GetHeight(), image, sampler);
}

/** \brief Draw image at active viewport
 *
 * \param xs float
 * \param ys float
 * \param xf float
 * \param yf float
 * \param image AD2D_Image&
 * \param sampler unsigned int
 * \return void
 *
 */
void AD2D_Window::PutImage(float xs, float ys, float xf, float yf, AD2D_Image& image, unsigned int sampler) {
	float CoordX1 = 0.0f + image.TexOffsetX;
	float CoordX2 = (float)(image.virtualWidth) * (1.0f / image.realWidth) + image.TexOffsetX;
	float CoordY1 = 0.0f + image.TexOffsetY;
	float CoordY2 = (float)(image.virtualHeight) * (1.0f / image.realHeight) + image.TexOffsetY;

	if (_mode == API_OPENGL) {
		glBindTexture(GL_TEXTURE_2D,image.texture);
		glBegin(GL_QUADS);
			glTexCoord2d(CoordX1, CoordY1);		glVertex2f(xs, ys);
			glTexCoord2d(CoordX2, CoordY1);		glVertex2f(xf, ys);
			glTexCoord2d(CoordX2, CoordY2);		glVertex2f(xf, yf);
			glTexCoord2d(CoordX1, CoordY2);		glVertex2f(xs, yf);
		glEnd();
	}
	else if (_mode == API_DIRECT3D) {
		DWORD color = D3DCOLOR_COLORVALUE(_currentColor.red, _currentColor.green, _currentColor.blue, _currentColor.alpha);

		D3DCUSTOMVERTEX* pVertices;
		_D3DVB->Lock(0, 4 * sizeof(D3DCUSTOMVERTEX), (void**)&pVertices, 0);
		pVertices[0] = {_currentViewport.startX + xs, _currentViewport.startY + ys, 0.0f, 1.0f, color, CoordX1, CoordY1 };
		pVertices[1] = {_currentViewport.startX + xf, _currentViewport.startY + ys, 0.0f, 1.0f, color, CoordX2, CoordY1 };
		pVertices[2] = {_currentViewport.startX + xf, _currentViewport.startY + yf, 0.0f, 1.0f, color, CoordX2, CoordY2 };
		pVertices[3] = {_currentViewport.startX + xs, _currentViewport.startY + yf, 0.0f, 1.0f, color, CoordX1, CoordY2 };
		_D3DVB->Unlock();

		//_D3DDEV->BeginScene();
		_D3DDEV->SetTexture(sampler, image.textureD3D);
		_D3DDEV->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
		//_D3DDEV->EndScene();
	}
}

/** \brief Draw rotated, zoomed image with center at point (x, y) at active viewport
 *
 * \param x float
 * \param y float
 * \param rad float
 * \param zoom float
 * \param image AD2D_Image&
 * \param sampler unsigned int
 * \return void
 *
 */
void AD2D_Window::RotImage(float x, float y, float rad, float zoom, AD2D_Image& image, unsigned int sampler) {
	float CoordX1 = 0.0f + image.TexOffsetX;
	float CoordX2 = (float)image.virtualWidth * (1.0f / image.realWidth) + image.TexOffsetX;
	float CoordY1 = 0.0f + image.TexOffsetY;
	float CoordY2 = (float)image.virtualHeight * (1.0f / image.realHeight) + image.TexOffsetY;

	if (_mode == API_OPENGL) {
		glBindTexture(GL_TEXTURE_2D,image.texture);
		glTranslatef(x, y, 0);
		glRotatef(rad,0,0,1);
		glTranslatef(-(float)image.virtualWidth * zoom / 2, -(float)image.virtualHeight * zoom / 2, 0);
		glBegin(GL_QUADS);
			glTexCoord2d(CoordX1, CoordY1);		glVertex2f(0, 0);
			glTexCoord2d(CoordX2, CoordY1);		glVertex2f((float)image.virtualWidth * zoom, 0);
			glTexCoord2d(CoordX2, CoordY2);		glVertex2f((float)image.virtualWidth * zoom, (float)image.virtualHeight * zoom);
			glTexCoord2d(CoordX1, CoordY2);		glVertex2f(0, (float)image.virtualHeight * zoom);
		glEnd();
		glTranslatef((float)image.virtualWidth * zoom / 2, (float)image.virtualHeight * zoom / 2, 0);
		glRotatef(rad,0,0,-1);
		glTranslatef(-x, -y, 0);
	}
	else if (_mode == API_DIRECT3D) {
		float RAD = (rad * 3.141592654) / 180;

		DWORD color = D3DCOLOR_COLORVALUE(_currentColor.red, _currentColor.green, _currentColor.blue, _currentColor.alpha);
		D3DCUSTOMVERTEX vertices[] = {
				{-(float)image.virtualWidth * zoom / 2, -(float)image.virtualHeight * zoom / 2, 0.0f, 1.0f, color, CoordX1, CoordY1, },
				{(float)image.virtualWidth * zoom / 2, -(float)image.virtualHeight * zoom / 2, 0.0f, 1.0f, color, CoordX2, CoordY1, },
				{(float)image.virtualWidth * zoom / 2, (float)image.virtualHeight * zoom / 2, 0.0f, 1.0f, color, CoordX2, CoordY2, },
				{-(float)image.virtualWidth * zoom / 2, (float)image.virtualHeight * zoom / 2, 0.0f, 1.0f, color, CoordX1, CoordY2, } };
		void* pVertices;

		D3DXMATRIX rotate;
		D3DXMATRIX move;
		D3DXMatrixRotationZ(&rotate, RAD);
		D3DXMatrixTranslation(&move, _currentViewport.startX + x, _currentViewport.startY + y, 0);
		for (int i = 0; i < 4; i++) {
            D3DXMATRIX product = rotate * move;
			D3DXVec3TransformCoord((D3DXVECTOR3*)&vertices[i], (D3DXVECTOR3*)&vertices[i], &product);
		}

		_D3DVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0);
		memcpy(pVertices, vertices, sizeof(vertices));
		_D3DVB->Unlock();

		//_D3DDEV->BeginScene();
		_D3DDEV->SetTexture(sampler, image.textureD3D);
		_D3DDEV->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
		//_D3DDEV->EndScene();
	}
}


/** \brief Draw text at active viewport
 *
 * \param x float
 * \param y float
 * \param size float
 * \param font AD2D_Font&
 * \param str const char*
 * \param border bool
 * \return void
 *
 */
void AD2D_Window::Print(float x, float y, float size, AD2D_Font& font, const char* str, bool border)
{
	float factor_x = ((float)16 / 256) * ((float)font.virtualWidth / font.realWidth);
	float factor_y = ((float)16 / 256) * ((float)font.virtualHeight / font.realHeight);

	char text[2048];
	strcpy(text, str);

	COLOR currentColor = GetColor();

	if (_mode == API_OPENGL) {
		glBindTexture(GL_TEXTURE_2D, font.texture);
		glBegin(GL_QUADS);
		float pos = 0.0f;
		if (border) {
			glColor4f(0.0f * currentColor.red, 0.0f * currentColor.green, 0.0f * currentColor.blue, currentColor.alpha);
			for (int l = 0; l < (int)strlen(text); l++) {
				int n = (unsigned char)text[l];
				float width = (float)font.GetCharWidth(n);
				float inc = (float)(width * size) / font.realSize;

				float CoordX1 = ((n % 16) + 0) * factor_x;
				float CoordX2 = ((float)(n % 16) + (width / font.realSize)) * factor_x;
				float CoordY1 = ((n / 16) + 0) * factor_y;
				float CoordY2 = ((n / 16) + 1) * factor_y;

				glTexCoord2f(CoordX1, CoordY1);		glVertex2d(x + pos - 1,y);
				glTexCoord2f(CoordX2, CoordY1);		glVertex2d(x + pos + inc - 1,y);
				glTexCoord2f(CoordX2, CoordY2);		glVertex2d(x + pos + inc - 1,y + size);
				glTexCoord2f(CoordX1, CoordY2);		glVertex2d(x + pos - 1,y + size);

				glTexCoord2f(CoordX1, CoordY1);		glVertex2d(x + pos + 1,y);
				glTexCoord2f(CoordX2, CoordY1);		glVertex2d(x + pos + inc + 1,y);
				glTexCoord2f(CoordX2, CoordY2);		glVertex2d(x + pos + inc + 1,y + size);
				glTexCoord2f(CoordX1, CoordY2);		glVertex2d(x + pos + 1,y + size);

				glTexCoord2f(CoordX1, CoordY1);		glVertex2d(x + pos,y - 1);
				glTexCoord2f(CoordX2, CoordY1);		glVertex2d(x + pos + inc,y - 1);
				glTexCoord2f(CoordX2, CoordY2);		glVertex2d(x + pos + inc,y + size - 1);
				glTexCoord2f(CoordX1, CoordY2);		glVertex2d(x + pos,y + size - 1);

				glTexCoord2f(CoordX1, CoordY1);		glVertex2d(x + pos,y + 1);
				glTexCoord2f(CoordX2, CoordY1);		glVertex2d(x + pos + inc,y + 1);
				glTexCoord2f(CoordX2, CoordY2);		glVertex2d(x + pos + inc,y + size + 1);
				glTexCoord2f(CoordX1, CoordY2);		glVertex2d(x + pos,y + size + 1);
				pos += inc;
			}
			glColor4f(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
			pos = 0.0f;
		}

		for (int l = 0; l < (int)strlen(text); l++)
		{
			int n = (unsigned char)text[l];
			float width = (float)font.GetCharWidth(n);
			float inc = (float)(width * size) / font.realSize;

			float CoordX1 = ((n % 16) + 0) * factor_x;
			float CoordX2 = ((float)(n % 16) + (width / font.realSize)) * factor_x;
			float CoordY1 = ((n / 16) + 0) * factor_y;
			float CoordY2 = ((n / 16) + 1) * factor_y;

			glTexCoord2f(CoordX1, CoordY1);		glVertex2d(x + pos,y);
			glTexCoord2f(CoordX2, CoordY1);		glVertex2d(x + pos + inc,y);
			glTexCoord2f(CoordX2, CoordY2);		glVertex2d(x + pos + inc,y + size);
			glTexCoord2f(CoordX1, CoordY2);		glVertex2d(x + pos,y + size);
			pos += inc;
		}
		glEnd();
	}
	else if (_mode == API_DIRECT3D) {
		int vNum = 0;
		int vSize = sizeof(D3DCUSTOMVERTEX) * strlen(text) * (border ? 20 : 4);

		D3DCUSTOMVERTEX* pVertices;
		_D3DVB->Lock(0, vSize, (void**)&pVertices, 0);

		float pos = 0.0f;
		if (border) {
			DWORD color = D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, currentColor.alpha);
			for (int l = 0; l < (int)strlen(text); l++) {
				int n = (unsigned char)text[l];
				float width = (float)font.GetCharWidth(n);
				float inc = (float)(width * size) / font.realSize;

				float CoordX1 = ((n % 16) + 0) * factor_x + font.TexOffsetX;
				float CoordX2 = ((float)(n % 16) + (width / font.realSize)) * factor_x + font.TexOffsetX;
				float CoordY1 = ((n / 16) + 0) * factor_y + font.TexOffsetY;
				float CoordY2 = ((n / 16) + 1) * factor_y + font.TexOffsetY;

				pVertices[vNum++] = { _currentViewport.startX + x + pos - 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc - 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX2, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc - 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos - 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc - 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos - 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX1, CoordY2 };

				pVertices[vNum++] = { _currentViewport.startX + x + pos + 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc + 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX2, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc + 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc + 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX1, CoordY2 };

				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y - 1, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y - 1, 0.0f, 1.0f, color, CoordX2, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size - 1, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y - 1, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size - 1, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y + size - 1, 0.0f, 1.0f, color, CoordX1, CoordY2 };

				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y + 1, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + 1, 0.0f, 1.0f, color, CoordX2, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size + 1, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y + 1, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size + 1, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y + size + 1, 0.0f, 1.0f, color, CoordX1, CoordY2 };

				pos += inc;
			}
			pos = 0.0f;
		}

		DWORD color = D3DCOLOR_COLORVALUE(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
		for (int l = 0; l < (int)strlen(text); l++)
		{
			int n = (unsigned char)text[l];
			float width = (float)font.GetCharWidth(n);
			float inc = (float)(width * size) / font.realSize;

			float CoordX1 = ((n % 16) + 0) * factor_x + font.TexOffsetX;
			float CoordX2 = ((float)(n % 16) + (width / font.realSize)) * factor_x + font.TexOffsetX;
			float CoordY1 = ((n / 16) + 0) * factor_y + font.TexOffsetY;
			float CoordY2 = ((n / 16) + 1) * factor_y + font.TexOffsetY;

			pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1, };
			pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX2, CoordY1, };
			pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2, };

			pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1, };
			pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2, };
			pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX1, CoordY2, };

			pos += inc;
		}
		_D3DVB->Unlock();

		//_D3DDEV->BeginScene();
		_D3DDEV->SetTexture(0, font.textureD3D);
		_D3DDEV->DrawPrimitive(D3DPT_TRIANGLELIST, 0, vNum / 3);
		//_D3DDEV->EndScene();
	}
}


/** \brief Draw text with parameters at active viewport
 *
 * \param x float
 * \param y float
 * \param size float
 * \param font AD2D_Font&
 * \param str const char*
 * \param border bool
 * \param ...
 * \return void
 *
 */
void AD2D_Window::PPrint(float x, float y, float size, AD2D_Font& font, const char* str, bool border, ...)
{
	float factor_x = ((float)16 / 256) * ((float)font.virtualWidth / font.realWidth);
	float factor_y = ((float)16 / 256) * ((float)font.virtualHeight / font.realHeight);

	char text[2048];
	va_list vl;

	va_start(vl,border);
	vsprintf(text,str,vl);
	va_end(vl);

	Print(x, y, size, font, text, border);
}


/** \brief Draw coloured text at active viewport
 *
 * \param x float
 * \param y float
 * \param size float
 * \param font AD2D_Font&
 * \param str const char*
 * \param color const char*
 * \param border bool
 * \return void
 *
 */
void AD2D_Window::CPrint(float x, float y, float size, AD2D_Font& font, const char* str, const char* color, bool border)
{
	float factor_x = ((float)16 / 256) * ((float)font.virtualWidth / font.realWidth);
	float factor_y = ((float)16 / 256) * ((float)font.virtualHeight / font.realHeight);

	char text[2048];
	strcpy(text, str);

	COLOR currentColor = GetColor();

	if (_mode == API_OPENGL) {
		glBindTexture(GL_TEXTURE_2D, font.texture);
		glBegin(GL_QUADS);
		float pos = 0.0f;
		if (border) {
			glColor4f(0.0f * currentColor.red, 0.0f * currentColor.green, 0.0f * currentColor.blue, currentColor.alpha);
			for (int l = 0; l < (int)strlen(text); l++) {
				int n = (unsigned char)text[l];
				float width = (float)font.GetCharWidth(n);
				float inc = (float)(width * size) / font.realSize;

				float CoordX1 = ((n % 16) + 0) * factor_x;
				float CoordX2 = ((float)(n % 16) + (width / font.realSize)) * factor_x;
				float CoordY1 = ((n / 16) + 0) * factor_y;
				float CoordY2 = ((n / 16) + 1) * factor_y;

				glTexCoord2f(CoordX1, CoordY1);		glVertex2d(x + pos - 1,y);
				glTexCoord2f(CoordX2, CoordY1);		glVertex2d(x + pos + inc - 1,y);
				glTexCoord2f(CoordX2, CoordY2);		glVertex2d(x + pos + inc - 1,y + size);
				glTexCoord2f(CoordX1, CoordY2);		glVertex2d(x + pos - 1,y + size);

				glTexCoord2f(CoordX1, CoordY1);		glVertex2d(x + pos + 1,y);
				glTexCoord2f(CoordX2, CoordY1);		glVertex2d(x + pos + inc + 1,y);
				glTexCoord2f(CoordX2, CoordY2);		glVertex2d(x + pos + inc + 1,y + size);
				glTexCoord2f(CoordX1, CoordY2);		glVertex2d(x + pos + 1,y + size);

				glTexCoord2f(CoordX1, CoordY1);		glVertex2d(x + pos,y - 1);
				glTexCoord2f(CoordX2, CoordY1);		glVertex2d(x + pos + inc,y - 1);
				glTexCoord2f(CoordX2, CoordY2);		glVertex2d(x + pos + inc,y + size - 1);
				glTexCoord2f(CoordX1, CoordY2);		glVertex2d(x + pos,y + size - 1);

				glTexCoord2f(CoordX1, CoordY1);		glVertex2d(x + pos,y + 1);
				glTexCoord2f(CoordX2, CoordY1);		glVertex2d(x + pos + inc,y + 1);
				glTexCoord2f(CoordX2, CoordY2);		glVertex2d(x + pos + inc,y + size + 1);
				glTexCoord2f(CoordX1, CoordY2);		glVertex2d(x + pos,y + size + 1);
				pos += inc;
			}
			glColor4f(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
			pos = 0.0f;
		}

		for (int l = 0; l < (int)strlen(text); l++)
		{
			int n = (unsigned char)text[l];
			float width = (float)font.GetCharWidth(n);
			float inc = (float)(width * size) / font.realSize;

			float CoordX1 = ((n % 16) + 0) * factor_x;
			float CoordX2 = ((float)(n % 16) + (width / font.realSize)) * factor_x;
			float CoordY1 = ((n / 16) + 0) * factor_y;
			float CoordY2 = ((n / 16) + 1) * factor_y;

			COLOR charColor = ConvertColorSTD(color[l]);
			glColor4f(charColor.red * currentColor.red, charColor.green * currentColor.green, charColor.blue * currentColor.blue, currentColor.alpha);

			glTexCoord2f(CoordX1, CoordY1);		glVertex2d(x + pos,y);
			glTexCoord2f(CoordX2, CoordY1);		glVertex2d(x + pos + inc,y);
			glTexCoord2f(CoordX2, CoordY2);		glVertex2d(x + pos + inc,y + size);
			glTexCoord2f(CoordX1, CoordY2);		glVertex2d(x + pos,y + size);
			pos += inc;
		}
		glEnd();
	}
	else if (_mode == API_DIRECT3D) {
		int vNum = 0;
		int vSize = sizeof(D3DCUSTOMVERTEX) * strlen(text) * (border ? 20 : 4);

		D3DCUSTOMVERTEX* pVertices;
		_D3DVB->Lock(0, vSize, (void**)&pVertices, 0);

		float pos = 0.0f;
		if (border) {
			DWORD color = D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, currentColor.alpha);
			for (int l = 0; l < (int)strlen(text); l++) {
				int n = (unsigned char)text[l];
				float width = (float)font.GetCharWidth(n);
				float inc = (float)(width * size) / font.realSize;

				float CoordX1 = ((n % 16) + 0) * factor_x + font.TexOffsetX;
				float CoordX2 = ((float)(n % 16) + (width / font.realSize)) * factor_x + font.TexOffsetX;
				float CoordY1 = ((n / 16) + 0) * factor_y + font.TexOffsetY;
				float CoordY2 = ((n / 16) + 1) * factor_y + font.TexOffsetY;

				pVertices[vNum++] = { _currentViewport.startX + x + pos - 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc - 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX2, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc - 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos - 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc - 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos - 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX1, CoordY2 };

				pVertices[vNum++] = { _currentViewport.startX + x + pos + 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc + 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX2, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc + 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + 1, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc + 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + 1, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX1, CoordY2 };

				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y - 1, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y - 1, 0.0f, 1.0f, color, CoordX2, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size - 1, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y - 1, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size - 1, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y + size - 1, 0.0f, 1.0f, color, CoordX1, CoordY2 };

				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y + 1, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + 1, 0.0f, 1.0f, color, CoordX2, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size + 1, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y + 1, 0.0f, 1.0f, color, CoordX1, CoordY1 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size + 1, 0.0f, 1.0f, color, CoordX2, CoordY2 };
				pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y + size + 1, 0.0f, 1.0f, color, CoordX1, CoordY2 };

				pos += inc;
			}
			pos = 0.0f;
		}

		for (int l = 0; l < (int)strlen(text); l++)
		{
			int n = (unsigned char)text[l];
			float width = (float)font.GetCharWidth(n);
			float inc = (float)(width * size) / font.realSize;

			float CoordX1 = ((n % 16) + 0) * factor_x + font.TexOffsetX;
			float CoordX2 = ((float)(n % 16) + (width / font.realSize)) * factor_x + font.TexOffsetX;
			float CoordY1 = ((n / 16) + 0) * factor_y + font.TexOffsetY;
			float CoordY2 = ((n / 16) + 1) * factor_y + font.TexOffsetY;

			COLOR charColor = ConvertColorSTD(color[l]);
			DWORD color = D3DCOLOR_COLORVALUE(charColor.red * currentColor.red,
					charColor.green * currentColor.green,
					charColor.blue * currentColor.blue,
					charColor.alpha * currentColor.alpha);

			pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1, };
			pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX2, CoordY1, };
			pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2, };

			pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y, 0.0f, 1.0f, color, CoordX1, CoordY1, };
			pVertices[vNum++] = { _currentViewport.startX + x + pos + inc, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX2, CoordY2, };
			pVertices[vNum++] = { _currentViewport.startX + x + pos, _currentViewport.startY + y + size, 0.0f, 1.0f, color, CoordX1, CoordY2, };

			pos += inc;
		}
		_D3DVB->Unlock();

		//_D3DDEV->BeginScene();
		_D3DDEV->SetTexture(0, font.textureD3D);
		_D3DDEV->DrawPrimitive(D3DPT_TRIANGLELIST, 0, vNum / 3);
		//_D3DDEV->EndScene();
	}
}


/** \brief Clear color and depth buffers at WINAPI 2D window
 *
 * \return void
 *
 */
void AD2D_Window::ClearBuffers() {
	if (_mode == API_OPENGL) {
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
	}
	else if (_mode == API_DIRECT3D) {
		_D3DDEV->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 0.0f, 0);
	}
}

/** \brief Swap back and front buffers at WINAPI 2D window
 *
 * \return void
 *
 */
void AD2D_Window::SwapBuffers() {
	if (_mode == API_OPENGL) {
		::SwapBuffers(_HDC);
	}
	else if (_mode == API_DIRECT3D) {
		_D3DDEV->EndScene();
		_D3DSwapChain->Present(NULL, NULL, NULL, NULL, NULL);
		_D3DDEV->BeginScene();
	}
}

/** \brief Save WINAPI 2D window screen shot
 *
 * \param path const char*
 * \return void
 *
 */
void AD2D_Window::ScreenShot(const char* path) {
	unsigned char* data = new unsigned char[_ResX * _ResY * 4];

	if (_mode == API_OPENGL) {
		glReadPixels(0, 0, _ResX, _ResY, GL_RGBA, GL_UNSIGNED_BYTE, data);

		FILE* file;
		file = fopen(path, "wb");
		if (file) {
			gdImagePtr image = gdImageCreateTrueColor(_ResX, _ResY);
			for (int x = 0; x < gdImageSX(image); x++)
			for (int y = 0; y < gdImageSY(image); y++) {
				unsigned char R = data[(x + y * _ResX) * 4 + 0];
				unsigned char G = data[(x + y * _ResX) * 4 + 1];
				unsigned char B = data[(x + y * _ResX) * 4 + 2];
				gdImageSetPixel(image, x, gdImageSY(image) - y - 1, gdTrueColor(R, G, B));
			}

			gdImageJpeg(image, file, 90);
			gdImageDestroy(image);
			fclose(file);
		}
	}
	else if (_mode == API_DIRECT3D)
		D3DXSaveSurfaceToFile(path, D3DXIFF_JPG, _D3DBackBuffer, NULL, NULL);

	delete[] data;
}

uint32_t ConvertBlend(int blend) {
	if (AD2D_Window::_mode == API_OPENGL) {
		if (blend == BLEND_ZERO) return GL_ZERO;
		else if (blend == BLEND_ONE) return GL_ONE;
		else if (blend == BLEND_SRCCOLOR) return GL_SRC_COLOR;
		else if (blend == BLEND_INVSRCCOLOR) return GL_ONE_MINUS_SRC_COLOR;
		else if (blend == BLEND_SRCALPHA) return GL_SRC_ALPHA;
		else if (blend == BLEND_INVSRCALPHA) return GL_ONE_MINUS_SRC_ALPHA;
		else if (blend == BLEND_DSTCOLOR) return GL_DST_COLOR;
		else if (blend == BLEND_INVDSTCOLOR) return GL_ONE_MINUS_DST_COLOR;
		else if (blend == BLEND_DSTALPHA) return GL_DST_ALPHA;
		else if (blend == BLEND_INVDSTALPHA) return GL_ONE_MINUS_DST_ALPHA;
	}
	else if (AD2D_Window::_mode == API_DIRECT3D) {
		if (blend == BLEND_ZERO) return D3DBLEND_ZERO;
		else if (blend == BLEND_ONE) return D3DBLEND_ONE;
		else if (blend == BLEND_SRCCOLOR) return D3DBLEND_SRCCOLOR;
		else if (blend == BLEND_INVSRCCOLOR) return D3DBLEND_INVSRCCOLOR;
		else if (blend == BLEND_SRCALPHA) return D3DBLEND_SRCALPHA;
		else if (blend == BLEND_INVSRCALPHA) return D3DBLEND_INVSRCALPHA;
		else if (blend == BLEND_DSTCOLOR) return D3DBLEND_DESTCOLOR;
		else if (blend == BLEND_INVDSTCOLOR) return D3DBLEND_INVDESTCOLOR;
		else if (blend == BLEND_DSTALPHA) return D3DBLEND_DESTALPHA;
		else if (blend == BLEND_INVDSTALPHA) return D3DBLEND_INVDESTALPHA;
	}

	return 0;
}

/** \brief Set WINAPI 2D window blending mode
 *
 * \param BlendSrc int
 * \param BlendDst int
 * \return void
 *
 */
void AD2D_Window::SetBlendFunc(int BlendSrc, int BlendDst) {
	if (_mode == API_OPENGL)
		glBlendFunc(ConvertBlend(BlendSrc), ConvertBlend(BlendDst));
	else if (_mode == API_DIRECT3D) {
		AD2D_Window* gfx = GetPointer();
		LPDIRECT3DDEVICE9 D3DDev = (gfx ? gfx->GetD3DDEV() : NULL);
		if (D3DDev) {
			D3DDev->SetRenderState(D3DRS_SRCBLEND, ConvertBlend(BlendSrc));
			D3DDev->SetRenderState(D3DRS_DESTBLEND, ConvertBlend(BlendDst));
		}
	}
}

/** \brief Set active drawing color
 *
 * \param R float
 * \param G float
 * \param B float
 * \param alpha float
 * \return void
 *
 */
void AD2D_Window::SetColor(float R, float G, float B, float alpha) {
	if (_mode == API_OPENGL)
		glColor4f(R, G, B, alpha);

	_currentColor = COLOR(R, G, B, alpha);
}

/** \brief Get active drawing color
 *
 * \return COLOR
 *
 */
COLOR AD2D_Window::GetColor() {
	return _currentColor;
}

/** \brief Set active drawing color (Indexed color - R=(color/36)%6, G=(color/6)%6, B=color%6)
 *
 * \param color unsigned char
 * \param alpha float
 * \return void
 *
 */
void AD2D_Window::SetColorSTD(unsigned char color, float alpha) {
	float r = (float)((int)((color / 6) / 6) % 6) / 5.0f;
	float g = (float)((int)(color / 6) % 6) / 5.0f;
	float b = (float)((int)color % 6) / 5.0f;

	SetColor(r, g, b, alpha);
}

/** \brief Convert indexed color into RGB color
 *
 * \param color unsigned char
 * \return COLOR
 *
 */
COLOR AD2D_Window::ConvertColorSTD(unsigned char color) {
	COLOR ret;
	ret.red = (float)((int)((color / 6) / 6) % 6) / 5.0f;
	ret.green = (float)((int)(color / 6) % 6) / 5.0f;
	ret.blue = (float)((int)color % 6) / 5.0f;
	ret.alpha = 1.0f;

	return ret;
}

/** \brief Convert indexed color into RGB color
 *
 * \param color unsigned char
 * \return COLOR
 *
 */
COLOR AD2D_Window::STD2RGB(unsigned char color) {
	COLOR ret;
	ret.red = (float)((int)((color / 6) / 6) % 6) / 5.0f;
	ret.green = (float)((int)(color / 6) % 6) / 5.0f;
	ret.blue = (float)((int)color % 6) / 5.0f;
	ret.alpha = 1.0f;

	return ret;
}

// ---- AD2D_Viewport ---- //

AD2D_Viewport::AD2D_Viewport() {
	this->startX = 0;
	this->startY = 0;
	this->width = 0;
	this->height = 0;
}

void AD2D_Viewport::Create(int tX, int tY, int bX, int bY) {
	this->startX = tX;
	this->startY = tY;
	this->width = (bX - tX) + 1;
	this->height = (bY - tY) + 1;
}

bool AD2D_Viewport::operator==(AD2D_Viewport& vp) {
	if (vp.startX == startX && vp.startY == startY && vp.width == width && vp.height == height)
		return true;

	return false;
}

bool AD2D_Viewport::operator!=(AD2D_Viewport& vp) {
	if (vp == *this)
		return false;

	return true;
}

// ---- AD2D_Image ---- //

AD2D_Image::AD2D_Image() {
	realWidth = 0;
	realHeight = 0;
	virtualWidth = 0;
	virtualHeight = 0;
	texture = 0;
	textureD3D = NULL;
	surfaceD3D = NULL;
}

AD2D_Image::~AD2D_Image() {
	if (texture)
		glDeleteTextures(1,&texture);
	if (textureD3D)
		textureD3D->Release();
	if (surfaceD3D)
		surfaceD3D->Release();
}

void AD2D_Image::AdjustSize(unsigned short width, unsigned short height) {
	if (width == 0 || height == 0)
		return;

	virtualWidth = width;
	virtualHeight = height;

	unsigned short realSize;

	realSize = 1;
	for (realSize; realSize < width; realSize*=2);
	realWidth = realSize;

	realSize = 1;
	for (realSize; realSize < height; realSize*=2);
	realHeight = realSize;

	TexOffsetX = (AD2D_Window::_mode == API_DIRECT3D ? (0.5f / realWidth) : 0.0f);
	TexOffsetY = (AD2D_Window::_mode == API_DIRECT3D ? (0.5f / realHeight) : 0.0f);
}

void AD2D_Image::Create(unsigned short width, unsigned short height, GLuint texdata) {
	AdjustSize(width, height);

	texture = texdata;
}

void AD2D_Image::Create(unsigned short width, unsigned short height, LPDIRECT3DTEXTURE9 texdata) {
	AdjustSize(width, height);

	textureD3D = texdata;
}

void AD2D_Image::Create(unsigned short width, unsigned short height, unsigned char* data) {
	AdjustSize(width, height);

	if (realWidth == 0 || realHeight == 0)
		return;

	unsigned char* _data = new unsigned char[realWidth*realHeight*4];
	for (int x = 0; x < realWidth; x++)
	for (int y = 0; y < realHeight; y++) {
		if (x < virtualWidth && y < virtualHeight) {
			if (AD2D_Window::_mode == API_OPENGL) {
				_data[(x+y*realWidth)*4+0] = data[(x+y*virtualWidth)*4+0];
				_data[(x+y*realWidth)*4+1] = data[(x+y*virtualWidth)*4+1];
				_data[(x+y*realWidth)*4+2] = data[(x+y*virtualWidth)*4+2];
				_data[(x+y*realWidth)*4+3] = data[(x+y*virtualWidth)*4+3];
			}
			else if (AD2D_Window::_mode == API_DIRECT3D) {
				_data[(x+y*realWidth)*4+0] = data[(x+y*virtualWidth)*4+2];
				_data[(x+y*realWidth)*4+1] = data[(x+y*virtualWidth)*4+1];
				_data[(x+y*realWidth)*4+2] = data[(x+y*virtualWidth)*4+0];
				_data[(x+y*realWidth)*4+3] = data[(x+y*virtualWidth)*4+3];
			}
		}
		else {
			_data[(x+y*realWidth)*4+0] = 0;
			_data[(x+y*realWidth)*4+1] = 0;
			_data[(x+y*realWidth)*4+2] = 0;
			_data[(x+y*realWidth)*4+3] = 0;
		}
	}

	if (AD2D_Window::_mode == API_OPENGL) {
		if (!texture)
			glGenTextures(1, &texture);

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, realWidth, realHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data);
	}
	else if (AD2D_Window::_mode == API_DIRECT3D) {
		AD2D_Window* gfx = AD2D_Window::GetPointer();
		LPDIRECT3DDEVICE9 D3DDev = gfx->GetD3DDEV();
		if (!textureD3D)
			D3DXCreateTexture(D3DDev, realWidth, realHeight, 1, 0, D3DFMT_A8B8G8R8, D3DPOOL_MANAGED, &textureD3D);

		D3DLOCKED_RECT rect;
		textureD3D->LockRect(0, &rect, NULL, NULL);
		memcpy(rect.pBits, _data, realWidth * realHeight * 4);
		textureD3D->UnlockRect(0);
	}

	delete[] _data;
}

void AD2D_Image::CreateBMP(const char* path) {
	FILE* file;
	file = fopen(path, "r");
	if (!file) return;
	else fclose(file);

	AUX_RGBImageRec *TempTextureBMP = auxDIBImageLoad(path);

	unsigned char* _data = new unsigned char[TempTextureBMP->sizeX*TempTextureBMP->sizeY*4];
	for (int x = 0; x < TempTextureBMP->sizeX; x++)
	for (int y = 0; y < TempTextureBMP->sizeY; y++) {
		_data[(x+y*TempTextureBMP->sizeX)*4+0] = TempTextureBMP->data[(x+(TempTextureBMP->sizeY-y-1)*TempTextureBMP->sizeX)*3+0];
		_data[(x+y*TempTextureBMP->sizeX)*4+1] = TempTextureBMP->data[(x+(TempTextureBMP->sizeY-y-1)*TempTextureBMP->sizeX)*3+1];
		_data[(x+y*TempTextureBMP->sizeX)*4+2] = TempTextureBMP->data[(x+(TempTextureBMP->sizeY-y-1)*TempTextureBMP->sizeX)*3+2];
		_data[(x+y*TempTextureBMP->sizeX)*4+3] = 255;
	}

	Create(TempTextureBMP->sizeX, TempTextureBMP->sizeY, _data);

	delete[] _data;
}

void AD2D_Image::CreateBMP_(unsigned char* data, long int size) {
	if (!data)
		return;

	uint32_t bWidth = 0;
	uint32_t bHeight = 0;
	uint16_t bBits = 0;

	memcpy(&bWidth, data + 18, 4);
	memcpy(&bHeight, data + 22, 4);
	memcpy(&bBits, data + 28, 2);

	if (bBits != 24) {
		delete[] data;
		return;
	}

	unsigned char* wdata = new unsigned char[bWidth*bHeight*3];
	memcpy(wdata, data + 54, bWidth*bHeight*3);

	delete[] data;

	unsigned char* _data = new unsigned char[bWidth*bHeight*4];
	for (int x = 0; x < bWidth; x++)
	for (int y = bHeight - 1; y >= 0; y--) {
		_data[(x+y*bWidth)*4+0] = wdata[(x+y*bWidth)*3+2];
		_data[(x+y*bWidth)*4+1] = wdata[(x+y*bWidth)*3+1];
		_data[(x+y*bWidth)*4+2] = wdata[(x+y*bWidth)*3+0];
		_data[(x+y*bWidth)*4+3] = 255;
	}

	Create(bWidth, bHeight, _data);

	delete[] data;
	delete[] _data;
}

void AD2D_Image::CreatePNG(const char* path) {
	FILE* file;
	file = fopen(path, "rb");
	if (!file) return;

	gdImagePtr image;
	image = gdImageCreateFromPng(file);

	fclose(file);

	if (!image)
		return;

	unsigned char* _data = new unsigned char[gdImageSX(image)*gdImageSY(image)*4];
	for (int x = 0; x < gdImageSX(image); x++)
	for (int y = 0; y < gdImageSY(image); y++) {
		int c = gdImageGetPixel(image, x, y);
		_data[(x+y*gdImageSX(image))*4+0] = gdImageRed(image, c);
		_data[(x+y*gdImageSX(image))*4+1] = gdImageGreen(image, c);
		_data[(x+y*gdImageSX(image))*4+2] = gdImageBlue(image, c);
		_data[(x+y*gdImageSX(image))*4+3] = (int)(255.0f * (float)(127 - gdImageAlpha(image, c)) / 127);
	}

	Create(gdImageSX(image), gdImageSY(image), _data);

	gdImageDestroy(image);

	delete[] _data;
}

void AD2D_Image::CreatePNG_(unsigned char* data, long int size) {
	if (!data)
		return;

	gdImagePtr image;
	image = gdImageCreateFromPngPtr(size, data);

	delete[] data;

	if (!image)
		return;

	unsigned char* _data = new unsigned char[gdImageSX(image)*gdImageSY(image)*4];
	for (int x = 0; x < gdImageSX(image); x++)
	for (int y = 0; y < gdImageSY(image); y++) {
		int c = gdImageGetPixel(image, x, y);
		_data[(x+y*gdImageSX(image))*4+0] = gdImageRed(image, c);
		_data[(x+y*gdImageSX(image))*4+1] = gdImageGreen(image, c);
		_data[(x+y*gdImageSX(image))*4+2] = gdImageBlue(image, c);
		_data[(x+y*gdImageSX(image))*4+3] = (int)(255.0f * (float)(127 - gdImageAlpha(image, c)) / 127);
	}

	Create(gdImageSX(image), gdImageSY(image), _data);

	gdImageDestroy(image);

	delete[] _data;
}

void AD2D_Image::CreateGNP(const char* path) {
	FILE* file;
	file = fopen(path, "rb");
	if (!file) return;

	fseek(file, 0, SEEK_END);
	unsigned long size = ftell(file);
	unsigned char* buffer = new unsigned char[size];
	fseek(file, 0, SEEK_SET);

	fread(buffer, 1, size, file);

	for (unsigned long i = 0; i < size; i++)
		buffer[i] = buffer[i] ^ (i % 256);

	fclose(file);

	gdImagePtr image;
	image = gdImageCreateFromPngPtr(size, buffer);

	delete[] buffer;

	unsigned char* _data = new unsigned char[gdImageSX(image)*gdImageSY(image)*4];
	for (int x = 0; x < gdImageSX(image); x++)
	for (int y = 0; y < gdImageSY(image); y++) {
		int c = gdImageGetPixel(image, x, y);
		_data[(x+y*gdImageSX(image))*4+0] = gdImageRed(image, c);
		_data[(x+y*gdImageSX(image))*4+1] = gdImageGreen(image, c);
		_data[(x+y*gdImageSX(image))*4+2] = gdImageBlue(image, c);
		_data[(x+y*gdImageSX(image))*4+3] = (int)(255.0f * (float)(127 - gdImageAlpha(image, c)) / 127);
	}

	Create(gdImageSX(image), gdImageSY(image), _data);

	gdImageDestroy(image);

	delete[] _data;
}

void AD2D_Image::CreateGNP_(unsigned char* data, long int size) {
	if (!data)
		return;

	for (unsigned long i = 0; i < size; i++)
		data[i] = data[i] ^ (i % 256);

	gdImagePtr image;
	image = gdImageCreateFromPngPtr(size, data);

	delete[] data;

	unsigned char* _data = new unsigned char[gdImageSX(image)*gdImageSY(image)*4];
	for (int x = 0; x < gdImageSX(image); x++)
	for (int y = 0; y < gdImageSY(image); y++) {
		int c = gdImageGetPixel(image, x, y);
		_data[(x+y*gdImageSX(image))*4+0] = gdImageRed(image, c);
		_data[(x+y*gdImageSX(image))*4+1] = gdImageGreen(image, c);
		_data[(x+y*gdImageSX(image))*4+2] = gdImageBlue(image, c);
		_data[(x+y*gdImageSX(image))*4+3] = (int)(255.0f * (float)(127 - gdImageAlpha(image, c)) / 127);
	}

	Create(gdImageSX(image), gdImageSY(image), _data);

	gdImageDestroy(image);

	delete[] _data;
}

void AD2D_Image::CreateAlphaMask(unsigned char r, unsigned char g, unsigned char b, unsigned char tolerance) {
	if (!texture)
		return;

	unsigned char *data = new unsigned char[realWidth * realHeight * 4];

	if (AD2D_Window::_mode == API_OPENGL) {
		glBindTexture(GL_TEXTURE_2D,texture);
		glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
	}
	else if (AD2D_Window::_mode == API_DIRECT3D) {
		D3DLOCKED_RECT rect;
		textureD3D->LockRect(0, &rect, NULL, NULL);
		memcpy(data, rect.pBits, realWidth * realHeight * 4);
		textureD3D->UnlockRect(0);
	}

	for (int x = 0; x < realWidth; x++)
	for (int y = 0; y < realHeight; y++) {
		if (x < virtualWidth && y < virtualHeight) {
			unsigned char alpha = 255;
			if (abs((int)r - (int)data[(x+y*realWidth)*4+0]) <= tolerance &&
				abs((int)g - (int)data[(x+y*realWidth)*4+1]) <= tolerance &&
				abs((int)b - (int)data[(x+y*realWidth)*4+2]) <= tolerance)
			{
				int point = 0;
				point = std::max(point,abs((int)r - (int)data[(x+y*realWidth)*4+0]));
				point = std::max(point,abs((int)g - (int)data[(x+y*realWidth)*4+1]));
				point = std::max(point,abs((int)b - (int)data[(x+y*realWidth)*4+2]));
				alpha = (unsigned char)((float)point * (float)255 / (tolerance+1));
			}
			data[(x+y*realWidth)*4+3] = alpha;
		}
		else
			data[(x+y*realWidth)*4+3] = 0;
	}

	if (AD2D_Window::_mode == API_OPENGL)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, realWidth, realHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
	else if (AD2D_Window::_mode == API_DIRECT3D) {
		AD2D_Window* gfx = AD2D_Window::GetPointer();
		LPDIRECT3DDEVICE9 D3DDev = gfx->GetD3DDEV();
		if (textureD3D)
			textureD3D->Release();

		textureD3D = NULL;
		D3DXCreateTexture(D3DDev, realWidth, realHeight, 1, 0, D3DFMT_A8B8G8R8, D3DPOOL_MANAGED, &textureD3D);

		D3DLOCKED_RECT rect;
		textureD3D->LockRect(0, &rect, NULL, NULL);
		memcpy(rect.pBits, data, realWidth * realHeight * 4);
		textureD3D->UnlockRect(0);
	}

	delete[] data;
}

void AD2D_Image::Update(unsigned char* data) {
	unsigned char* _data = data;
	if (realWidth != virtualWidth || realHeight != virtualHeight) {
		_data = new unsigned char[realWidth*realHeight*4];
		for (int x = 0; x < realWidth; x++)
		for (int y = 0; y < realHeight; y++) {
			if (x < virtualWidth && y < virtualHeight) {
				_data[(x+y*realWidth)*4+0] = data[(x+y*virtualWidth)*4+0];
				_data[(x+y*realWidth)*4+1] = data[(x+y*virtualWidth)*4+1];
				_data[(x+y*realWidth)*4+2] = data[(x+y*virtualWidth)*4+2];
				_data[(x+y*realWidth)*4+3] = data[(x+y*virtualWidth)*4+3];
			}
			else {
				_data[(x+y*realWidth)*4+0] = 0;
				_data[(x+y*realWidth)*4+1] = 0;
				_data[(x+y*realWidth)*4+2] = 0;
				_data[(x+y*realWidth)*4+3] = 0;
			}
		}
	}

	if (AD2D_Window::_mode == API_OPENGL) {
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, realWidth, realHeight, GL_RGBA, GL_UNSIGNED_BYTE, _data);
	}
	else if (AD2D_Window::_mode == API_DIRECT3D) {
		AD2D_Window* gfx = AD2D_Window::GetPointer();
		LPDIRECT3DDEVICE9 D3DDev = gfx->GetD3DDEV();
		if (textureD3D)
			textureD3D->Release();

		textureD3D = NULL;
		D3DXCreateTexture(D3DDev, realWidth, realHeight, 1, 0, D3DFMT_A8B8G8R8, D3DPOOL_MANAGED, &textureD3D);

		D3DLOCKED_RECT rect;
		textureD3D->LockRect(0, &rect, NULL, NULL);
		memcpy(rect.pBits, _data, realWidth * realHeight * 4);
		textureD3D->UnlockRect(0);
	}

	if (_data != data)
		delete[] _data;
}

unsigned char* AD2D_Image::GetData(unsigned char* data) {
	if (AD2D_Window::_mode == API_OPENGL) {
		glBindTexture(GL_TEXTURE_2D, texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else if (AD2D_Window::_mode == API_DIRECT3D) {
		AD2D_Window* gfx = AD2D_Window::GetPointer();
		LPDIRECT3DDEVICE9 D3DDev = gfx->GetD3DDEV();

		D3DLOCKED_RECT rect;
		textureD3D->LockRect(0, &rect, NULL, NULL);
		memcpy(data, rect.pBits, realWidth * realHeight * 4);
		textureD3D->UnlockRect(0);

		for (int x = 0; x < realWidth; x++)
		for (int y = 0; y < realHeight; y++) {
			if (x < virtualWidth && y < virtualHeight) {
				unsigned char temp = data[(x+y*virtualWidth)*4+0];
				data[(x+y*virtualWidth)*4+0] = data[(x+y*virtualWidth)*4+2];
				data[(x+y*virtualWidth)*4+2] = temp;
			}
		}
	}
}

unsigned short AD2D_Image::GetWidth() {
	return virtualWidth;
}

unsigned short AD2D_Image::GetHeight() {
	return virtualHeight;
}

void AD2D_Font::ReadWidthMap(const char* path) {
	FILE* file;
	file = fopen(path, "rb");
	if (!file) return;

	unsigned char key = 0;
	while(!feof(file)) {
		int width = fgetc(file) + fgetc(file) * 256;

		widthMap[key++] = width;
	}

	if (virtualWidth > 0)
		realSize = virtualWidth / 16;
}

void AD2D_Font::ReadWidthMap_(unsigned char* data, long int size) {
	if (!data)
		return;

	long int pos = 0;
	unsigned char key = 0;
	while(pos < size) {
		int width = data[pos] + data[pos+1] * 256;
		pos += 2;

		widthMap[key++] = width;
	}

	if (virtualWidth > 0)
		realSize = virtualWidth / 16;

	delete[] data;
}

int AD2D_Font::GetCharWidth(unsigned char key) {
	std::map<unsigned char, int>::iterator it = widthMap.find(key);
	if (it != widthMap.end())
		return it->second;

	return 0;
}

float AD2D_Font::GetCharWidth(unsigned char key, float fontSize) {
	std::map<unsigned char, int>::iterator it = widthMap.find(key);
	if (it != widthMap.end())
		return ((float)it->second * fontSize / realSize);

	return 0.0f;
}

float AD2D_Font::GetTextWidth(std::string text, float fontSize) {
	float width = 0.0f;
	for (int i = 0; i < text.length(); i++) {
		unsigned char key = text[i];
		width += ((float)GetCharWidth(key) * fontSize / realSize);
	}

	return width;
}

int AD2D_Font::GetTextWidthCharNumber(std::string text, float fontSize, float width) {
	int number = 0;

	float twidth = 0.0f;
	for (int i = 0; i < text.length(); i++) {
		unsigned char key = text[i];
		twidth += ((float)GetCharWidth(key) * fontSize / realSize);
		if (twidth < width)
			number++;
		else
			break;
	}

	return number;
}
