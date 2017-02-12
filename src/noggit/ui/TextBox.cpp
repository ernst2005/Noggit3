// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/TextBox.h>

#include <utf8.h>

#include <string>

#include <noggit/application.h> // arial12
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/ui/Text.h>
#include <noggit/ui/Texture.h>
#include <noggit/Video.h>
#include <opengl/scoped.hpp>

//! \todo Handle Selection, Handle Clipboard ( CTRL + C / CTRL + V / CTRL + X ), Handle the Backspace staying down. Details, but better like that.

static const std::string texture("Interface\\Common\\Common-Input-Border.blp");
static const std::string textureFocused("Interface\\Common\\Common-Input-Border.blp");

UITextBox::UITextBox(float xPos, float yPos, float w, float h)
  : UIFrame(xPos, yPos, w, h)
  , _texture (texture)
  , _textureFocused (textureFocused)
  , _uiText(new UIText(8.0f, 2.5f, app.getArial16(), eJustifyLeft))
  , _value("")
  , _focus(false)
  , _enterFunction(nullptr)
  , _updateFunction(nullptr)
{
}

UITextBox::UITextBox(float xPos, float yPos, float w, float h, TriggerFunction enterFunction)
  : UIFrame(xPos, yPos, w, h)
  , _texture (texture)
  , _textureFocused (textureFocused)
  , _uiText(new UIText(8.0f, 2.5f, app.getArial16(), eJustifyLeft))
  , _value("")
  , _focus(false)
  , _enterFunction(enterFunction)
  , _updateFunction(nullptr)
{
}

UITextBox::UITextBox(float xPos, float yPos, float w, float h, const freetype::font_data& pFont)
  : UIFrame(xPos, yPos, w, h)
  , _texture (texture)
  , _textureFocused (textureFocused)
  , _uiText(new UIText(8.0f, 2.5f, pFont, eJustifyLeft))
  , _value("")
  , _focus(false)
  , _enterFunction(nullptr)
  , _updateFunction(nullptr)
{
}

UITextBox::UITextBox(float xPos, float yPos, float w, float h, const freetype::font_data& pFont, TriggerFunction enterFunction)
  : UIFrame(xPos, yPos, w, h)
  , _texture (texture)
  , _textureFocused (textureFocused)
  , _uiText(new UIText(8.0f, 2.5f, pFont, eJustifyLeft))
  , _value("")
  , _focus(false)
  , _enterFunction(enterFunction)
  , _updateFunction(nullptr)
{
}

void UITextBox::render() const
{
  opengl::scoped::matrix_pusher const matrix;
  gl.translatef(x(), y(), 0.0f);

  gl.color3f(1.0f, 1.0f, 1.0f);

  opengl::texture::set_active_texture();
  opengl::texture::enable_texture();

  if (_focus)
    _textureFocused->bind();
  else
    _texture->bind();

  gl.begin(GL_TRIANGLE_STRIP);
  gl.texCoord2f(0.0f, 0.0f);
  gl.vertex2f(0.0f, 0.0f);
  gl.texCoord2f(1.0f, 0.0f);
  gl.vertex2f(width(), 0.0f);
  gl.texCoord2f(0.0f, 1.0f);
  gl.vertex2f(0.0f, height());
  gl.texCoord2f(1.0f, 1.0f);
  gl.vertex2f(width(), height());
  gl.end();

  opengl::texture::disable_texture();

  _uiText->render();
}

UIFrame::Ptr UITextBox::processLeftClick(float /*mx*/, float /*my*/)
{
  _focus = !_focus;
  return this;
}

void UITextBox::value(const std::string& value_)
{
  _value = value_;
  _uiText->setText(value_);
  if (_updateFunction)
  {
    _updateFunction(this, _value);
  }
}

const std::string& UITextBox::value() const
{
  return _value;
}

bool UITextBox::key_down (SDLKey sym, Uint16 unicode)
{
  if (!_focus)
  {
    return false;
  }

  if (sym == SDLK_BACKSPACE && !_value.empty())
  {
    const char* firstBeforeEnd(_value.c_str() + _value.length());
    utf8::prior(firstBeforeEnd, _value.c_str());
    _value.erase(firstBeforeEnd - _value.c_str());
  }
  else if (sym == SDLK_ESCAPE)
  {
    _focus = false;
  }
  else if (sym == SDLK_RETURN)
  {
    _focus = false;
    if (_enterFunction)
    {
      _enterFunction(this, _value);
    }
  }
  else if (unicode)
  {
    utf8::append(unicode, std::back_inserter(_value));
  }

  _uiText->setText(_value);

  return true;
}

void UITextBox::clear()
{
  value("");
}
