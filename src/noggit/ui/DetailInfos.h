// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/ui/Window.h>

class detailInfos;
class UIMapViewGUI;
class UIText;

class UIDetailInfos : public UIWindow
{
private:
  UIText* theInfos;

public:
  UIDetailInfos(float x, float y, float width, float height);
  void setText(const std::string& t);
};
