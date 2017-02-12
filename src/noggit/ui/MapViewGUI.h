// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

class UIToolbar;
class UIStatusBar;
class UIDetailInfos;
class UIDoodadSpawner;
class UIAppInfo;
class UIMinimapWindow;
class UIZoneIDBrowser;
class MapView;
class UIFrame;
class UITexturePicker;
class UITextureSwitcher;
class UIHelp;
class UICursorSwitcher;
class UIWindow;
class UIExitWarning;
class UISaveCurrentWarning;
class UICapsWarning;
class UIWaterSaveWarning;
class UIHelperModels;
class UISlider;
class UIGradient;
class UIWater;
class UIObjectEditor;
class UIRotationEditor;
namespace ui
{
  class FlattenTool;
  class shader_tool;
  class terrain_tool;
}

class UIModel;
class UIAlphamap;
class UIWaterTypeBrowser;

#include <noggit/ui/Frame.h>
#include <noggit/ui/CurrentTexture.h>

//! \todo Give better name.
class UIMapViewGUI : public UIFrame
{
private:
  bool _tilemode;
  UICursorSwitcher* CursorSwitcher;
  UIHelp* _help;
  //  UIDoodadSpawner* _test;


public:
  // position of the tools window(s)
  int tool_settings_x;
  int tool_settings_y;
  UISlider* paintPressureSlider;
  UISlider* paintHardnessSlider;
  UIGradient *G1;

  // Editor paramter
  int ground_edit_mode;
  int selection_view_mode;

  MapView* theMapview;
  // UI elements
  UIFrame* TexturePalette;
  UIModel* ModelBrowser;
  UIFrame* SelectedTexture;
  UIMinimapWindow* minimapWindow;
  UIToolbar* guiToolbar;
  UIStatusBar* guiStatusbar;
  UIDetailInfos* guidetailInfos;
  UIAppInfo* guiappInfo;
  UIZoneIDBrowser* ZoneIDBrowser;
  UITexturePicker* TexturePicker;
  UITextureSwitcher* TextureSwitcher;
  UIWater* guiWater;
  UIWaterTypeBrowser* guiWaterTypeSelector;
  UICurrentTexture* guiCurrentTexture;
  UIWindow *settings_paint;
  UIObjectEditor *objectEditor;
  UIRotationEditor *rotationEditor;
  ui::FlattenTool *flattenTool;
  ui::terrain_tool *terrainTool;
  ui::shader_tool *shaderTool;

  UIExitWarning *escWarning;
  UISaveCurrentWarning *scWarning;
  UICapsWarning *capsWarning;
  UIWaterSaveWarning *waterSaveWarning;
  UIHelperModels *HelperModels;

  explicit UIMapViewGUI(MapView* setMapview);

  void setTilemode(bool enabled);
  virtual void render() const;

  void showCursorSwitcher();
  void hideCursorSwitcher();
  void toggleCursorSwitcher();
  void showHelp();
  void hideHelp();
  void toggleHelp();

  void showTest();
  void hideTest();
  void toggleTest();
};
