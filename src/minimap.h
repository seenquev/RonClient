/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __MINIMAP_H_
#define __MINIMAP_H_

#include <map>
#include <string>

#include "ad2d.h"
#include "luascript.h"
#include "mthread.h"
#include "position.h"
#include "realtime.h"
#include "tools.h"

typedef std::pair<unsigned char, std::string> Marker;
typedef std::map<Position, Marker> MarkersMap;

typedef std::pair<unsigned char, std::string> Waypoint;
typedef std::pair<Position, Waypoint> WaypointElement;
typedef std::list<WaypointElement> WaypointsList;

class MiniMap {
private:
	AD2D_Image*		minimap;
	unsigned char*	data;
	unsigned short* sdata;

	time_lt			lastUpdate;

	FILE*			wfile;
	FILE*			rfile;
	unsigned int	wfileN;
	unsigned int	rfileN;
	unsigned char	wdata[12288];
	unsigned char	rdata[12288];

	float cx;
	float cy;
	char cz;

	float		offsetX;
	float		offsetY;
	float		zoom;

	MarkersMap		markers;
	WaypointsList	waypoints;

public:
	static bool		updateImage;
	static MUTEX	lockMiniMap;

public:
	MiniMap();
	~MiniMap();

	void AddMarker(Position pos, Marker marker);
	Marker GetMarker(Position pos);
	void RemoveMarker(Position pos);
	MarkersMap GetMarkers();
	void ClearMarkers();

	void AddWaypoint(Position pos, Waypoint waypoint, unsigned short num = 0xFFFF);
	Waypoint GetWaypoint(Position pos);
	unsigned short GetWaypointNumber(Position pos);
	void RemoveWaypoint(Position pos);
	WaypointsList GetWaypoints();
	void ClearWaypoints();

	void FileGetMap(Position corner, int x, int y, int w, int h);
	void FileSetPix(Position pix, unsigned char color, unsigned short speed);
	void FileFlush();
	void CloseFile();

	void UpdateImage();
	void ClearImage();

	void MoveLevel(char step);
	void ResetLevel();
	char GetOffsetLevel();

	void SetZoom(float zoom);
	float GetZoom();
	void ChangeZoom(float multiplier);
	void ResetZoom();
	void GetOffsetPos(float& x, float& y);
	void SetOffsetPos(float x, float y);

	void ClearMiniMap();
	void MoveMiniMap(int dx, int dy);
	void SetPix(Position corner, Position pix, unsigned char color, unsigned short speed);
	void SetPix(int x, int y, unsigned char color);
	unsigned short GetSpeed(int x, int y);
	unsigned short GetSpeed(Position corner, Position pix);

	void PrintMiniMap(AD2D_Window* gfx, float width, float height);

	//Lua functions
	static int LuaGetMiniMap(lua_State* L);
	static int LuaMiniMapChangeLevel(lua_State* L);
	static int LuaMiniMapResetLevel(lua_State* L);
	static int LuaMiniMapSetZoom(lua_State* L);
	static int LuaMiniMapGetZoom(lua_State* L);
	static int LuaMiniMapChangeZoom(lua_State* L);
	static int LuaMiniMapResetZoom(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__MINIMAP_H_
