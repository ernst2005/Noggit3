// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>
#include <vector>

#include "AppState.h"
#include "ModelManager.h"
#include "math/vector_3d.hpp"

#include <boost/optional.hpp>

// ui classes
class UIFrame;
class UIStatusBar;
class UIAbout;
class UIMinimapWindow;
class UIMenuBar;

class World;
class Model;
class MapView;

struct MapEntry
{
	int mapID;
	std::string name;
	int areaType;
};

struct BookmarkEntry
{
	int mapID;
	std::string name;
	math::vector_3d pos;
	float ah;
	float av;
};

class Menu : public AppState
{
public:
	Menu();
	~Menu();

	void tick(float t, float dt);
	void display(float t, float dt);

	void keypressed(SDL_KeyboardEvent* e);
	void mouseclick(SDL_MouseButtonEvent* e);
	void mousemove(SDL_MouseMotionEvent* e);

	//! \todo Make private when new buttons are implemented.
	void loadMap(int mapID);
	void loadBookmark(int bookmarkID);

	//! \brief Enter the the map on the given location.
	void enterMapAt(math::vector_3d pos, bool pAutoHeight = true, float av = -30.0f, float ah = -90.0f);

private:
	UIFrame* mGUIFrame;
	UIStatusBar* mGUIStatusbar;
	UIAbout* mGUICreditsWindow;
	UIMinimapWindow* mGUIMinimapWindow;
	UIMenuBar* mGUImenuBar;

	std::vector<MapEntry> mMaps;
	std::vector<BookmarkEntry> mBookmarks;

	boost::optional<scoped_model_reference> mBackgroundModel;
	int mLastBackgroundId;

	void createBookmarkList();
	void createMapList();
	void buildMenuBar();
	void randBackground();

	void resizewindow();
};
