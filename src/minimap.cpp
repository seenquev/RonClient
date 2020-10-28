/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "minimap.h"

#include "allocator.h"
#include "game.h"
#include "luascript.h"


// ---- MiniMap ---- //

bool MiniMap::updateImage = false;
MUTEX MiniMap::lockMiniMap;


MiniMap::MiniMap() {
	lastUpdate = 0;

	wfileN = 0;
	rfileN = 0;
	wfile = NULL;
	rfile = NULL;
	memset(wdata, 0, 4096);
	memset(wdata + 4096, 0xFF, 8192);
	memset(rdata, 0, 4096);
	memset(rdata + 4096, 0xFF, 8192);

	cx = 0.0f;
	cy = 0.0f;
	cz = 0;

	offsetX = 0.0f;
	offsetY = 0.0f;
	zoom = 2.0f;

	minimap = new(M_PLACE) AD2D_Image;

	unsigned char* _data = new(M_PLACE) unsigned char[256 * 256 * 4];
	memset(_data, 0, 256 * 256 * 4);

	if (AD2D_Window::_mode == API_OPENGL) {
		minimap->AdjustSize(256, 256);
		glGenTextures(1,&minimap->texture);
		glBindTexture(GL_TEXTURE_2D, minimap->texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, _data);
	}
	else if (AD2D_Window::_mode == API_DIRECT3D) {
		minimap->Create(256, 256, _data);
	}

	delete_debug_array(_data, M_PLACE);

	data = new(M_PLACE) unsigned char[256 * 256];
	memset(data, 0, 256 * 256);
	sdata = new(M_PLACE) unsigned short[256 * 256];
	memset(sdata, 0xFF, 256 * 256 * 2);
}

MiniMap::~MiniMap() {
	LOCKCLASS lockClass(lockMiniMap);

	if (wfile) {
		FileFlush();
		fclose(wfile);
	}

	if (rfile)
		fclose(rfile);

	if (minimap)
		delete_debug(minimap, M_PLACE);

	if (data)
		delete_debug_array(data, M_PLACE);

	if (sdata)
		delete_debug_array(sdata, M_PLACE);
}


void MiniMap::AddMarker(Position pos, Marker marker) {
	LOCKCLASS lockClass(lockMiniMap);

	markers[pos] = marker;
}

Marker MiniMap::GetMarker(Position pos) {
	LOCKCLASS lockClass(lockMiniMap);

	MarkersMap::iterator it = markers.find(pos);
	if (it != markers.end())
		return it->second;

	return Marker(0, std::string(""));
}

void MiniMap::RemoveMarker(Position pos) {
	LOCKCLASS lockClass(lockMiniMap);

	MarkersMap::iterator it = markers.find(pos);
	if (it != markers.end())
		markers.erase(it);
}

MarkersMap MiniMap::GetMarkers() {
	LOCKCLASS lockClass(lockMiniMap);

	return markers;
}

void MiniMap::ClearMarkers() {
	LOCKCLASS lockClass(lockMiniMap);

	markers.clear();
}


void MiniMap::AddWaypoint(Position pos, Waypoint waypoint, unsigned short num) {
	LOCKCLASS lockClass(lockMiniMap);

	if (num == 0xFFFF)
		waypoints.push_back(WaypointElement(pos, waypoint));
	else {
		WaypointsList::iterator it = waypoints.begin();
		for (it; it != waypoints.end() && num > 0; it++, num--) {
			Position tempPos = it->first;
			if (pos == tempPos) {
				it->second = waypoint;
				return;
			}
		}

		waypoints.insert(it, WaypointElement(pos, waypoint));
	}
}

Waypoint MiniMap::GetWaypoint(Position pos) {
	LOCKCLASS lockClass(lockMiniMap);

	WaypointsList::iterator it = waypoints.begin();
	for (it; it != waypoints.end(); it++) {
		Position tempPos = it->first;
		if (pos == tempPos)
			return it->second;
	}

	return Waypoint(0, std::string(""));
}

unsigned short MiniMap::GetWaypointNumber(Position pos) {
	LOCKCLASS lockClass(lockMiniMap);

	unsigned short num = 0;

	WaypointsList::iterator it = waypoints.begin();
	for (it; it != waypoints.end(); it++, num++) {
		Position tempPos = it->first;
		if (pos == tempPos)
			return num;
	}

	return 0xFFFF;
}

void MiniMap::RemoveWaypoint(Position pos) {
	LOCKCLASS lockClass(lockMiniMap);

	WaypointsList::iterator it = waypoints.begin();
	for (it; it != waypoints.end(); it++) {
		Position tempPos = it->first;
		if (pos == tempPos)
			break;
	}

	if (it != waypoints.end())
		waypoints.erase(it);
}

WaypointsList MiniMap::GetWaypoints() {
	LOCKCLASS lockClass(lockMiniMap);

	return waypoints;
}

void MiniMap::ClearWaypoints() {
	LOCKCLASS lockClass(lockMiniMap);

	waypoints.clear();
}


void MiniMap::FileGetMap(Position corner, int x, int y, int w, int h) {
	LOCKCLASS lockClass(lockMiniMap);

	if (w == 256 && h == 256)
		ClearMiniMap();

	if (rfile) {
		fclose(rfile);
		rfileN = 0;
	}

	int _x = corner.x - 119;
	int _y = corner.y - 121;

	int mz = corner.z + cz;
	for (int a = x; a < x + w; a++)
	for (int b = y; b < y + h; b++) {
		int mx = (_x + a) / 64;
		int my = (_y + b) / 64;

		if (_x + a < 0 || _y + b < 0)
			continue;

		unsigned int fileN = mx * 10000 + my * 100 + mz;
		if (rfileN != fileN) {
			if (rfile)
				fclose(rfile);
			memset(rdata, 0, 4096);
			memset(rdata + 4096, 0xFF, 8192);

			char filename[256];
			sprintf(filename, "%s/minimap/%02d%02d%02d.map", Game::filesLocation.c_str(), mx, my, mz);
			rfile = fopen(filename, "rb");
			if (!rfile)
				rfile = fopen(filename, "rb");

			if (rfile)
				fread(rdata, 12288, 1, rfile);

			rfileN = fileN;
		}

		int px = (_x + a) % 64;
		int py = (_y + b) % 64;

		data[a + b * 256] = rdata[px + py * 64];
		sdata[a + b * 256] = *(unsigned short*)(rdata + 4096 + (px + py * 64) * 2);
	}

	updateImage = true;
}

void MiniMap::FileSetPix(Position pix, unsigned char color, unsigned short speed) {
	LOCKCLASS lockClass(lockMiniMap);

	int mx = pix.x / 64;
	int my = pix.y / 64;
	int mz = pix.z;

	if (pix.x < 0 || pix.y < 0)
		return;

	unsigned int fileN = mx * 10000 + my * 100 + mz;
	if (wfileN != fileN) {
		if (wfile) {
			FileFlush();
			fclose(wfile);
		}
		memset(wdata, 0, 4096);
		memset(wdata + 4096, 0xFF, 8192);

		char filename[256];
		sprintf(filename, "%s/minimap/%02d%02d%02d.map", Game::filesLocation.c_str(), mx, my, mz);
		wfile = fopen(filename, "rb");
		if (!wfile)
			wfile = fopen(filename, "rb");

		if (wfile) {
			fread(wdata, 12288, 1, wfile);
			fclose(wfile);
		}

		wfile = fopen(filename, "wb");
		wfileN = fileN;
	}

	int px = pix.x % 64;
	int py = pix.y % 64;

	wdata[px + py * 64] = color;
	*(unsigned short*)(wdata + 4096 + (px + py * 64) * 2) = speed;
}

void MiniMap::FileFlush() {
	LOCKCLASS lockClass(lockMiniMap);

	if (wfile) {
		fseek(wfile, 0, SEEK_SET);
		fwrite(wdata, 12288, 1, wfile);
		fflush(wfile);
	}
}

void MiniMap::CloseFile() {
	FileFlush();

	if (rfile)
		fclose(rfile);
	if (wfile)
		fclose(wfile);
}

void MiniMap::UpdateImage() {
	LOCKCLASS lockClass(lockMiniMap);

	unsigned char* _data = new(M_PLACE) unsigned char[256 * 256 * 4];
	memset(_data, 0, 256 * 256 * 4);
	for (int x = 0; x < 256; x++)
	for (int y = 0; y < 256; y++) {
		COLOR color = AD2D_Window::STD2RGB(data[(x + y * 256)]);
		//COLOR color(0, (float)sdata[x + y * 256] / 255, 0);
		if (AD2D_Window::_mode == API_OPENGL) {
			_data[(x + y * 256) * 4 + 0] = (int)(color.red * 255);
			_data[(x + y * 256) * 4 + 1] = (int)(color.green * 255);
			_data[(x + y * 256) * 4 + 2] = (int)(color.blue * 255);
		}
		else if (AD2D_Window::_mode == API_DIRECT3D) {
			_data[(x + y * 256) * 4 + 2] = (int)(color.red * 255);
			_data[(x + y * 256) * 4 + 1] = (int)(color.green * 255);
			_data[(x + y * 256) * 4 + 0] = (int)(color.blue * 255);
		}
		_data[(x + y * 256) * 4 + 3] = 255;
	}

	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	if (player && player->GetTargetPos() != Position(0, 0, 0)) {
		Position playerPos = player->GetPosition();
		Position pos = playerPos;

		std::list<Direction> autoPath = player->GetAutoPath();
		std::list<Direction>::iterator it = autoPath.begin();
		for (it; it != autoPath.end(); it++) {
			Direction dir = *it;
			if (dir == NORTH || dir == NORTHEAST || dir == NORTHWEST) pos.y++;
			else if (dir == SOUTH || dir == SOUTHEAST || dir == SOUTHWEST) pos.y--;
			if (dir == EAST || dir == NORTHEAST || dir == SOUTHEAST) pos.x--;
			else if (dir == WEST || dir == NORTHEAST || dir == SOUTHWEST) pos.x++;

			int px = 127 - (pos.x - playerPos.x);
			int py = 127 - (pos.y - playerPos.y);
			if (px >= 0 && px <= 255 && py >= 0 && py <= 255) {
				_data[(px + py * 256) * 4 + 0] = 255;
				_data[(px + py * 256) * 4 + 1] = 255;
				_data[(px + py * 256) * 4 + 2] = 255;
				_data[(px + py * 256) * 4 + 3] = 255;
			}
		}
	}

	minimap->Update(_data);

	delete_debug_array(_data, M_PLACE);
}


void MiniMap::MoveLevel(char step) {
	cz += step;
}

void MiniMap::ResetLevel() {
	cz = 0;
}

char MiniMap::GetOffsetLevel() {
	return cz;
}


void MiniMap::SetZoom(float zoom) {
	this->zoom = zoom;
}

float MiniMap::GetZoom() {
	return zoom;
}

void MiniMap::ChangeZoom(float multiplier) {
	if (zoom * multiplier < 1.0f || zoom * multiplier > 10.0f)
		multiplier = 1.0f;

    zoom *= multiplier;
	offsetX *= multiplier;
	offsetY *= multiplier;
}

void MiniMap::ResetZoom() {
	zoom = 2.0f;
	offsetX = 0;
	offsetY = 0;
}

void MiniMap::GetOffsetPos(float& x, float& y) {
	x = offsetX;
	y = offsetY;
}

void MiniMap::SetOffsetPos(float x, float y) {
	offsetX = x;
	offsetY = y;
}


void MiniMap::ClearMiniMap() {
	LOCKCLASS lockClass(lockMiniMap);

	memset(data, 0, 256 * 256);
	memset(sdata, 0xFF, 256 * 256 * 2);
}

void MiniMap::MoveMiniMap(int dx, int dy) {
	LOCKCLASS lockClass(lockMiniMap);

	unsigned char* _data = new(M_PLACE) unsigned char[256 * 256];
	unsigned short* _sdata = new(M_PLACE) unsigned short[256 * 256];
	memcpy(_data, data, 256 * 256);
	memcpy(_sdata, sdata, 256 * 256 * 2);

	for (int x = 0; x < 256; x++)
	for (int y = 0; y < 256; y++) {
		int px = x - dx;
		int py = y - dy;

		if (px >= 0 && px < 256 && py >= 0 && py < 256) {
			data[x + y * 256] = _data[px + py * 256];
			sdata[x + y * 256] = _sdata[px + py * 256];
		}
		else {
			data[x + y * 256] = 0;
			sdata[x + y * 256] = 0;
		}
	}

	delete_debug_array(_data, M_PLACE);
	delete_debug_array(_sdata, M_PLACE);

	updateImage = true;
}

void MiniMap::SetPix(Position corner, Position pix, unsigned char color, unsigned short speed) {
	LOCKCLASS lockClass(lockMiniMap);

	int x = 119 + pix.x - corner.x;
	int y = 121 + pix.y - corner.y;

	if (cz == 0) {
		data[x + y * 256] = color;
		sdata[x + y * 256] = speed;
	}
	FileSetPix(pix, color, speed);

	updateImage = true;
}

void MiniMap::SetPix(int x, int y, unsigned char color) {
	LOCKCLASS lockClass(lockMiniMap);

	if (cz == 0)
		data[x + y * 256] = color;

	updateImage = true;
}

unsigned short MiniMap::GetSpeed(int x, int y) {
	LOCKCLASS lockClass(lockMiniMap);

	if (x < 0 || x > 255 || y < 0 || y > 255)
		return 0;

	return sdata[x + y * 256];
}

unsigned short MiniMap::GetSpeed(Position corner, Position pix) {
	LOCKCLASS lockClass(lockMiniMap);

	int x = 119 + pix.x - corner.x;
	int y = 121 + pix.y - corner.y;

	if (x < 0 || x > 255 || y < 0 || y > 255)
		return 0;

	return sdata[x + y * 256];
}

void MiniMap::PrintMiniMap(AD2D_Window* gfx, float width, float height) {
	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(lockMiniMap);

	if (updateImage) {
		updateImage = false;
		UpdateImage();
	}

	float x1 = offsetX;
	float y1 = offsetY;
	float x2 = x1 + width;
	float y2 = y1 + height;

	float dw = (x2 - x1) * zoom - (x2 - x1);
	float dh = (y2 - y1) * zoom - (y2 - y1);
	float zw = ((x2 - x1) * zoom / 256);
	float zh = ((y2 - y1) * zoom / 256);

	gfx->SetBlendFunc(BLEND_ONE, BLEND_ZERO);
	if (AD2D_Window::_mode == API_DIRECT3D) {
		LPDIRECT3DDEVICE9 D3DDev = gfx->GetD3DDEV();
		D3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		D3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	}
	gfx->PutImage(x1 - dw / 2, y1 - dh / 2, x2 + dw / 2, y2 + dh / 2, *minimap);
	if (AD2D_Window::_mode == API_DIRECT3D) {
		LPDIRECT3DDEVICE9 D3DDev = gfx->GetD3DDEV();
		D3DDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		D3DDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	}
	gfx->SetBlendFunc(BLEND_SRCALPHA, BLEND_INVSRCALPHA);

	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);

	MarkersMap::iterator it = markers.begin();
	for (it; it != markers.end(); it++) {
		Position pos = it->first;
		Marker marker = it->second;

		Position corner = map->GetCorner();
		if (map && pos.z == corner.z + cz) {
			float px = x1 + (x2 - x1) / 2 + (float)(pos.x - (corner.x + 8)) * zw;
			float py = y1 + (y2 - y1) / 2 + (float)(pos.y - (corner.y + 6)) * zh;

			AD2D_Image* image = Icons::GetMinimapIcon(marker.first);
			if (image)
				gfx->PutImage(px - image->GetWidth() / 2, py - image->GetHeight() / 2, *image);
		}
	}

	Position lastPos = Position(0, 0, 0);
	WaypointsList::iterator wit = waypoints.begin();
	for (wit; wit != waypoints.end(); wit++) {
		Position pos = wit->first;
		Waypoint waypoint = wit->second;

		Position corner = map->GetCorner();
		if (map && pos.z == corner.z + cz) {
			float px = x1 + (x2 - x1) / 2 + (float)(pos.x - (corner.x + 8)) * zw;
			float py = y1 + (y2 - y1) / 2 + (float)(pos.y - (corner.y + 6)) * zh;

			if (lastPos != Position(0, 0, 0) && pos != lastPos) {
				float lpx = x1 + (x2 - x1) / 2 + (float)(lastPos.x - (corner.x + 8)) * zw;
				float lpy = y1 + (y2 - y1) / 2 + (float)(lastPos.y - (corner.y + 6)) * zh;

				gfx->PutLine(lpx, lpy, px, py);
			}

			AD2D_Image* image = Icons::GetWaypointIcon(waypoint.first);
			if (image)
				gfx->PutImage(px - image->GetWidth() / 2, py - image->GetHeight() / 2, *image);
		}

		lastPos = pos;
	}

	if (waypoints.size() > 2) {
		Position pos = waypoints.begin()->first;

		Position corner = map->GetCorner();
		if (map && pos.z == corner.z + cz) {
			float px = x1 + (x2 - x1) / 2 + (float)(pos.x - (corner.x + 8)) * zw;
			float py = y1 + (y2 - y1) / 2 + (float)(pos.y - (corner.y + 6)) * zh;

			if (lastPos != Position(0, 0, 0)) {
				float lpx = x1 + (x2 - x1) / 2 + (float)(lastPos.x - (corner.x + 8)) * zw;
				float lpy = y1 + (y2 - y1) / 2 + (float)(lastPos.y - (corner.y + 6)) * zh;

				gfx->PutLine(lpx, lpy, px, py);
			}
		}
	}
}


//Lua functions

int MiniMap::LuaGetMiniMap(lua_State* L) {
	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);
	MiniMap* minimap = (map ? map->GetMiniMap() : NULL);

	LuaScript::PushNumber(L, (unsigned long)minimap);
	return 1;
}

int MiniMap::LuaMiniMapChangeLevel(lua_State* L) {
	int step = LuaScript::PopNumber(L);
	MiniMap* minimap = (MiniMap*)((unsigned long)LuaScript::PopNumber(L));

	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);
	if (map)
		map->MoveMiniMapLevel(step);

	return 1;
}

int MiniMap::LuaMiniMapResetLevel(lua_State* L) {
	MiniMap* minimap = (MiniMap*)((unsigned long)LuaScript::PopNumber(L));

	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);
	if (map)
		map->ResetMiniMapLevel();

	return 1;
}

int MiniMap::LuaMiniMapSetZoom(lua_State* L) {
	float zoom = LuaScript::PopNumber(L);
	MiniMap* minimap = (MiniMap*)((unsigned long)LuaScript::PopNumber(L));

	if (minimap)
		minimap->SetZoom(zoom);

	return 1;
}

int MiniMap::LuaMiniMapGetZoom(lua_State* L) {
	MiniMap* minimap = (MiniMap*)((unsigned long)LuaScript::PopNumber(L));

	float zoom = 0.0f;
	if (minimap)
		zoom = minimap->GetZoom();

	LuaScript::PushNumber(L, zoom);
	return 1;
}

int MiniMap::LuaMiniMapChangeZoom(lua_State* L) {
	float multiplier = LuaScript::PopNumber(L);
	MiniMap* minimap = (MiniMap*)((unsigned long)LuaScript::PopNumber(L));

	if (minimap)
		minimap->ChangeZoom(multiplier);

	return 1;
}

int MiniMap::LuaMiniMapResetZoom(lua_State* L) {
	MiniMap* minimap = (MiniMap*)((unsigned long)LuaScript::PopNumber(L));

	if (minimap)
		minimap->ResetZoom();

	return 1;
}


void MiniMap::LuaRegisterFunctions(lua_State* L) {
	//getMiniMap() : minimapPtr
	lua_register(L, "getMiniMap", MiniMap::LuaGetMiniMap);

	//changeMiniMapLevel(minimapPtr, step)
	lua_register(L, "changeMiniMapLevel", MiniMap::LuaMiniMapChangeLevel);

	//resetMiniMapLevel(minimapPtr)
	lua_register(L, "resetMiniMapLevel", MiniMap::LuaMiniMapResetLevel);

	//setMiniMapZoom(minimapPtr, zoom)
	lua_register(L, "setMiniMapZoom", MiniMap::LuaMiniMapChangeZoom);

	//getMiniMapZoom(minimapPtr) : zoom
	lua_register(L, "getMiniMapZoom", MiniMap::LuaMiniMapChangeZoom);

	//changeMiniMapZoom(minimapPtr, multiplier)
	lua_register(L, "changeMiniMapZoom", MiniMap::LuaMiniMapChangeZoom);

	//resetMiniMapZoom(minimapPtr)
	lua_register(L, "resetMiniMapZoom", MiniMap::LuaMiniMapResetZoom);
}
