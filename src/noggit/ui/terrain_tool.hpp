// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/Window.h>

class UIToggleGroup;
class UISlider;

namespace ui
{
  class terrain_tool : public UIWindow
  {
  public:
    terrain_tool(float x, float y, bool tablet);

    void changeTerrain(math::vector_3d const& pos, float dt);

    void nextType();
    void changeRadius(float change);
    void changeInnerRadius(float change);
    void changeSpeed(float change);
    
    // vertex edit only functions
    void moveVertices(float dt);
    void flattenVertices();
    void resetVertex();

    void changeOrientation(math::vector_3d const& pos, float change);
    void changeAngle(math::vector_3d const& pos, float change);
    void setOrientRelativeTo(math::vector_3d const& pos);

    // tablet
    void setTabletControlValue(float pressure);


    float brushRadius() const { return _radius; }
    float innerRadius() const { return _inner_radius;  }
  
  private:
    void updateVertices(math::vector_3d const& cursor_pos);

    static const int winWidth = 180;
    
    float _radius;
    float _speed;
    float _inner_radius;
    float& _angle;
    float& _orientation;

    bool _tablet;

    int& _edit_type;
    int _vertex_mode;
    int _tablet_control;
    int _tablet_active_group;
  
    // UI stuff:

    UIToggleGroup* _type_toggle;
    UIToggleGroup* _vertex_toggle;
    UIToggleGroup* _tablet_control_toggle = nullptr;
    UIToggleGroup* _tablet_active_group_toggle = nullptr;

    UISlider* _radius_slider;
    UISlider* _inner_radius_slider;
    UISlider* _speed_slider;
  };
}

