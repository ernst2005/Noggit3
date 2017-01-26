// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/MenuBar.h>

#include <map>
#include <string>
#include <vector>

#include <noggit/ui/CheckBox.h>
#include <noggit/ui/Text.h>
#include <noggit/Video.h>
#include <opengl/scoped.hpp>

UIMenuBar::UIMenuBar()
  : UIWindow(0.0f, 0.0f, static_cast<float>(video.xres()), static_cast<float>(video.yres()))
  , mNumMenus(0)
{
}

void UIMenuBar::render() const
{
  gl.color4f(0.2f, 0.2f, 0.2f, 0.5f);
  gl.begin(GL_TRIANGLE_STRIP);
  gl.vertex2f(0.0f, 0.0f);
  gl.vertex2f(static_cast<float>(video.xres()), 0.0f);
  gl.vertex2f(0.0f, 30.0f);
  gl.vertex2f(static_cast<float>(video.xres()), 30.0f);
  gl.end();

  UIFrame::render();

  gl.color3f(1.0f, 1.0f, 1.0f);

  opengl::texture::set_active_texture();
  opengl::texture::enable_texture();

  texture->bind();

  //Draw Top Side
  gl.begin(GL_TRIANGLE_STRIP);
  gl.texCoord2f(0.5f, 1.0f);
  gl.vertex2f(0.0f, 33.0f);
  gl.texCoord2f(0.5f, 0.0f);
  gl.vertex2f(static_cast<float>(video.xres()), 33.0f);
  gl.texCoord2f(0.375f, 1.0f);
  gl.vertex2f(0.0f, 17.0f);
  gl.texCoord2f(0.375f, 0.0f);
  gl.vertex2f(static_cast<float>(video.xres()), 17.0f);
  gl.end();

  opengl::texture::disable_texture();
}

void UIMenuBar::resize()
{
  width(static_cast<float>(video.xres()));
}

void UIMenuBar::CloseAll()
{
  for (MenuPanes::iterator it(mMenuPanes.begin()), end(mMenuPanes.end()); it != end; ++it)
  {
    it->second->hide();
  }
}

void UIMenuBar::AddMenu(const std::string& pName)
{
  mMenuPanes[pName] = new MenuPane(1.0f + mNumMenus * 101.0f, 33.0f);

  addChild(mMenuPanes[pName]);
  addChild(new MenuButton(mMenuPanes[pName], 3.0f + mNumMenus * 100.0f, 5.0f, pName));

  mNumMenus++;
}

MenuPane::Ptr UIMenuBar::GetMenu(const std::string& pName)
{
  return mMenuPanes[pName];
}

UIFrame::Ptr UIMenuBar::processLeftClick(float mx, float my)
{
  UIFrame::Ptr tmp(UIFrame::processLeftClick(mx, my));
  if (tmp)
  {
    return tmp;
  }

  CloseAll();
  return nullptr;
}

MenuButton::MenuButton(MenuPane::Ptr pPane, float pX, float pY, const std::string& pText)
  : UIButton(pX, pY, 95.0f, 27.0f, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp")
  , mPane(pPane)
{
  setText(pText);
}

UIFrame* MenuButton::processLeftClick(float /*pX*/, float /*pY*/)
{
  clicked = true;
  mPane->Open();

  return this;
}

MenuItem::MenuItem(MenuPane::Ptr pParent, float pX, float pY, float pHeight, const std::string& pText, const std::string& pNormal, const std::string& pDown)
  : UIButton(pX, pY, pHeight, pText, pNormal, pDown)
  , mParent(pParent)
{
  setLeft();
}

MenuItemButton::MenuItemButton(MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, std::function<void()> fun)
  : MenuItem(pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp")
{
  setClickFunc (fun);
}

UIFrame* MenuItemButton::processLeftClick(float x, float y)
{
  UIButton::processLeftClick (x, y);

  mParent->Close();

  return this;
}

MenuItemToggle::MenuItemToggle(MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, bool * pMyState, bool pInvert)
  : MenuItem(pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp")
  , mMyCheckbox(new UICheckBox(147.0f, -1.0f, ""))
  , mMyState(pMyState)
  , mInvert(pInvert)
{
  setText(pText);
  setLeft();

  addChild(mMyCheckbox);

  mMyCheckbox->setState(*mMyState);
}

UIFrame* MenuItemToggle::processLeftClick(float /*pX*/, float /*pY*/)
{
  clicked = true;
  *mMyState = !(*mMyState);
  if (mInvert)
    mMyCheckbox->setState(!(*mMyState));
  else
    mMyCheckbox->setState(*mMyState);
  mParent->Close();
  return this;
}

void MenuItemToggle::render() const
{
  if (mInvert)
    mMyCheckbox->setState(!(*mMyState));
  else
    mMyCheckbox->setState(*mMyState);

  gl.color3f(1.0f, 1.0f, 1.0f);

  opengl::scoped::matrix_pusher const matrix;
  gl.translatef(x(), y(), 0.0f);

  opengl::texture::set_active_texture();
  opengl::texture::enable_texture();

  if (!clicked)
    texture->bind();
  else
    textureDown->bind();

  gl.begin(GL_TRIANGLE_STRIP);
  gl.texCoord2f(0.0f, 0.0f);
  gl.vertex2f(0.0f, 0.0f);
  gl.texCoord2f(1.0f, .0f);
  gl.vertex2f(width(), 0.0f);
  gl.texCoord2f(0.0f, 1.0f);
  gl.vertex2f(0.0f, height());
  gl.texCoord2f(1.0f, 1.0f);
  gl.vertex2f(width(), height());
  gl.end();

  opengl::texture::disable_texture();

  text->render();
  mMyCheckbox->render();
}

MenuItemSwitch::MenuItemSwitch(MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, bool * pMyState, bool pInvert)
  : MenuItem(pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp")
  , mMyState(pMyState)
  , mInvert(pInvert)
{
  setText(pText);
  setLeft();
}

UIFrame::Ptr MenuItemSwitch::processLeftClick(float /*pX*/, float /*pY*/)
{
  clicked = true;
  *mMyState = mInvert;
  mParent->Close();
  return this;
}


MenuItemSet::MenuItemSet(MenuPane::Ptr pParent, float pX, float pY, const std::string& pText, int * pMyState, int pSet)
  : MenuItem(pParent, pX, pY, 30.0f, pText, "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp")
  , mSet(pSet)
  , mMyState(pMyState)
{
  setText(pText);
  setLeft();
}

UIFrame::Ptr MenuItemSet::processLeftClick(float /*pX*/, float /*pY*/)
{
  clicked = true;
  *mMyState = mSet;
  mParent->Close();
  return this;
}

MenuItemSeperator::MenuItemSeperator(MenuPane::Ptr pParent, float pX, float pY, const std::string& pText)
  : MenuItem(pParent, pX, pY, 20.0f, pText, "Interface\\BUTTONS\\UI-SliderBar-Background.blp", "Interface\\BUTTONS\\UI-SliderBar-Background.blp")
{
  setText(pText);
}

UIFrame::Ptr MenuItemSeperator::processLeftClick(float /*pX*/, float /*pY*/)
{
  return nullptr;
}

MenuPane::MenuPane(float pX, float pY)
  : UIWindow(pX, pY, 180.0f, 1.0f)
{
  movable(false);
  hide();

  mNumItems = 0;
}

void MenuPane::Close()
{
  static_cast<UIMenuBar::Ptr>(parent())->CloseAll();
}

void MenuPane::Open()
{
  Close();
  show();
}

void MenuPane::fixSizes()
{
  height(6.0f + mNumItems * 25.0f);

  width(std::max((*_children.rbegin())->width() + 5.0f, width()));
  const float buttonWidth = width() - 5.0f;
  for (Children::iterator it(_children.begin()), end(_children.end())
    ; it != end; ++it)
  {
    (*it)->width(buttonWidth);
  }
}

void MenuPane::AddMenuItemButton(const std::string& pName, std::function<void()> fun)
{
  addChild(new MenuItemButton(this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, fun));
  fixSizes();
}
void MenuPane::AddMenuItemToggle(const std::string& pName, bool * pMyState, bool pInvert)
{
  addChild(new MenuItemToggle(this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pInvert));
  fixSizes();
}
void MenuPane::AddMenuItemSwitch(const std::string& pName, bool * pMyState, bool pInvert)
{
  addChild(new MenuItemSwitch(this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pInvert));
  fixSizes();
}
void MenuPane::AddMenuItemSet(const std::string& pName, int * pMyState, int pSet)
{
  addChild(new MenuItemSet(this, 5.0f, 5.0f + 25.0f * mNumItems++, pName, pMyState, pSet));
  fixSizes();
}
void MenuPane::AddMenuItemSeperator(const std::string& pName)
{
  addChild(new MenuItemSeperator(this, 5.0f, 5.0f + 25.0f * mNumItems++, pName));
  fixSizes();
}
