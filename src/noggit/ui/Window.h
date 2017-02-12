// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <string>

#include <noggit/FreeType.h> // fonts.
#include <noggit/ui/Frame.h>
#include <noggit/TextureManager.h>

class UIWindow : public UIFrame
{
public:
  typedef UIWindow* Ptr;

  float getX(){ return this->_x; }
  float getY(){ return this->_y; }
  float getW(){ return this->_width; }
  float getH(){ return this->_height; }

protected:
  scoped_blp_texture_reference texture;

public:
  UIWindow(float xPos, float yPos, float w, float h);
  UIWindow(float xPos, float yPos, float w, float h, const std::string& pTexture);
  UIFrame::Ptr processLeftClick(float mx, float my);
  void render() const;
};
