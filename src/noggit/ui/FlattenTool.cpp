// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/ui/FlattenTool.hpp>

#include <noggit/application.h>
#include <noggit/ui/Text.h>
#include <noggit/ui/Slider.h>
#include <noggit/ui/TextBox.h>
#include <noggit/ui/CheckBox.h>
#include <noggit/ui/ToggleGroup.h>
#include <noggit/Misc.h>
#include <noggit/Environment.h>
#include <noggit/World.h>



namespace ui
{
  FlattenTool::FlattenTool(float x, float y)
    : UIWindow(x, y, (float)winWidth, (float)winHeight)
    , _radius(10.0f)
    , _speed(2.0f)
    , _angle(45.0f)
    , _orientation(0.0f)
    , _locked(false)
    , _angled_mode(false)
    , _flatten_type(eFlattenType_Linear)
    , _flatten_mode(eFlattenMode_Both)
  {
    addChild(new UIText(78.5f, 2.0f, "Flatten / Blur", app.getArial14(), eJustifyCenter));


    _type_toggle = new UIToggleGroup(&_flatten_type);
    addChild(new UICheckBox(6.0f, 15.0f, "Flat", _type_toggle, eFlattenType_Flat));
    addChild(new UICheckBox(80.0f, 15.0f, "Linear", _type_toggle, eFlattenType_Linear));
    addChild(new UICheckBox(6.0f, 40.0f, "Smooth", _type_toggle, eFlattenType_Smooth));
	addChild(new UICheckBox(80.0f, 40.0f, "Origin", _type_toggle, eFlattenType_Origin));

    _type_toggle->Activate(eFlattenType_Linear);

    _radius_slider = new UISlider(6.0f, 85.0f, 167.0f, 1000.0f, 0.00001f);
    _radius_slider->setFunc([&](float f) {_radius = f;});
    _radius_slider->setValue(_radius / 1000);
    _radius_slider->setText("Brush radius: ");
    addChild(_radius_slider);

    _speed_slider = new UISlider(6.0f, 110.0f, 167.0f, 10.0f, 0.00001f);
    _speed_slider->setFunc([&](float f) {_speed = f;});
    _speed_slider->setValue(_speed / 10.0f);
    _speed_slider->setText("Brush Speed: ");
    addChild(_speed_slider);


    addChild(new UIText(5.0f, 130.0f, "Flatten options:", app.getArial14(), eJustifyLeft));

    addChild (_angle_checkbox = new UICheckBox(6.0f, 150.0f, "Flatten Angle", &_angled_mode));

    _angle_slider = new UISlider(6.0f, 190.0f, 167.0f, 89.0f, 0.00001f);
    _angle_slider->setFunc( [&] (float f) { _angle = f; });
    _angle_slider->setValue(_angle / 89.0f);
    _angle_slider->setText("Angle: ");
    addChild(_angle_slider);

    _orientation_slider = new UISlider(6.0f, 220.0f, 167.0f, 360.0f, 0.00001f);
    _orientation_slider->setFunc( [&] (float f) { _orientation = f; });
    _orientation_slider->setValue(_orientation / 360.0f);
    _orientation_slider->setText("Orientation: ");
    addChild(_orientation_slider);


    addChild (_lock_checkbox = new UICheckBox(5.0f, 235.0f, "flatten relative to:", &_locked));

    addChild(new UIText(5.0f, 265.0f, "X:", app.getArial12(), eJustifyLeft));
    _lock_x = new UITextBox(50.0f, 265.0f, 100.0f, 30.0f, app.getArial12(), [&] (UITextBox::Ptr ptr, const std::string str) 
    {
      updateLockCoord(ptr, str, _lock_pos.x);
    });
    _lock_x->value(misc::floatToStr(_lock_pos.x));
    addChild(_lock_x);

    addChild(new UIText(5.0f, 285.0f, "Z:", app.getArial12(), eJustifyLeft));
    _lock_z = new UITextBox(50.0f, 285.0f, 100.0f, 30.0f, app.getArial12(), [&](UITextBox::Ptr ptr, const std::string str)
    {
      updateLockCoord(ptr, str, _lock_pos.z);
    });
    _lock_z->value(misc::floatToStr(_lock_pos.z));
    addChild(_lock_z);

    addChild(new UIText(5.0f, 305.0f, "Height:", app.getArial12(), eJustifyLeft));
    _lock_h = new UITextBox(50.0f, 305.0f, 100.0f, 30.0f, app.getArial12(), [&](UITextBox::Ptr ptr, const std::string str)
    {
      updateLockCoord(ptr, str, _lock_pos.y);
    });
    _lock_h->value(misc::floatToStr(_lock_pos.y));
    addChild(_lock_h);


    addChild(new UIText(5.0f, 330.0, "Lock:", app.getArial14(), eJustifyLeft));

    _mode_toggle = new UIToggleGroup(&_flatten_mode);
    addChild(new UICheckBox(5.0f, 345.0f, "Off", _mode_toggle, eFlattenMode_Both));
    addChild(new UICheckBox(105.0f, 345.0f, "Up", _mode_toggle, eFlattenMode_Raise));
    addChild(new UICheckBox(5.0f, 370.0f, "Down", _mode_toggle, eFlattenMode_Lower));
    _mode_toggle->Activate(eFlattenMode_Both);
  }

  void FlattenTool::flatten(math::vector_3d const& cursor_pos, float dt)
  {
    gWorld->flattenTerrain ( cursor_pos
                           , 1.f - pow (0.5f, dt *_speed)
                           , _radius
                           , _flatten_type
                           , _flatten_mode
                           , _locked ? _lock_pos : cursor_pos
                           , math::degrees (_angled_mode ? _angle : 0.0f)
                           , math::degrees (_angled_mode ? _orientation : 0.0f)
                           );
  }

  void FlattenTool::blur(math::vector_3d const& cursor_pos, float dt)
  {
    gWorld->blurTerrain ( cursor_pos
                        , 1.f - pow (0.5f, dt * _speed)
                        , _radius
                        , _flatten_type
                       );
  }

  void FlattenTool::nextFlattenType()
  {
    _flatten_type = (++_flatten_type) % eFlattenType_Count;
    _type_toggle->Activate(_flatten_type);
  }

  void FlattenTool::nextFlattenMode()
  {
    _flatten_mode = (++_flatten_mode) % eFlattenMode_Count;
    _mode_toggle->Activate(_flatten_mode);
  }

  void FlattenTool::toggleFlattenAngle()
  {
    _angled_mode = !_angled_mode;
    _angle_checkbox->setState(_angled_mode);
  }

  void FlattenTool::toggleFlattenLock()
  {
    _locked = !_locked;
    _lock_checkbox->setState(_locked);
  }

  void FlattenTool::lockPos (math::vector_3d const& cursor_pos)
  {
    _lock_pos = cursor_pos;
    _lock_x->value(misc::floatToStr(_lock_pos.x));
    _lock_z->value(misc::floatToStr(_lock_pos.z));
    _lock_h->value(misc::floatToStr(_lock_pos.y));

    if (!_locked)
    {
      toggleFlattenLock();
    }
  }

  void FlattenTool::changeRadius(float change)
  {
    _radius = std::max(0.0f, std::min(1000.0f, _radius + change));
    _radius_slider->setValue(_radius / 1000.0f);
  }

  void FlattenTool::changeSpeed(float change)
  {
    _speed = std::max(0.0f, std::min(10.0f, _speed + change));
    _speed_slider->setValue(_speed / 10.0f);
  }

  void FlattenTool::changeOrientation(float change)
  {
    _orientation += change;
    
    if (_orientation < 0.0f)
    {
      _orientation += 360.0f;
    }
    else if (_orientation > 360.0f)
    {
      _orientation -= 360.0f;
    }

    _orientation_slider->setValue(_orientation / 360.0f);
  }

  void FlattenTool::changeAngle(float change)
  {
    _angle = std::max(0.0f, std::min(89.0f, _angle + change));
    _angle_slider->setValue(_angle / 90.0f);
  }

  void FlattenTool::changeHeight(float change)
  {
    _lock_pos.y += change;
    _lock_h->value(misc::floatToStr(_lock_pos.y));
  }

  void FlattenTool::setRadius(float radius)
  {
    _radius = std::max(0.0f, std::min(1000.0f, radius));
    _radius_slider->setValue(_radius / 1000.0f);
  }

  void FlattenTool::updateLockCoord(UITextBox* cb, std::string const& str, float& value)
  {
    try
    {
      float val = std::atof(str.c_str());
      value = val;
      cb->value(misc::floatToStr(val));
    }
    catch (std::exception const & e)
    {
      cb->value(misc::floatToStr(value));
    }
  }
}

