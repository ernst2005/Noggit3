// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/ScrollBar.h>

#include <iostream>
#include <sstream>
#include <vector>

#include <noggit/Log.h>
#include <noggit/Misc.h>
#include <noggit/application.h> // app.getArial14(), arialn13
#include <noggit/ui/Button.h>
#include <noggit/ui/Text.h>
#include <noggit/ui/Texture.h>

const float UIScrollBar::WIDTH = 16.0f;

UIScrollBar::UIScrollBar(float xpos, float ypos, float h, int n, Orientation orientation)
  : UIFrame(xpos, ypos, 0.0f, 0.0f)
  , mTarget()
  , num(n)
  , value(0)
  , changeFunc(nullptr)
  , ScrollKnob(nullptr)
  , _orientation(orientation)
  , extValue()
{
  UIButton::Ptr ScrollUp(nullptr);
  UIButton::Ptr ScrollDown(nullptr);

  if (_orientation == Vertical)
  {
    width(UIScrollBar::WIDTH);
    height(h);
    ScrollUp = new UIButton(-6.0f, -8.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Up.blp", "Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Down.blp");
    ScrollDown = new UIButton(-6.0f, height() - 24.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Up.blp", "Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Down.blp");
    ScrollKnob = new UITexture(-6.0f, 10.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-Knob.blp");
  }
  else
  {
    width(h);
    height(UIScrollBar::WIDTH);
    //! \todo filenames for buttons!
    ScrollUp = new UIButton(-8.0f, -6.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Up.blp", "Interface\\Buttons\\UI-ScrollBar-ScrollUpButton-Down.blp");
    ScrollDown = new UIButton(width() - 24.0f, -6.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Up.blp", "Interface\\Buttons\\UI-ScrollBar-ScrollDownButton-Down.blp");
    ScrollKnob = new UITexture(10.0f, -6.0f, 32.0f, 32.0f, "Interface\\Buttons\\UI-ScrollBar-Knob.blp");
  }

  addChild(ScrollUp);
  addChild(ScrollDown);
  addChild(ScrollKnob);

  ScrollUp->setClickFunc ([this] { clickReturn (-1); });
  ScrollDown->setClickFunc ([this] { clickReturn (1); });
}

bool UIScrollBar::processLeftDrag(float mx, float my, float /*xChange*/, float /*yChange*/)
{
  if (num < 0)
    return false;

  float tx(0.0f);
  float ty(0.0f);
  getOffset(&tx, &ty);
  mx -= tx + 32.0f;
  my -= ty + 32.0f;

  if (_orientation == Vertical)
    value = std::min(num - 1, std::max(0, misc::FtoIround(num * my / (height() - 64.0f))));
  else
    value = std::min(num - 1, std::max(0, misc::FtoIround(num * mx / (width() - 64.0f))));

  setScrollNoob();

  if (changeFunc)
  {
    changeFunc(this, value);
  }

  return true;
}

UIFrame::Ptr UIScrollBar::processLeftClick(float mx, float my)
{
  UIFrame::Ptr temp = UIFrame::processLeftClick(mx, my);
  if (temp == ScrollKnob)
    //! \note We want the drag event for the knob.
  {
    return this;
  }
  return temp;
}

void UIScrollBar::clickReturn(int id)
{
  if (id != 0)
  {
    value = std::min(num - 1, std::max(0, value + id));

    setScrollNoob();

    if (changeFunc)
    {
      changeFunc(this, value);
    }
  }
}

void UIScrollBar::setChangeFunc(void(*f)(UIFrame *, int))
{
  changeFunc = f;
}

int UIScrollBar::getValue() const
{
  return value;
}

void UIScrollBar::setValue(int i)
{
  if (i > -1 && i < num)
    value = i;
  setScrollNoob();
}

void UIScrollBar::setNum(int i)
{
  num = i;
  value = 0;
  setScrollNoob();
}

void UIScrollBar::setScrollNoob()
{
  if (num)
  {
    if (_orientation == Vertical)
      ScrollKnob->y(11.0f + ((height() - 54.0f) / (num - 1)) * value);
    else
      ScrollKnob->x(11.0f + ((width() - 54.0f) / (num - 1)) * value);
  }
}
