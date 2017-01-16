// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#undef _UNICODE

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#ifdef __FILESAREMISSING
#include <IL/il.h>
#endif

#include "Brush.h" // brush
#include "ConfigFile.h"
#include "DBC.h"
#include "Environment.h"
#include "FreeType.h" // freetype::
#include "Log.h"
#include "MapChunk.h"
#include "MapView.h"
#include "Misc.h"
#include "ModelManager.h" // ModelManager
#include "application.h" // app.getStates(), gPop, gFPS, app.getArial14(), morpheus40, arial...
#include "Project.h"
#include "Settings.h"
#include "Environment.h"
#include "TextureManager.h" // TextureManager, Texture
#include "UIAppInfo.h" // appInfo
#include "UICapsWarning.h"
#include "UICheckBox.h" // UICheckBox
#include "UICursorSwitcher.h" // UICursorSwitcher
#include "UIDetailInfos.h" // detailInfos
#include "UIExitWarning.h"
#include "UIGradient.h" // UIGradient
#include "UIHelperModels.h"
#include "UIMapViewGUI.h" // UIMapViewGUI
#include "UIMenuBar.h" // UIMenuBar, menu items, ..
#include "UIMinimapWindow.h" // UIMinimapWindow
#include "UIModelImport.h"
#include "UIObjectEditor.h"
#include "UIRotationEditor.h"
#include "UISlider.h" // UISlider
#include "UIStatusBar.h" // statusBar
#include "UIText.h" // UIText
#include "UITexture.h" // textureUI
#include "UITexturePicker.h"
#include "UITextureSwitcher.h"
#include "UITexturingGUI.h"
#include "UIToggleGroup.h" // UIToggleGroup
#include "UIToolbar.h" // UIToolbar
#include "UIToolbarIcon.h" // ToolbarIcon
#include "UIWater.h"
#include "UIWaterSaveWarning.h"
#include "UIWaterTypeBrowser.h"
#include "UIZoneIDBrowser.h"
#include "WMOInstance.h" // WMOInstance
#include "World.h"
#include "map_index.hpp"
#include <opengl/scoped.hpp>
#include <opengl/matrix.hpp>

static const float XSENS = 15.0f;
static const float YSENS = 15.0f;
static const float SPEED = 200.6f;

int MouseX;
int MouseY;
float mh, mv, rh, rv;

float moveratio = 0.1f;
float rotratio = 0.2f;
float keyx, keyy, keyz, keyr, keys;

int tool_settings_x;
int tool_settings_y;

bool MoveObj;

math::vector_3d objMove;
math::vector_3d objMoveOffset;
math::vector_3d objRot;

boost::optional<selection_type> lastSelected;

bool TestSelection = false;

extern bool DrawMapContour;
extern bool drawFlags;

// extern row and col form Palette UI


// This variables store the current status of the
// Shift, Alt and CTRL keys


bool  leftMouse = false;
bool  leftClicked = false;
bool  rightMouse = false;
bool  painting = false;

// Vars for the ground editing toggle mode
// store the status of some view settings when
// the ground editing mode is switched on
// to restore them if switch back again

bool  alloff = true;
bool  alloff_models = false;
bool  alloff_doodads = false;
bool  alloff_contour = false;
bool  alloff_wmo = false;
bool  alloff_detailselect = false;
bool  alloff_fog = false;
bool  alloff_terrain = false;


UISlider* shader_radius;
UISlider* shader_red;
UISlider* shader_green;
UISlider* shader_blue;
UISlider* shader_speed;
float shaderRadius = 15.0f;
float shaderSpeed = 1.0f;
float shaderRed = 1.0f;
float shaderGreen = 1.0f;
float shaderBlue = 1.0f;


UISlider* ground_brush_radius;
float groundBrushRadius = 15.0f;
UISlider* ground_brush_speed;
float groundBrushSpeed = 1.0f;
UISlider* ground_blur_speed;
float groundBlurSpeed = 2.0f;

#ifdef _WIN32
int		groundTabletControlSelect = 1;
int    groundTabletSelect = 1;
int shaderTabletControlSelect = 0;//Defaulting to off
#endif

UISlider* blur_brush;
float blurBrushRadius = 10.0f;
int    blurBrushType = 1;

int flattenType = eFlattenMode_Both;

UICheckBox* toggle_flatten;

UISlider* flatten_angle;
float flattenAngle = 0.0f;

UISlider* flatten_orientation;
float flattenOrientation = 0.0f;

math::vector_3d flattenRelativePos(0, 0, 0);
bool flattenRelativeMode = false;
UITextBox* flatten_relative_x;
UITextBox* flatten_relative_y;
UITextBox* flatten_relative_z;
UICheckBox* toggle_flatten_relative;

UISlider* paint_brush;
UISlider* spray_size;
UISlider* spray_pressure;

float brushPressure = 0.9f;
float brushLevel = 255.0f;
float brushSpraySize = 10.0f;
float brushSprayPressure = 5.0f;
bool sprayBrushActive = false;

UICheckBox* toggleSpray;

int terrainMode = 0;
int saveterrainMode = 0;

Brush textureBrush;
Brush sprayBrush;

UICursorSwitcher* CursorSwitcher;

bool Saving = false;

UIFrame* LastClicked;

// main GUI object
UIMapViewGUI* mainGui;

UIFrame* MapChunkWindow;

UIToggleGroup * gFlattenTypeGroup;
UIToggleGroup * gBlurToggleGroup;
UIToggleGroup * gGroundToggleGroup;
UIToggleGroup * gFlagsToggleGroup;

#ifdef _WIN32
UIToggleGroup * gGroundTabletControl;
UIToggleGroup * gShaderTabletControl;
UIToggleGroup * gGroundTabletActiveGroup;
#endif

UIWindow *setting_ground;
UIWindow *setting_blur;
UIWindow *settings_paint;
UIWindow *settings_shader;

void toggleFlattenLock(bool b, int)
{
  flattenRelativeMode = b;
}

void updateFlattenRelativeX(UITextBox::Ptr textBox, const std::string& value)
{
  try
  {
    float val = std::atof(value.c_str());
    flattenRelativePos.x = val;
    flatten_relative_x->value(misc::floatToStr(val));
  }
  catch (std::exception const & e)
  {
    flatten_relative_x->value(misc::floatToStr(flattenRelativePos.x));
  }
}

void updateFlattenRelativeY(UITextBox::Ptr textBox, const std::string& value)
{
  try
  {
    float val = std::atof(value.c_str());
    flattenRelativePos.y = val;
    flatten_relative_y->value(misc::floatToStr(val));
  }
  catch (std::exception const & e)
  {
    flatten_relative_x->value(misc::floatToStr(flattenRelativePos.y));
  }
}

void updateFlattenRelativeZ(UITextBox::Ptr textBox, const std::string& value)
{
  try
  {
    float val = std::atof(value.c_str());
    flattenRelativePos.z = val;
    flatten_relative_z->value(misc::floatToStr(val));
  }
  catch (std::exception const & e)
  {
    flatten_relative_x->value(misc::floatToStr(flattenRelativePos.z));
  }
}

void setGroundBrushRadius(float f)
{
  groundBrushRadius = f;
}

void SetShaderRadius(float f)
{
  shaderRadius = f;
}

void SetShaderSpeed(float f)
{
  shaderSpeed = f;
}

void SetShaderRed(float f)
{
  shaderRed = f;
  Environment::getInstance()->cursorColorR = f / 2;
}

void SetShaderGreen(float f)
{
  shaderGreen = f;
  Environment::getInstance()->cursorColorG = f / 2;
}

void SetShaderBlue(float f)
{
  shaderBlue = f;
  Environment::getInstance()->cursorColorB = f / 2;
}

void setGroundBrushSpeed(float f)
{
  groundBrushSpeed = f;
}


void setBlurBrushRadius(float f)
{
  blurBrushRadius = f;
}

void setBlurBrushSpeed(float f)
{
  groundBlurSpeed = f;
}

void setFlattenAngle(float f)
{
  flattenAngle = f;
  Environment::getInstance()->flattenAngle = f;
}

void setFlattenOrientation(float f)
{
  flattenOrientation = f;
  Environment::getInstance()->flattenOrientation = (M_PI*f) / 180;
}


void setTextureBrushHardness(float f)
{
  textureBrush.setHardness(f);
  sprayBrush.setHardness(f);
}

void setTextureBrushRadius(float f)
{
  textureBrush.setRadius(f);
}

void setTextureBrushPressure(float f)
{
  brushPressure = f;
}

void toggleSprayBrush(bool b, int)
{
  sprayBrushActive = b;
}

void toggleHighlightPaintable(bool b, int)
{
  Environment::getInstance()->highlightPaintableChunks = b;
}

void setSprayBrushSize(float f)
{
  brushSpraySize = f;
  sprayBrush.setRadius(std::max(f, 1.0f) * TEXDETAILSIZE / 2.0f);
}

void setSprayBrushPressure(float f)
{
  brushSprayPressure = f;
}

void setTextureBrushLevel(float f)
{
  brushLevel = (1.0f - f)*255.0f;
}

void toggleFlattenAngle(bool state, int)
{
  Environment::getInstance()->flattenAngleEnabled = state;
}

void SaveOrReload(UIFrame*, int pMode)
{
  if (pMode == 1)
  {
    gWorld->mapIndex->reloadTile(tile_index(gWorld->camera));
  }
  else if (pMode == 0)
  {
    gWorld->mapIndex->saveTile(tile_index(gWorld->camera));
  }
  else if (pMode == 2)
  {
    gWorld->mapIndex->saveChanged();
  }
  else if (pMode == 3)
  {
    mainGui->escWarning->show();
  }
}

void change_settings_window(int oldid, int newid)
{
  if (oldid + 1 == newid || !setting_ground || !setting_blur || !settings_paint
    || !settings_shader || !mainGui || !mainGui->guiWater || !mainGui->objectEditor)
  {
    return;
  }

  mainGui->guiWaterTypeSelector->hide();
  setting_ground->hide();
  setting_blur->hide();
  settings_paint->hide();
  settings_shader->hide();
  mainGui->guiWater->hide();
  mainGui->TextureSwitcher->hide();
  mainGui->objectEditor->hide();
  mainGui->objectEditor->filename->hide();
  mainGui->objectEditor->modelImport->hide();
  mainGui->rotationEditor->hide();
  mainGui->ZoneIDBrowser->hide();

  if (!mainGui || !mainGui->TexturePalette)
    return;
  mainGui->TexturePalette->hide();
  // fetch old win position
  switch (oldid)
  {
  case 1:
    tool_settings_x = (int)setting_ground->x();
    tool_settings_y = (int)setting_ground->y();
    break;
  case 2:
    tool_settings_x = (int)setting_blur->x();
    tool_settings_y = (int)setting_blur->y();
    break;
  case 3:
    tool_settings_x = (int)settings_paint->x();
    tool_settings_y = (int)settings_paint->y();
    break;
  case 5:
    tool_settings_x = (int)mainGui->ZoneIDBrowser->x() + 230;
    tool_settings_y = (int)mainGui->ZoneIDBrowser->y();
    break;
  case 7:
    tool_settings_x = (int)mainGui->guiWater->x();
    tool_settings_y = (int)mainGui->guiWater->y();
    break;
  case 9:
    tool_settings_x = (int)settings_shader->x();
    tool_settings_y = (int)settings_shader->y();
    break;
  }
  // set new win pos and make visible
  switch (newid)
  {
  case 1:
    setting_ground->x((const float)tool_settings_x);
    setting_ground->y((const float)tool_settings_y);
    setting_ground->show();
    break;
  case 2:
    setting_blur->x((const float)tool_settings_x);
    setting_blur->y((const float)tool_settings_y);
    setting_blur->show();
    break;
  case 3:
    settings_paint->x((const float)tool_settings_x);
    settings_paint->y((const float)tool_settings_y);
    settings_paint->show();
    break;
  case 5:
    mainGui->ZoneIDBrowser->x((const float)tool_settings_x - 230);
    mainGui->ZoneIDBrowser->y((const float)tool_settings_y);
    mainGui->ZoneIDBrowser->show();
    break;
  case 7:
    mainGui->guiWater->x((const float)tool_settings_x);
    mainGui->guiWater->y((const float)tool_settings_y);
    mainGui->guiWater->show();
    break;
  case 9:
    settings_shader->x((const float)tool_settings_x);
    settings_shader->y((const float)tool_settings_y);
    settings_shader->show();
    break;
  case 10:
    mainGui->objectEditor->x((const float)tool_settings_x - 90.0f);
    mainGui->objectEditor->y((const float)tool_settings_y);
    mainGui->objectEditor->show();
    mainGui->objectEditor->filename->show();
    mainGui->rotationEditor->x(mainGui->objectEditor->getX() - mainGui->rotationEditor->getW() - 10.0f);
    mainGui->rotationEditor->y((const float)tool_settings_y);
  }
}

void openSwapper(UIFrame*, int)
{
  mainGui->TextureSwitcher->show();
  settings_paint->hide();
}

void removeTexDuplicateOnADT(UIFrame*, int)
{
  gWorld->removeTexDuplicateOnADT(tile_index(gWorld->camera));
}

void openHelp(UIFrame*, int)
{
  mainGui->showHelp();
}


void openURL(UIFrame*, int target)
{
#if defined(_WIN32) || defined(WIN32)
  if (target == 1)  ShellExecute(nullptr, "open", "http://modcraft.superparanoid.de", nullptr, nullptr, SW_SHOWNORMAL);
  if (target == 2)  ShellExecute(nullptr, "open", "http://modcraft.superparanoid.de/wiki/index.php5?title=Noggit_user_manual", nullptr, nullptr, SW_SHOWNORMAL);
#endif
}

void ResetSelectedObjectRotation(UIFrame*, int)
{
  if (gWorld->IsSelection(eEntry_WMO))
  {
    WMOInstance* wmo = boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection());
    gWorld->updateTilesWMO(wmo);
    wmo->resetDirection();
    gWorld->updateTilesWMO(wmo);
  }
  else if (gWorld->IsSelection(eEntry_Model))
  {
    ModelInstance* m2 = boost::get<selected_model_type> (*gWorld->GetCurrentSelection());
    gWorld->updateTilesModel(m2);
    m2->resetDirection();
    m2->recalcExtents();
    gWorld->updateTilesModel(m2);
  }
}

void SnapSelectedObjectToGround(UIFrame*, int)
{
  if (gWorld->IsSelection(eEntry_WMO))
  {
    WMOInstance* wmo = boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection());
    math::vector_3d t = math::vector_3d(wmo->pos.x, wmo->pos.z, 0);
    gWorld->GetVertex(wmo->pos.x, wmo->pos.z, &t);
    wmo->pos.y = t.y;
    gWorld->updateTilesWMO(wmo);
  }
  else if (gWorld->IsSelection(eEntry_Model))
  {
    ModelInstance* m2 = boost::get<selected_model_type> (*gWorld->GetCurrentSelection());
    math::vector_3d t = math::vector_3d(m2->pos.x, m2->pos.z, 0);
    gWorld->GetVertex(m2->pos.x, m2->pos.z, &t);
    m2->pos.y = t.y;
    gWorld->updateTilesModel(m2);
  }
}


void DeleteSelectedObject(UIFrame*, int)
{
  if (gWorld->IsSelection(eEntry_WMO))
  {
    gWorld->deleteWMOInstance(boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->mUniqueID);
  }
  else if (gWorld->IsSelection(eEntry_Model))
  {
    gWorld->deleteModelInstance(boost::get<selected_model_type> (*gWorld->GetCurrentSelection())->d1);
  }
}

void showHelperModels(UIFrame*, int)
{
  mainGui->HelperModels->show();
}

/*!
\brief Import a new model form a text file or a hard coded one.
Imports a model from the import.txt (or the ImportFile from the config), the wowModelViewer log or just insert some hard coded testing models.
\param id the id switch the import kind
*/
void InsertObject(UIFrame*, int id)
{
  //! \todo Beautify.

  // Test if there is an selection
  if (!gWorld->HasSelection())
    return;
  // the list of the models to import
  std::vector<std::string> m2s_to_add;
  std::vector<std::string> wmos_to_add;

  // the import file
  std::string importFile;

  const char* filesToAdd[15] = { "", "", "World\\Scale\\humanmalescale.m2", "World\\Scale\\50x50.m2", "World\\Scale\\100x100.m2", "World\\Scale\\250x250.m2", "World\\Scale\\500x500.m2", "World\\Scale\\1000x1000.m2", "World\\Scale\\50yardradiusdisc.m2", "World\\Scale\\200yardradiusdisc.m2", "World\\Scale\\777yardradiusdisc.m2", "World\\Scale\\50yardradiussphere.m2", "World\\Scale\\200yardradiussphere.m2", "World\\Scale\\777yardradiussphere.m2", "" };

  // MODELINSERT FROM TEXTFILE
  // is a source file set in config file?


  if (id == 0 || id == 14 || id == 15)
  {
    importFile = Settings::getInstance()->wmvLogFile;
  }
  else if (id == 1 || id > 99)
  {
    importFile = Settings::getInstance()->importFile;
  }
  else
  {
    m2s_to_add.push_back(filesToAdd[id]);
  }

  LogDebug << id << "-" << importFile << std::endl;

  std::string lastModel;
  std::string lastWMO;

  if (importFile != "")
  {
    size_t foundString;
    std::string line;
    std::string findThis;
    std::ifstream fileReader(importFile.c_str());
    int counter = 1;
    if (fileReader.is_open())
    {
      while (!fileReader.eof())
      {
        getline(fileReader, line);
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);

        if (line.find(".m2") != std::string::npos || line.find(".mdx") != std::string::npos)
        {
          if (id < 99 || (id - 99) == counter)
          {
            if (id == 14)
            {
              // M2 inside line
              // is it the modelviewer log then cut the log messages out
              findThis = "loading model: ";
              foundString = line.find(findThis);
              if (foundString != std::string::npos)
              {
                // cut path
                line = line.substr(foundString + findThis.size());
              }
              else
              {
                // invalid line
                continue;
              }
            }
            // swap mdx to m2
            size_t found = line.rfind(".mdx");
            if (found != std::string::npos)
            {
              line.replace(found, 4, ".m2");
            }

            line = line.substr(0, line.find(".m2") + 3);

            m2s_to_add.push_back(line);
            lastModel = line;
          }
          counter++;
        }
        else if (line.find(".wmo") != std::string::npos)
        {
          if (id < 99 || (id - 99) == counter)
          {
            if (id == 15)
            {
              // WMO inside line
              findThis = "loading wmo ";
              foundString = line.find(findThis);
              // is it the modelviewer log then cut the log messages out
              if (foundString != std::string::npos)
              {
                // cut path
                line = line.substr(foundString + findThis.size());
              }
              else
              {
                // invalid line
                continue;
              }
            }

            line = line.substr(0, line.find(".wmo") + 4);
            wmos_to_add.push_back(line);
            lastWMO = line;
          }
          counter++;
        }
      }
      fileReader.close();
    }
    else
    {
      // file not exist, no rights ore other error
      LogError << importFile << std::endl;
    }
  }


  math::vector_3d selectionPosition;
  switch (gWorld->GetCurrentSelection()->which())
  {
  case eEntry_Model:
    selectionPosition = boost::get<selected_model_type> (*gWorld->GetCurrentSelection())->pos;
    break;
  case eEntry_WMO:
    selectionPosition = boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->pos;
    break;
  case eEntry_MapChunk:
    selectionPosition = boost::get<selected_chunk_type> (*gWorld->GetCurrentSelection()).position;
    break;
  }


  if (id == 14)
  {
    LogDebug << "M2 Problem 14:" << lastModel << " - " << id << std::endl;
    if (lastModel != "")
    {
      if (!MPQFile::exists(lastModel))
      {
        LogError << "Failed adding " << lastModel << ". It was not in any MPQ." << std::endl;
      }
      else
      {
        gWorld->addM2(lastModel, selectionPosition, false);
      }
    }
  }
  else if (id == 15)
  {
    LogDebug << "Wmo Problem 15:" << lastModel << " - " << id << std::endl;
    if (lastWMO != "")
    {
      if (!MPQFile::exists(lastWMO))
      {
        LogError << "Failed adding " << lastWMO << ". It was not in any MPQ." << std::endl;
      }
      else
      {
        gWorld->addWMO(lastWMO, selectionPosition, false);
      }
    }
  }
  else
  {

    for (std::vector<std::string>::iterator it = wmos_to_add.begin(); it != wmos_to_add.end(); ++it)
    {

      if (!MPQFile::exists(*it))
      {
        LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
        continue;
      }

      gWorld->addWMO(*it, selectionPosition, false);
    }

    for (std::vector<std::string>::iterator it = m2s_to_add.begin(); it != m2s_to_add.end(); ++it)
    {

      if (!MPQFile::exists(*it))
      {

        LogError << "Failed adding " << *it << ". It was not in any MPQ." << std::endl;
        continue;
      }

      gWorld->addM2(*it, selectionPosition, false);
    }
  }
  //! \todo Memoryleak: These models will never get deleted.
}

void view_texture_palette(UIFrame*, int)
{
  mainGui->TexturePalette->toggleVisibility();
}

void exit_tilemode(UIFrame*, int)
{
  app.pop = true;
}

void test_menu_action(UIFrame*, int)
{
  gWorld->saveWDT();
}

void moveHeightmap(UIFrame*, int)
{
  // set areaid on all chunks of the current ADT
  if (Environment::getInstance()->selectedAreaID)
  {
    gWorld->moveHeight(Environment::getInstance()->selectedAreaID, tile_index(gWorld->camera));
  }
}

void clearHeightmap(UIFrame*, int)
{
  // set areaid on all chunks of the current ADT
  if (Environment::getInstance()->selectedAreaID)
  {
    gWorld->clearHeight(Environment::getInstance()->selectedAreaID, tile_index(gWorld->camera));
  }
}

void adtSetAreaID(UIFrame*, int)
{
  // set areaid on all chunks of the current ADT
  if (Environment::getInstance()->selectedAreaID)
  {
    gWorld->setAreaID(Environment::getInstance()->selectedAreaID, tile_index(gWorld->camera));
  }
}

void clearAllModels(UIFrame*, int)
{
  // call the clearAllModelsOnADT method to clear them all on current ADT
  gWorld->clearAllModelsOnADT(tile_index(gWorld->camera));
}

void ClearDupModels(UIFrame*, int)
{
  gWorld->delete_duplicate_model_and_wmo_instances();
}

void menuWater(UIFrame*, int id)
{
  if (id == 1)
  {
    gWorld->addWaterLayer(tile_index(gWorld->camera));
  }
  else if (id == 0)
  {
    gWorld->deleteWaterLayer(tile_index(gWorld->camera));
  }
}

void funcAllFix(UIFrame*, int id)
{
  gWorld->fixAllGaps();
}

void ClearShader(UIFrame*, int id)
{
  gWorld->ClearShader(tile_index(gWorld->camera));
}

void toBigAlpha(UIFrame*, int)
{
  gWorld->convertMapToBigAlpha();
}

void changeZoneIDValue(UIFrame* /*f*/, int set)
{
  Environment::getInstance()->selectedAreaID = set;
  if (Environment::getInstance()->areaIDColors.find(set) == Environment::getInstance()->areaIDColors.end())
  {
    math::vector_3d newColor = math::vector_3d(misc::randfloat(0.0f, 1.0f), misc::randfloat(0.0f, 1.0f), misc::randfloat(0.0f, 1.0f));
    Environment::getInstance()->areaIDColors.insert(std::pair<int, math::vector_3d>(set, newColor));
  }
}

std::string getCurrentHeightmapPath()
{
  // get MapName
  std::string mapName;
  int id = gWorld->getMapID();
  for (DBCFile::Iterator i = gMapDB.begin(); i != gMapDB.end(); ++i)
  {
    if (i->getInt(MapDB::MapID) == id)
      mapName = i->getString(MapDB::InternalName);
  }

  // build the path and filename string.
  std::stringstream png_filename;
  png_filename << Project::getInstance()->getPath() << "world\\maps\\" << mapName << "\\H_" << mapName
    << "_" << misc::FtoIround((gWorld->camera.x - (TILESIZE / 2)) / TILESIZE) << "_" << misc::FtoIround((gWorld->camera.z - (TILESIZE / 2)) / TILESIZE) << ".png";
  return png_filename.str();

}

void clearTexture(UIFrame* /*f*/, int /*set*/)
{
  // set areaid on all chunks of the current ADT
  gWorld->setBaseTexture(tile_index(gWorld->camera));
}


void showCursorSwitcher(UIFrame* /*f*/, int /*set*/)
{
  mainGui->showCursorSwitcher();
}

#ifdef __FILESAREMISSING
void exportPNG(UIFrame *f, int set)
{
  // create the image and write to disc.
  GLfloat* data = new GLfloat[272 * 272];

  ilInit();

  int width = 272;
  int height = 272;
  int bytesToUsePerPixel = 32;  // 16 bit per channel
  int sizeOfByte = sizeof(unsigned char);
  int theSize = width * height * sizeOfByte * bytesToUsePerPixel;

  unsigned char * imData = (unsigned char*)malloc(theSize);

  int colors = 0;
  // write the height data to the image array
  for (int i = 0; i < theSize; i++)
  {
    imData[i] = colors;
    if (i == 100)colors = 200;
    if (i == 200)colors = 4000;
  }


  ILuint ImageName; // The image name.
  ilGenImages(1, &ImageName); // Grab a new image name.
  ilBindImage(ImageName); // bind it
  ilTexImage(width, height, 1, bytesToUsePerPixel, GL_LUMINANCE, IL_UNSIGNED_BYTE, nullptr);
  ilSetData(imData);
  ilEnable(IL_FILE_OVERWRITE);
  //ilSave(IL_PNG, getCurrentHeightmapPath().c_str());
  ilSave(IL_PNG, "test2.png");
  free(imData);
}

void importPNG(UIFrame *f, int set)
{
  ilInit();

  //ILboolean loadImage = ilLoadImage( getCurrentHeightmapPath().c_str() ) ;
  const char *image = "test.png";
  ILboolean loadImage = ilLoadImage(image);

  std::stringstream MessageText;
  if (loadImage)
  {

    LogDebug << "Image loaded: " << image << "\n";
    LogDebug << "ImageSize: " << ilGetInteger(IL_IMAGE_SIZE_OF_DATA) << "\n";
    LogDebug << "BPP: " << ilGetInteger(IL_IMAGE_BITS_PER_PIXEL) << "\n";
    LogDebug << "Format: " << ilGetInteger(IL_IMAGE_FORMAT) << "\n";
    LogDebug << "SizeofData: " << ilGetInteger(IL_IMAGE_SIZE_OF_DATA) << "\n";

  }
  else
  {
    LogDebug << "Cant load Image: " << image << "\n";
    ILenum err = ilGetError();

    MessageText << err << "\n";
    //MessageText << ilGetString(ilGetError()) << "\n";
    LogDebug << MessageText.str();
  }
}
#else
void exportPNG(UIFrame*, int) {}
void importPNG(UIFrame*, int) {}
#endif

void MapView::createGUI()
{
  // create main gui object that holds all other gui elements for access ( in the future ;) )
  mainGui = new UIMapViewGUI(this);
  mainGui->guiCurrentTexture->current_texture->setClickFunc(view_texture_palette, 0);

  mainGui->ZoneIDBrowser->setMapID(gWorld->getMapID());
  mainGui->ZoneIDBrowser->setChangeFunc(changeZoneIDValue);
  tool_settings_x = video.xres() - 186;
  tool_settings_y = 38;


  // Raise/Lower
#ifdef _WIN32
  if (app.tabletActive && Settings::getInstance()->tabletMode)
    setting_ground = new UIWindow((float)tool_settings_x, (float)tool_settings_y, 180.0f, 240.0f);
  else
#endif
    setting_ground = new UIWindow((float)tool_settings_x, (float)tool_settings_y, 180.0f, 160.0f);

  setting_ground->movable(true);
  mainGui->addChild(setting_ground);

  setting_ground->addChild(new UIText(78.5f, 2.0f, "Raise / Lower", app.getArial14(), eJustifyCenter));

  gGroundToggleGroup = new UIToggleGroup(&Environment::getInstance()->groundBrushType);
  setting_ground->addChild(new UICheckBox(6.0f, 15.0f, "Flat", gGroundToggleGroup, 0));
  setting_ground->addChild(new UICheckBox(85.0f, 15.0f, "Linear", gGroundToggleGroup, 1));
  setting_ground->addChild(new UICheckBox(6.0f, 40.0f, "Smooth", gGroundToggleGroup, 2));
  setting_ground->addChild(new UICheckBox(85.0f, 40.0f, "Polynomial", gGroundToggleGroup, 3));
  setting_ground->addChild(new UICheckBox(6.0f, 65.0f, "Trigonom", gGroundToggleGroup, 4));
  setting_ground->addChild(new UICheckBox(85.0f, 65.0f, "Quadratic", gGroundToggleGroup, 5));
  gGroundToggleGroup->Activate(1);

  ground_brush_radius = new UISlider(6.0f, 120.0f, 167.0f, 1000.0f, 0.00001f);
  ground_brush_radius->setFunc(setGroundBrushRadius);
  ground_brush_radius->setValue(groundBrushRadius / 1000);
  ground_brush_radius->setText("Brush radius: ");
  setting_ground->addChild(ground_brush_radius);

  ground_brush_speed = new UISlider(6.0f, 145.0f, 167.0f, 10.0f, 0.00001f);
  ground_brush_speed->setFunc(setGroundBrushSpeed);
  ground_brush_speed->setValue(groundBrushSpeed / 10);
  ground_brush_speed->setText("Brush Speed: ");
  setting_ground->addChild(ground_brush_speed);

#ifdef _WIN32
  if (app.tabletActive && Settings::getInstance()->tabletMode)
  {
    setting_ground->addChild(new UIText(78.5f, 170.0f, "Tablet Control", app.getArial14(), eJustifyCenter));

    gGroundTabletControl = new UIToggleGroup(&groundTabletControlSelect);
    setting_ground->addChild(new UICheckBox(6.0f, 182.0f, "Off", gGroundTabletControl, 0));
    setting_ground->addChild(new UICheckBox(85.0f, 182.0f, "On", gGroundTabletControl, 1));
    gGroundTabletControl->Activate(1);

    gGroundTabletActiveGroup = new UIToggleGroup(&groundTabletSelect);
    setting_ground->addChild(new UICheckBox(6.0f, 207.0f, "Radius", gGroundTabletActiveGroup, 0));
    setting_ground->addChild(new UICheckBox(85.0f, 207.0f, "Speed", gGroundTabletActiveGroup, 1));
    gGroundTabletActiveGroup->Activate(1);
  }
#endif

  // shader
#ifdef _WIN32
  if (app.tabletActive && Settings::getInstance()->tabletMode)
    settings_shader = new UIWindow((float)tool_settings_x, (float)tool_settings_y, 180.0f, 200.0f);
  else
#endif
    settings_shader = new UIWindow((float)tool_settings_x, (float)tool_settings_y, 180.0f, 160.0f);
  settings_shader->movable(true);
  settings_shader->hide();
  mainGui->addChild(settings_shader);

  settings_shader->addChild(new UIText(78.5f, 2.0f, "Shader", app.getArial14(), eJustifyCenter));

  shader_radius = new UISlider(6.0f, 33.0f, 167.0f, 1000.0f, 0.00001f);
  shader_radius->setFunc(SetShaderRadius);
  shader_radius->setValue(shaderRadius / 1000);
  shader_radius->setText("Radius: ");
  settings_shader->addChild(shader_radius);

  shader_speed = new UISlider(6.0f, 59.0f, 167.0f, 10.0f, 0.00001f);
  shader_speed->setFunc(SetShaderSpeed);
  shader_speed->setValue(shaderSpeed / 10.0f);
  shader_speed->setText("Speed: ");
  settings_shader->addChild(shader_speed);

  shader_red = new UISlider(6.0f, 85.0f, 167.0f, 2.0f, 0.00001f);
  shader_red->setFunc(SetShaderRed);
  shader_red->setValue(shaderRed / 2.0f);
  shader_red->setText("Red: ");
  settings_shader->addChild(shader_red);

  shader_green = new UISlider(6.0f, 111.0f, 167.0f, 2.0f, 0.00001f);
  shader_green->setFunc(SetShaderGreen);
  shader_green->setValue(shaderGreen / 2.0f);
  shader_green->setText("Green: ");
  settings_shader->addChild(shader_green);

  shader_blue = new UISlider(6.0f, 137.0f, 167.0f, 2.0f, 0.00001f);
  shader_blue->setFunc(SetShaderBlue);
  shader_blue->setValue(shaderBlue / 2.0f);
  shader_blue->setText("Blue: ");
  settings_shader->addChild(shader_blue);
#ifdef _WIN32
  if (app.tabletActive && Settings::getInstance()->tabletMode)
  {
    settings_shader->addChild(new UIText(78.5f, 137.0f, "Tablet Control", app.getArial14(), eJustifyCenter));

    gShaderTabletControl = new UIToggleGroup(&shaderTabletControlSelect);
    settings_shader->addChild(new UICheckBox(6.0f, 151.0f, "Off", gShaderTabletControl, 0));
    settings_shader->addChild(new UICheckBox(85.0f, 151.0f, "On", gShaderTabletControl, 1));
    gShaderTabletControl->Activate(0);
  }
#endif

  // flatten/blur
  setting_blur = new UIWindow((float)tool_settings_x, (float)tool_settings_y, 180.0f, 400.0f);
  setting_blur->movable(true);
  setting_blur->hide();
  mainGui->addChild(setting_blur);

  setting_blur->addChild(new UIText(78.5f, 2.0f, "Flatten / Blur", app.getArial14(), eJustifyCenter));

  gBlurToggleGroup = new UIToggleGroup(&blurBrushType);
  setting_blur->addChild(new UICheckBox(6.0f, 15.0f, "Flat", gBlurToggleGroup, 0));
  setting_blur->addChild(new UICheckBox(80.0f, 15.0f, "Linear", gBlurToggleGroup, 1));
  setting_blur->addChild(new UICheckBox(6.0f, 40.0f, "Smooth", gBlurToggleGroup, 2));
  gBlurToggleGroup->Activate(1);

  blur_brush = new UISlider(6.0f, 85.0f, 167.0f, 1000.0f, 0.00001f);
  blur_brush->setFunc(setBlurBrushRadius);
  blur_brush->setValue(blurBrushRadius / 1000);
  blur_brush->setText("Brush radius: ");
  setting_blur->addChild(blur_brush);

  ground_blur_speed = new UISlider(6.0f, 110.0f, 167.0f, 10.0f, 0.00001f);
  ground_blur_speed->setFunc(setBlurBrushSpeed);
  ground_blur_speed->setValue(groundBlurSpeed / 10);
  ground_blur_speed->setText("Brush Speed: ");
  setting_blur->addChild(ground_blur_speed);

  setting_blur->addChild(new UIText(5.0f, 130.0f, "Flatten options:", app.getArial14(), eJustifyLeft));

  toggle_flatten = new UICheckBox(6.0f, 150.0f, "Flatten Angle", toggleFlattenAngle, 0);
  toggle_flatten->setState(Environment::getInstance()->flattenAngleEnabled);
  setting_blur->addChild(toggle_flatten);

  flatten_angle = new UISlider(6.0f, 190.0f, 167.0f, 90.0f, 0.00001f);
  flatten_angle->setValue(flattenAngle / 10);
  flatten_angle->setText("Angle: ");
  flatten_angle->setFunc(setFlattenAngle);
  setting_blur->addChild(flatten_angle);

  flatten_orientation = new UISlider(6.0f, 220.0f, 167.0f, 360.0f, 0.00001f);
  flatten_orientation->setValue(flattenOrientation / 10);
  flatten_orientation->setText("Orientation: ");
  flatten_orientation->setFunc(setFlattenOrientation);
  setting_blur->addChild(flatten_orientation);

  toggle_flatten_relative = new UICheckBox(5.0f, 235.0f, "flatten relative to:", toggleFlattenLock, 0);
  toggle_flatten_relative->setState(flattenRelativeMode);
  setting_blur->addChild(toggle_flatten_relative);

  setting_blur->addChild(new UIText(5.0f, 265.0f, "X:", app.getArial12(), eJustifyLeft));
  flatten_relative_x = new UITextBox(50.0f, 265.0f, 100.0f, 30.0f, app.getArial12(), updateFlattenRelativeX);
  flatten_relative_x->value(misc::floatToStr(flattenRelativePos.x));
  setting_blur->addChild(flatten_relative_x);

  setting_blur->addChild(new UIText(5.0f, 285.0f, "Z:", app.getArial12(), eJustifyLeft));
  flatten_relative_z = new UITextBox(50.0f, 285.0f, 100.0f, 30.0f, app.getArial12(), updateFlattenRelativeZ);
  flatten_relative_z->value(misc::floatToStr(flattenRelativePos.z));
  setting_blur->addChild(flatten_relative_z);

  setting_blur->addChild(new UIText(5.0f, 305.0f, "Height:", app.getArial12(), eJustifyLeft));
  flatten_relative_y = new UITextBox(50.0f, 305.0f, 100.0f, 30.0f, app.getArial12(), updateFlattenRelativeY);
  flatten_relative_y->value(misc::floatToStr(flattenRelativePos.y));
  setting_blur->addChild(flatten_relative_y);

  setting_blur->addChild(new UIText(5.0f, 330.0, "Flatten Type:", app.getArial14(), eJustifyLeft));

  gFlattenTypeGroup = new UIToggleGroup(&flattenType);

  setting_blur->addChild(new UICheckBox(5.0f, 345.0f, "Raise/Lower", gFlattenTypeGroup, eFlattenMode_Both));
  setting_blur->addChild(new UICheckBox(105.0f, 345.0f, "Raise", gFlattenTypeGroup, eFlattenMode_Raise));
  setting_blur->addChild(new UICheckBox(5.0f, 370.0f, "Lower", gFlattenTypeGroup, eFlattenMode_Lower));

  gFlattenTypeGroup->Activate(flattenType);


  //3D Paint settings UIWindow
  settings_paint = new UIWindow((float)tool_settings_x, (float)tool_settings_y, 180.0f, 280.0f);
  settings_paint->hide();
  settings_paint->movable(true);

  mainGui->addChild(settings_paint);

  settings_paint->addChild(new UIText(78.5f, 2.0f, "3D Paint", app.getArial14(), eJustifyCenter));


  mainGui->G1 = new UIGradient;
  mainGui->G1->width(20.0f);
  mainGui->G1->x(settings_paint->width() - 4.0f - mainGui->G1->width());
  mainGui->G1->y(4.0f);
  mainGui->G1->height(92.0f);
  mainGui->G1->setMaxColor(1.0f, 1.0f, 1.0f, 1.0f);
  mainGui->G1->setMinColor(0.0f, 0.0f, 0.0f, 1.0f);
  mainGui->G1->horiz = false;
  mainGui->G1->setClickColor(1.0f, 0.0f, 0.0f, 1.0f);
  mainGui->G1->setClickFunc(setTextureBrushLevel);
  mainGui->G1->setValue(0.0f);

  settings_paint->addChild(mainGui->G1);

  mainGui->paintHardnessSlider = new UISlider(6.0f, 33.0f, 145.0f, 1.0f, 0.0f);
  mainGui->paintHardnessSlider->setFunc(setTextureBrushHardness);
  mainGui->paintHardnessSlider->setValue(textureBrush.getHardness());
  mainGui->paintHardnessSlider->setText("Hardness: ");
  settings_paint->addChild(mainGui->paintHardnessSlider);

  paint_brush = new UISlider(6.0f, 59.0f, 145.0f, 100.0f, 0.00001f);
  paint_brush->setFunc(setTextureBrushRadius);
  paint_brush->setValue(textureBrush.getRadius() / 100);
  paint_brush->setText("Radius: ");
  settings_paint->addChild(paint_brush);

  mainGui->paintPressureSlider = new UISlider(6.0f, 85.0f, 145.0f, 0.99f, 0.01f);
  mainGui->paintPressureSlider->setFunc(setTextureBrushPressure);
  mainGui->paintPressureSlider->setValue(brushPressure);
  mainGui->paintPressureSlider->setText("Pressure: ");
  settings_paint->addChild(mainGui->paintPressureSlider);

  UICheckBox* toggleHighlight = new UICheckBox(3.0f, 105.0f, "Hightligh paintable chunks", toggleHighlightPaintable, 0);
  toggleHighlight->setState(Environment::getInstance()->highlightPaintableChunks);
  settings_paint->addChild(toggleHighlight);


  toggleSpray = new UICheckBox(3.0f, 138.0f, "Toggle spray", toggleSprayBrush, 0);
  settings_paint->addChild(toggleSpray);

  spray_size = new UISlider(6.0f, 180.0f, 170.0f, 40.0f, 0.0001f);
  spray_size->setFunc(setSprayBrushSize);
  spray_size->setValue(brushSpraySize / 40.0f);
  spray_size->setText("Spray size: ");
  settings_paint->addChild(spray_size);

  spray_pressure = new UISlider(6.0f, 205.0f, 170.0f, 100.0f, 0.0001f);
  spray_pressure->setFunc(setSprayBrushPressure);
  spray_pressure->setValue(brushSprayPressure / 100.0f);
  spray_pressure->setText("Spray pressure (/1k): ");
  settings_paint->addChild(spray_pressure);

  UIButton* B1;
  B1 = new UIButton(6.0f, 230.0f, 170.0f, 30.0f, "Texture swapper", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", openSwapper, 1);
  settings_paint->addChild(B1);

  UIButton* rmDup = new UIButton(6.0f, 255.0f, 170.0f, 30.0f, "Remove texture duplicates", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", removeTexDuplicateOnADT, 0);
  settings_paint->addChild(rmDup);

  mainGui->addChild(mainGui->TexturePalette = UITexturingGUI::createTexturePalette(mainGui));
  mainGui->TexturePalette->hide();
  //mainGui->addChild(mainGui->SelectedTexture = UITexturingGUI::createSelectedTexture());
  //mainGui->SelectedTexture->hide();
  mainGui->addChild(UITexturingGUI::createTilesetLoader());
  mainGui->addChild(UITexturingGUI::createTextureFilter());
  mainGui->addChild(MapChunkWindow = UITexturingGUI::createMapChunkWindow());
  MapChunkWindow->hide();

  // create the menu
  UIMenuBar * mbar = new UIMenuBar();

  mbar->AddMenu("File");
  mbar->AddMenu("Edit");
  mbar->AddMenu("View");
  mbar->AddMenu("Assist");
  mbar->AddMenu("Help");

  // mbar->GetMenu( "File" )->AddMenuItemButton( "CTRL+SHIFT+S Save current", SaveOrReload, 0 );
  mbar->GetMenu("File")->AddMenuItemButton("CTRL+S Save", SaveOrReload, 2);
  // mbar->GetMenu( "File" )->AddMenuItemButton( "SHIFT+J Reload tile", SaveOrReload, 1 );
  //  mbar->GetMenu( "File" )->AddMenuItemSeperator( "Import and Export" );
  // mbar->GetMenu( "File" )->AddMenuItemButton( "Export heightmap", exportPNG, 1 );
  // mbar->GetMenu( "File" )->AddMenuItemButton( "Import heightmap", importPNG, 1 );
  mbar->GetMenu("File")->AddMenuItemSeperator(" ");
  mbar->GetMenu("File")->AddMenuItemButton("ESC Exit", SaveOrReload, 3);

  //  mbar->GetMenu( "File" )->AddMenuItemSeperator( "Test" );
  //mbar->GetMenu( "File" )->AddMenuItemButton( "AreaID", test_menu_action, 1 );

  mbar->GetMenu("Edit")->AddMenuItemSeperator("selected object");
  mbar->GetMenu("Edit")->AddMenuItemButton("DEL delete", DeleteSelectedObject, 0);
  mbar->GetMenu("Edit")->AddMenuItemButton("CTRL + R reset rotation", ResetSelectedObjectRotation, 0);
  mbar->GetMenu("Edit")->AddMenuItemButton("PAGE DOWN set to ground", SnapSelectedObjectToGround, 0);

  mbar->GetMenu("Edit")->AddMenuItemSeperator("Options");
  mbar->GetMenu("Edit")->AddMenuItemToggle("Auto select mode", &Settings::getInstance()->AutoSelectingMode, false);


  mbar->GetMenu("Assist")->AddMenuItemSeperator("Model");
  mbar->GetMenu("Assist")->AddMenuItemButton("Last M2 from MV", InsertObject, 14);
  mbar->GetMenu("Assist")->AddMenuItemButton("Last WMO from MV", InsertObject, 15);
  mbar->GetMenu("Assist")->AddMenuItemButton("Helper models", showHelperModels, 2);
  mbar->GetMenu("Assist")->AddMenuItemSeperator("ADT");
  mbar->GetMenu("Assist")->AddMenuItemButton("Set Area ID", adtSetAreaID, 0);
  mbar->GetMenu("Assist")->AddMenuItemButton("Clear height map", clearHeightmap, 0);

  mbar->GetMenu("Assist")->AddMenuItemButton("Clear texture", clearTexture, 0);
  mbar->GetMenu("Assist")->AddMenuItemButton("Clear models", clearAllModels, 0);
  mbar->GetMenu("Assist")->AddMenuItemButton("Clear duplicate models", ClearDupModels, 0);
  mbar->GetMenu("Assist")->AddMenuItemButton("Clear water", menuWater, 0);
  mbar->GetMenu("Assist")->AddMenuItemButton("Create water", menuWater, 1);
  mbar->GetMenu("Assist")->AddMenuItemButton("Fix gaps (all loaded adts)", funcAllFix, 0);
  mbar->GetMenu("Assist")->AddMenuItemButton("Clear standard shader", ClearShader, 0);
  mbar->GetMenu("Assist")->AddMenuItemButton("Map to big alpha", toBigAlpha, 0);

  mbar->GetMenu("View")->AddMenuItemSeperator("Windows");
  mbar->GetMenu("View")->AddMenuItemToggle("Toolbar", mainGui->guiToolbar->hidden_evil(), true);

  mbar->GetMenu("View")->AddMenuItemToggle("Texture palette", mainGui->TexturePalette->hidden_evil(), true);
  mbar->GetMenu("View")->AddMenuItemButton("Cursor options", showCursorSwitcher, 0);
  mbar->GetMenu("View")->AddMenuItemSeperator("Toggle");
  mbar->GetMenu("View")->AddMenuItemToggle("F1 M2s", &gWorld->drawmodels);
  mbar->GetMenu("View")->AddMenuItemToggle("F2 WMO doodadsets", &gWorld->drawdoodads);
  mbar->GetMenu("View")->AddMenuItemToggle("F3 Terrain", &gWorld->drawterrain);
  mbar->GetMenu("View")->AddMenuItemToggle("F4 Water", &gWorld->drawwater);
  mbar->GetMenu("View")->AddMenuItemToggle("F6 WMOs", &gWorld->drawwmo);
  mbar->GetMenu("View")->AddMenuItemToggle("F7 Lines", &gWorld->drawlines);
  mbar->GetMenu("View")->AddMenuItemToggle("F8 Detail infos", mainGui->guidetailInfos->hidden_evil(), true);
  mbar->GetMenu("View")->AddMenuItemToggle("F9 Map contour infos", &DrawMapContour);
  mbar->GetMenu("View")->AddMenuItemToggle("F11 Toggle Animation", &gWorld->renderAnimations);
  mbar->GetMenu("View")->AddMenuItemToggle("F Fog", &gWorld->drawfog);
  mbar->GetMenu("View")->AddMenuItemToggle("Hole lines always on", &Settings::getInstance()->holelinesOn, false);
  mbar->GetMenu("View")->AddMenuItemToggle("Wireframe", &gWorld->drawwireframe);
  mbar->GetMenu("View")->AddMenuItemToggle("Models with box", &Settings::getInstance()->renderModelsWithBox);

  mbar->GetMenu("Help")->AddMenuItemButton("Key Bindings F10", openHelp, 0);
  mbar->GetMenu("Help")->AddMenuItemButton("Manual online", openURL, 2);
  mbar->GetMenu("Help")->AddMenuItemButton("Homepage", openURL, 1);

  mainGui->addChild(mbar);


  addHotkey(SDLK_ESCAPE, MOD_none, static_cast<AppState::Function>(&MapView::quitask));
  addHotkey(SDLK_s, MOD_ctrl, static_cast<AppState::Function>(&MapView::save));
  addHotkey(SDLK_s, MOD_meta, static_cast<AppState::Function>(&MapView::save));

  // ESC warning
  mainGui->escWarning = new UIExitWarning(this);
  mainGui->escWarning->hide();
  mainGui->addChild(mainGui->escWarning);

  // CAPS warning
  mainGui->capsWarning = new UICapsWarning(this);
  mainGui->capsWarning->hide();
  mainGui->addChild(mainGui->capsWarning);

  // Water unable to save warning
  mainGui->waterSaveWarning = new UIWaterSaveWarning(this);
  mainGui->waterSaveWarning->hide();
  mainGui->addChild(mainGui->waterSaveWarning);

  // modelimport
  mainGui->objectEditor->modelImport = new UIModelImport(this);
  mainGui->objectEditor->modelImport->hide();
  mainGui->addChild(mainGui->objectEditor->modelImport);

  // helper models
  mainGui->HelperModels = new UIHelperModels(this);
  mainGui->HelperModels->hide();
  mainGui->HelperModels->movable(true);
  mainGui->addChild(mainGui->HelperModels);
}

MapView::MapView(float ah0, float av0)
  : ah(ah0)
  , av(av0)
  , _GUIDisplayingEnabled(true)
  , mTimespeed(0.0f)
{
  LastClicked = nullptr;

  moving = strafing = updown = lookat = turn = 0.0f;

  mousedir = -1.0f;

  movespd = SPEED;

  lastBrushUpdate = 0;
  textureBrush.init();
  sprayBrush.init();
  setSprayBrushSize(10.0f);

  look = false;
  mViewMode = eViewMode_3D;

  createGUI();

  // Set camera y (height) position to current ground height plus some space.
  math::vector_3d t = math::vector_3d(0, 0, 0);
  tile_index tile(gWorld->camera);
  if (!gWorld->mapIndex->tileLoaded(tile))
  {
    gWorld->mapIndex->loadTile(tile);
  }

  gWorld->GetVertex(gWorld->camera.x, gWorld->camera.z, &t);

  // min elevation according to https://wowdev.wiki/AreaTable.dbc
  //! \ todo use the current area's MinElevation
  if (t.y < -5000.0f)
  {
    //! \todo use the height of a model/wmo of the tile (or the map) ?
    t.y = 0.0f;
  }

  gWorld->camera.y = t.y + 50.0f;
}

MapView::~MapView()
{
  delete mainGui;
  mainGui = nullptr;
  delete gWorld;
  gWorld = nullptr;
}

void MapView::tick(float t, float dt)
{

  // start unloading tiles
  gWorld->mapIndex->unloadTiles(tile_index(gWorld->camera));

  dt = std::min(dt, 1.0f);

  // write some stuff into infos window for debuging
  std::stringstream appinfoText;
  appinfoText << "Project Path: " << Project::getInstance()->getPath() << std::endl;
  appinfoText << "Current cursor: " << Environment::getInstance()->cursorType << std::endl;
  mainGui->guiappInfo->setText(appinfoText.str());
#ifdef _WIN32
  if (app.tabletActive && Settings::getInstance()->tabletMode)
  {
    switch (terrainMode)
    {
    case 0:
      switch (groundTabletSelect)
      {
      case 0:
        if (groundTabletControlSelect == 1)
        {
          groundBrushRadius = std::max(0.0f, std::min(1000.0f, (float)app.pressure / 20.48f));
          ground_brush_radius->setValue(groundBrushRadius / 1000.0f);
        }
        break;
      case 1:
        if (groundTabletControlSelect == 1)
        {
          groundBrushSpeed = std::max(0.0f, std::min(10.0f, (float)app.pressure / 204.8f));
          ground_brush_speed->setValue(groundBrushSpeed / 10.0f);
        }
        break;
      }
      break;

    case 1:
      blurBrushRadius = std::max(0.0f, std::min(1000.0f, (float)app.pressure / 20.0f));
      blur_brush->setValue(blurBrushRadius / 1000.0f);
      break;
    case 2:
      mainGui->paintPressureSlider->setValue(std::max(0.0f, std::min(1.0f, (float)app.pressure / 2048.0f)));
      mainGui->paintPressureSlider->setValue(mainGui->paintPressureSlider->value);
      break;
    case 8:
      if (shaderTabletControlSelect == 1)
      {
        shaderRadius = std::max(0.0f, std::min(1000.0f, (float)app.pressure / 20.48f));
        shader_radius->setValue(shaderRadius / 1000.0f);
      }

      break;
    }
  }
#endif

  if (SDL_GetAppState() & SDL_APPINPUTFOCUS)
  {
    math::vector_3d dir(1.0f, 0.0f, 0.0f);
    math::vector_3d dirUp(1.0f, 0.0f, 0.0f);
    math::vector_3d dirRight(0.0f, 0.0f, 1.0f);
    math::rotate(0.0f, 0.0f, &dir.x, &dir.y, math::degrees(av));
    math::rotate(0.0f, 0.0f, &dir.x, &dir.z, math::degrees(ah));

    if (Environment::getInstance()->ShiftDown)
    {
      dirUp.x = 0.0f;
      dirUp.y = 1.0f;
      dirRight *= 0.0f; //! \todo  WAT?
    }
    else if (Environment::getInstance()->CtrlDown)
    {
      dirUp.x = 0.0f;
      dirUp.y = 1.0f;
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.y, math::degrees(av));
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.y, math::degrees(av));
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.z, math::degrees(ah));
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.z, math::degrees(ah));
    }
    else
    {
      math::rotate(0.0f, 0.0f, &dirUp.x, &dirUp.z, math::degrees(ah));
      math::rotate(0.0f, 0.0f, &dirRight.x, &dirRight.z, math::degrees(ah));
    }
    auto Selection = gWorld->GetCurrentSelection();
    if (Selection)
    {
      // update rotation editor if the selection has changed
      if (!lastSelected || lastSelected != Selection)
      {
        mainGui->rotationEditor->select(*Selection);
      }

      bool canMoveObj = !mainGui->rotationEditor->hasFocus();

      // Set move scale and rotate for numpad keys
      if (Environment::getInstance()->CtrlDown && Environment::getInstance()->ShiftDown)  moveratio = 0.1f;
      else if (Environment::getInstance()->ShiftDown) moveratio = 0.01f;
      else if (Environment::getInstance()->CtrlDown) moveratio = 0.005f;
      else moveratio = 0.001f;

      if (canMoveObj && (keyx != 0 || keyy != 0 || keyz != 0 || keyr != 0 || keys != 0))
      {
        // Move scale and rotate with numpad keys
        if (Selection->which() == eEntry_WMO)
        {
          gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          boost::get<selected_wmo_type> (*Selection)->pos.x += keyx * moveratio;
          boost::get<selected_wmo_type> (*Selection)->pos.y += keyy * moveratio;
          boost::get<selected_wmo_type> (*Selection)->pos.z += keyz * moveratio;
          boost::get<selected_wmo_type> (*Selection)->dir.y += keyr * moveratio * 5;

          boost::get<selected_wmo_type> (*Selection)->recalcExtents();
          gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          mainGui->rotationEditor->updateValues();
        }

        if (Selection->which() == eEntry_Model)
        {
          gWorld->updateTilesModel(boost::get<selected_model_type> (*Selection));
          boost::get<selected_model_type> (*Selection)->pos.x += keyx * moveratio;
          boost::get<selected_model_type> (*Selection)->pos.y += keyy * moveratio;
          boost::get<selected_model_type> (*Selection)->pos.z += keyz * moveratio;
          boost::get<selected_model_type> (*Selection)->dir.y += keyr * moveratio * 5;
          boost::get<selected_model_type> (*Selection)->sc += keys * moveratio / 50;
          boost::get<selected_model_type> (*Selection)->recalcExtents();
          gWorld->updateTilesModel(boost::get<selected_model_type> (*Selection));
          mainGui->rotationEditor->updateValues();
        }
      }

      math::vector_3d ObjPos;
      if (gWorld->IsSelection(eEntry_Model))
      {
        //! \todo  Tell me what this is.
        ObjPos = boost::get<selected_model_type> (*Selection)->pos - gWorld->camera;
        math::rotate(0.0f, 0.0f, &ObjPos.x, &ObjPos.y, math::degrees(av));
        math::rotate(0.0f, 0.0f, &ObjPos.x, &ObjPos.z, math::degrees(ah));
        ObjPos.x = abs(ObjPos.x);
      }

      // moving and scaling objects
      //! \todo  Alternatively automatically align it to the terrain.
      if (MoveObj && canMoveObj)
      {
        ObjPos.x = 80.0f;
        if (Selection->which() == eEntry_WMO)
        {
          gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));

          if (Environment::getInstance()->ShiftDown)
          {
            boost::get<selected_wmo_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
            boost::get<selected_wmo_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
          }
          else
          {
            if (Environment::getInstance()->moveModelToCursorPos)
            {
              boost::get<selected_wmo_type> (*Selection)->pos.x = Environment::getInstance()->Pos3DX - objMoveOffset.x;
              boost::get<selected_wmo_type> (*Selection)->pos.z = Environment::getInstance()->Pos3DZ - objMoveOffset.z;
            }
            else
            {
              boost::get<selected_wmo_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
              boost::get<selected_wmo_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
            }
          }

          boost::get<selected_wmo_type> (*Selection)->recalcExtents();
          gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          mainGui->rotationEditor->updateValues();
        }
        else if (Selection->which() == eEntry_Model)
        {
          gWorld->updateTilesModel(boost::get<selected_model_type> (*Selection));
          if (Environment::getInstance()->AltDown)
          {
            float ScaleAmount = pow(2.0f, mv * 4.0f);

            boost::get<selected_model_type> (*Selection)->sc *= ScaleAmount;
            if (boost::get<selected_model_type> (*Selection)->sc > 63.9f)
              boost::get<selected_model_type> (*Selection)->sc = 63.9f;
            else if (boost::get<selected_model_type> (*Selection)->sc < 0.00098f)
              boost::get<selected_model_type> (*Selection)->sc = 0.00098f;
          }
          else
          {
            if (Environment::getInstance()->ShiftDown)
            {
              boost::get<selected_model_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
              boost::get<selected_model_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
            }
            else
            {
              if (Environment::getInstance()->moveModelToCursorPos)
              {
                boost::get<selected_model_type> (*Selection)->pos.x = Environment::getInstance()->Pos3DX - objMoveOffset.x;
                boost::get<selected_model_type> (*Selection)->pos.z = Environment::getInstance()->Pos3DZ - objMoveOffset.z;
              }
              else
              {
                boost::get<selected_model_type> (*Selection)->pos += mv * dirUp * ObjPos.x;
                boost::get<selected_model_type> (*Selection)->pos -= mh * dirRight * ObjPos.x;
              }

            }
          }

          mainGui->rotationEditor->updateValues();
          boost::get<selected_model_type> (*Selection)->recalcExtents();
          gWorld->updateTilesModel(boost::get<selected_model_type> (*Selection));
        }
      }


      // rotating objects
      if (look && canMoveObj)
      {
        float * lTarget = nullptr;
        bool lModify = false;

        if (Selection->which() == eEntry_Model)
        {
          lModify = Environment::getInstance()->ShiftDown | Environment::getInstance()->CtrlDown | Environment::getInstance()->AltDown;
          if (Environment::getInstance()->ShiftDown)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.y;
          else if (Environment::getInstance()->CtrlDown)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.x;
          else if (Environment::getInstance()->AltDown)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.z;

        }
        else if (Selection->which() == eEntry_WMO)
        {
          lModify = Environment::getInstance()->ShiftDown | Environment::getInstance()->CtrlDown | Environment::getInstance()->AltDown;
          if (Environment::getInstance()->ShiftDown)
            lTarget = &boost::get<selected_wmo_type> (*Selection)->dir.y;
          else if (Environment::getInstance()->CtrlDown)
            lTarget = &boost::get<selected_wmo_type> (*Selection)->dir.x;
          else if (Environment::getInstance()->AltDown)
            lTarget = &boost::get<selected_wmo_type> (*Selection)->dir.z;

        }

        if (lModify && lTarget)
        {
          gWorld->updateTilesEntry(*Selection);

          *lTarget = *lTarget + rh + rv;

          if (*lTarget > 360.0f)
            *lTarget = *lTarget - 360.0f;
          else if (*lTarget < -360.0f)
            *lTarget = *lTarget + 360.0f;

          mainGui->rotationEditor->updateValues();

          if (Selection->which() == eEntry_WMO)
          {
            boost::get<selected_wmo_type> (*Selection)->recalcExtents();
            gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));
          }
          else if (Selection->which() == eEntry_Model)
          {
            boost::get<selected_model_type> (*Selection)->recalcExtents();
            gWorld->updateTilesModel(boost::get<selected_model_type> (*Selection));
          }
        }
      }

      mh = 0;
      mv = 0;
      rh = 0;
      rv = 0;


      if (leftMouse && Selection->which() == eEntry_MapChunk)
      {
        float xPos, yPos, zPos;

        xPos = Environment::getInstance()->Pos3DX;
        yPos = Environment::getInstance()->Pos3DY;
        zPos = Environment::getInstance()->Pos3DZ;

        bool underMap = gWorld->isUnderMap(xPos, zPos, yPos);

        switch (terrainMode)
        {
        case 0:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (Environment::getInstance()->ShiftDown)
            {
              gWorld->changeTerrain(xPos, zPos, 7.5f * dt * groundBrushSpeed, groundBrushRadius, Environment::getInstance()->groundBrushType);
            }
            else if (Environment::getInstance()->CtrlDown)
            {
              gWorld->changeTerrain(xPos, zPos, -7.5f * dt * groundBrushSpeed, groundBrushRadius, Environment::getInstance()->groundBrushType);
            }
          }
          break;
        case 1:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (Environment::getInstance()->ShiftDown)
            {
              if (flattenRelativeMode)
              {
                if (Environment::getInstance()->flattenAngleEnabled)
                {
                  gWorld->flattenTerrain(xPos, zPos, pow(0.5f, dt * groundBlurSpeed), blurBrushRadius, blurBrushType, flattenType, flattenRelativePos, flattenAngle, flattenOrientation);
                }
                else
                {
                  gWorld->flattenTerrain(xPos, zPos, pow(0.5f, dt * groundBlurSpeed), blurBrushRadius, blurBrushType, flattenType, flattenRelativePos);
                }
              }
              else
              {
                if (Environment::getInstance()->flattenAngleEnabled)
                {
                  gWorld->flattenTerrain(xPos, zPos, yPos, pow(0.5f, dt * groundBlurSpeed), blurBrushRadius, blurBrushType, flattenType, flattenAngle, flattenOrientation);
                }
                else
                {
                  gWorld->flattenTerrain(xPos, zPos, yPos, pow(0.5f, dt * groundBlurSpeed), blurBrushRadius, blurBrushType, flattenType);
                }
              }

            }
            else if (Environment::getInstance()->CtrlDown)
            {
              gWorld->blurTerrain(xPos, zPos, pow(0.5f, dt * groundBlurSpeed), std::min(blurBrushRadius, 30.0f), blurBrushType);
            }
          }
          break;
        case 2:
          if (Environment::getInstance()->ShiftDown && Environment::getInstance()->CtrlDown && Environment::getInstance()->AltDown)
          {
            // clear chunk texture
            if (mViewMode == eViewMode_3D && !underMap)
              gWorld->eraseTextures(xPos, zPos);
            else if (mViewMode == eViewMode_2D)
              gWorld->eraseTextures(CHUNKSIZE * 4.0f * video.ratio() * (static_cast<float>(MouseX) / static_cast<float>(video.xres()) - 0.5f) / gWorld->zoom + gWorld->camera.x, CHUNKSIZE * 4.0f * (static_cast<float>(MouseY) / static_cast<float>(video.yres()) - 0.5f) / gWorld->zoom + gWorld->camera.z);
          }
          else if (Environment::getInstance()->CtrlDown)
          {
            // Pick texture
            mainGui->TexturePicker->getTextures(*gWorld->GetCurrentSelection());
          }
          else  if (Environment::getInstance()->ShiftDown)
          {
            // Paint 3d if shift down.
            if (UITexturingGUI::getSelectedTexture())
            {
              if (textureBrush.needUpdate())
              {
                textureBrush.GenerateTexture();
              }
              if (mViewMode == eViewMode_3D && !underMap)
              {
                if (mainGui->TextureSwitcher->hidden())
                {
                  if (sprayBrushActive)
                  {
                    gWorld->sprayTexture(xPos, zPos, &sprayBrush, brushLevel, 1.0f - pow(1.0f - brushPressure, dt * 10.0f),
                      textureBrush.getRadius(), brushSprayPressure,
                      UITexturingGUI::getSelectedTexture()
                    );
                  }
                  else
                  {
                    gWorld->paintTexture(xPos, zPos, &textureBrush, brushLevel, 1.0f - pow(1.0f - brushPressure, dt * 10.0f), UITexturingGUI::getSelectedTexture());
                  }
                }
                else
                {
                  gWorld->overwriteTextureAtCurrentChunk(xPos, zPos, mainGui->TextureSwitcher->getTextures(), UITexturingGUI::getSelectedTexture());
                }
              }
            }
            // paint 2d if nothing is pressed.
            if (textureBrush.needUpdate())
            {
              textureBrush.GenerateTexture();
            }
            if (mViewMode == eViewMode_2D)
              gWorld->paintTexture(CHUNKSIZE * 4.0f * video.ratio() * (static_cast<float>(MouseX) / static_cast<float>(video.xres()) - 0.5f) / gWorld->zoom + gWorld->camera.x, CHUNKSIZE * 4.0f * (static_cast<float>(MouseY) / static_cast<float>(video.yres()) - 0.5f) / gWorld->zoom + gWorld->camera.z, &textureBrush, brushLevel, 1.0f - pow(1.0f - brushPressure, dt * 10.0f), UITexturingGUI::getSelectedTexture());
          }
          break;

        case 3:
          if (mViewMode == eViewMode_3D)
          {
            // no undermap check here, else it's impossible to remove holes
            if (Environment::getInstance()->ShiftDown)
            {
              boost::get<selected_chunk_type> (*Selection).chunk->getSelectionCoord(&xPos, &zPos);
              gWorld->removeHole(xPos, zPos, Environment::getInstance()->AltDown);
            }
            else if (Environment::getInstance()->CtrlDown && !underMap)
              gWorld->addHole(xPos, zPos, Environment::getInstance()->AltDown);
          }
          break;
        case 4:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (Environment::getInstance()->ShiftDown)
            {
              // draw the selected AreaId on current selected chunk
              MapChunk* chnk (boost::get<selected_chunk_type> (*gWorld->GetCurrentSelection()).chunk);
              gWorld->setAreaID(Environment::getInstance()->selectedAreaID, chnk->mt->index, chnk->px, chnk->py);
            }
            else if (Environment::getInstance()->CtrlDown)
            {
              // pick areaID from chunk
              MapChunk* chnk (boost::get<selected_chunk_type> (*gWorld->GetCurrentSelection()).chunk);
              int newID = chnk->getAreaID();
              Environment::getInstance()->selectedAreaID = newID;
              mainGui->ZoneIDBrowser->setZoneID(newID);
            }
          }
          break;
        case 5:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (Environment::getInstance()->ShiftDown)
            {
              gWorld->mapIndex->setFlag(true, xPos, zPos);
            }
            else if (Environment::getInstance()->CtrlDown)
            {
              gWorld->mapIndex->setFlag(false, xPos, zPos);
            }
          }
          break;
        case 6:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            auto lSelection = gWorld->GetCurrentSelection();
            MapChunk* chnk = boost::get<selected_chunk_type> (*Selection).chunk;

            if (Environment::getInstance()->ShiftDown)
            {
              gWorld->addWaterLayerChunk(chnk->mt->index, chnk->px, chnk->py);
            }
            if (Environment::getInstance()->CtrlDown && !Environment::getInstance()->AltDown)
            {
              gWorld->delWaterLayerChunk(chnk->mt->index, chnk->px, chnk->py);
            }
            if (Environment::getInstance()->AltDown && !Environment::getInstance()->CtrlDown)
            {
              gWorld->mapIndex->setWater(true, xPos, zPos);
            }
            if (Environment::getInstance()->AltDown && Environment::getInstance()->CtrlDown)
              gWorld->mapIndex->setWater(false, xPos, zPos);
          }
          break;
        case 8:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (Environment::getInstance()->ShiftDown)
              gWorld->changeShader(xPos, zPos, dt*shaderSpeed * 2, shaderRadius, true);
            if (Environment::getInstance()->CtrlDown)
              gWorld->changeShader(xPos, zPos, dt*shaderSpeed * 2, shaderRadius, false);
          }
          break;
        }
      }
    }

    if (mViewMode != eViewMode_2D)
    {
      if (turn != 0.0f)
      {
        ah += turn;
        mainGui->minimapWindow->changePlayerLookAt(math::degrees (ah));
      }
      if (lookat)
      {
        av += lookat;
        if (av < -80.0f)
          av = -80.0f;
        else if (av > 80.0f)
          av = 80.0f;
        mainGui->minimapWindow->changePlayerLookAt(math::degrees (ah));
      }
      if (moving)
        gWorld->camera += dir * dt * movespd * moving;
      if (strafing)
      {
        math::vector_3d right = dir % math::vector_3d(0.0f, 1.0f, 0.0f);
        right.normalize();
        gWorld->camera += right * dt * movespd * strafing;
      }
      if (updown)
        gWorld->camera += math::vector_3d(0.0f, dt * movespd * updown, 0.0f);

      gWorld->lookat = gWorld->camera + dir;
    }
    else
    {
      if (moving)
        gWorld->camera.z -= dt * movespd * moving / (gWorld->zoom * 1.5f);
      if (strafing)
        gWorld->camera.x += dt * movespd * strafing / (gWorld->zoom * 1.5f);
      if (updown)
        gWorld->zoom *= pow(2.0f, dt * updown * 4.0f);

      gWorld->zoom = std::min(std::max(gWorld->zoom, 0.1f), 2.0f);
    }
  }
  else
  {
    leftMouse = false;
    rightMouse = false;
    look = false;
    MoveObj = false;

    moving = 0;
    strafing = 0;
    updown = 0;
  }

  if ((t - lastBrushUpdate) > 0.1f && textureBrush.needUpdate())
  {
    textureBrush.GenerateTexture();
  }

  gWorld->time += this->mTimespeed * dt;


  gWorld->animtime += dt * 1000.0f;
  globalTime = static_cast<int>(gWorld->animtime);

  gWorld->tick(dt);

  lastSelected = gWorld->GetCurrentSelection();

  if (!MapChunkWindow->hidden() && gWorld->GetCurrentSelection() && gWorld->GetCurrentSelection()->which() == eEntry_MapChunk)
  {
    UITexturingGUI::setChunkWindow(boost::get<selected_chunk_type> (*gWorld->GetCurrentSelection()).chunk);
  }
}

void MapView::doSelection (bool selectTerrainOnly)
{
  video.set3D();
  opengl::matrix::look_at (gWorld->camera, gWorld->lookat, {0.0f, 1.0f, 0.0f});

  GLint viewport[4];
  gl.getIntegerv(GL_VIEWPORT, viewport);

  float win_x (Environment::getInstance()->screenX);
  float win_y (viewport[3] - Environment::getInstance()->screenY);

  math::vector_4d const normalized_device_coords
    ( 2.0f * (win_x - static_cast<float> (viewport[0])) / static_cast<float> (viewport[2]) - 1.0f
    , 2.0f * (win_y - static_cast<float> (viewport[1])) / static_cast<float> (viewport[3]) - 1.0f
    , 0.0f
    , 1.0f
    );

  math::vector_3d const pos ( ( ( opengl::matrix::model_view()
                                * opengl::matrix::projection()
                                ).inverted().transposed()
                              * normalized_device_coords
                              ).xyz_normalized_by_w()
                            );

  math::ray ray (gWorld->camera, pos - gWorld->camera);

  selection_result results (gWorld->intersect (ray, selectTerrainOnly));

  std::sort ( results.begin()
            , results.end()
            , [] (selection_entry const& lhs, selection_entry const& rhs)
              {
                return lhs.first < rhs.first;
              }
            );

  gWorld->SetCurrentSelection
    ( results.empty()
    ? boost::optional<selection_type>()
    : boost::optional<selection_type> (results.front().second)
    );
}


void MapView::displayGUIIfEnabled()
{
  if (_GUIDisplayingEnabled)
  {
    video.set2D();

    gl.enable(GL_BLEND);
    gl.blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    opengl::texture::disable_texture(1);
    opengl::texture::enable_texture(0);

    gl.disable(GL_DEPTH_TEST);
    gl.disable(GL_CULL_FACE);
    gl.disable(GL_LIGHTING);
    gl.color4f(1.0f, 1.0f, 1.0f, 1.0f);

    opengl::texture::disable_texture(0);

    mainGui->setTilemode(mViewMode != eViewMode_3D);
    mainGui->render();

    opengl::texture::enable_texture(0);
  }
}

void MapView::displayViewMode_2D(float /*t*/, float /*dt*/)
{
  video.setTileMode();
  gWorld->drawTileMode(ah);

  const float mX = (CHUNKSIZE * 4.0f * video.ratio() * (static_cast<float>(MouseX) / static_cast<float>(video.xres()) - 0.5f) / gWorld->zoom + gWorld->camera.x) / CHUNKSIZE;
  const float mY = (CHUNKSIZE * 4.0f * (static_cast<float>(MouseY) / static_cast<float>(video.yres()) - 0.5f) / gWorld->zoom + gWorld->camera.z) / CHUNKSIZE;

  // draw brush
  {
    opengl::scoped::matrix_pusher const matrix;

    gl.scalef(gWorld->zoom, gWorld->zoom, 1.0f);
    gl.translatef(-gWorld->camera.x / CHUNKSIZE, -gWorld->camera.z / CHUNKSIZE, 0);

    gl.color4f(1.0f, 1.0f, 1.0f, 0.5f);
    opengl::texture::set_active_texture(1);
    opengl::texture::disable_texture();
    opengl::texture::set_active_texture(0);
    opengl::texture::enable_texture();

    textureBrush.getTexture()->bind();

    const float tRadius = textureBrush.getRadius() / CHUNKSIZE;// *gWorld->zoom;
    gl.begin(GL_QUADS);
    gl.texCoord2f(0.0f, 0.0f);
    gl.vertex3f(mX - tRadius, mY + tRadius, 0);
    gl.texCoord2f(1.0f, 0.0f);
    gl.vertex3f(mX + tRadius, mY + tRadius, 0);
    gl.texCoord2f(1.0f, 1.0f);
    gl.vertex3f(mX + tRadius, mY - tRadius, 0);
    gl.texCoord2f(0.0f, 1.0f);
    gl.vertex3f(mX - tRadius, mY - tRadius, 0);
    gl.end();
  }

  displayGUIIfEnabled();
}

void MapView::displayViewMode_3D(float /*t*/, float /*dt*/)
{
  //! \note Select terrain below mouse, if no item selected or the item is map.
  if (!gWorld->IsSelection(eEntry_Model) && !gWorld->IsSelection(eEntry_WMO) && Settings::getInstance()->AutoSelectingMode)
  {
    doSelection(true);
  }

  video.set3D();

  gWorld->draw();

  displayGUIIfEnabled();
}

void MapView::display(float t, float dt)
{
  //! \todo  Get this out or do it somehow else. This is ugly and is a senseless if each draw.
  if (Saving)
  {
    video.setTileMode();
    gWorld->saveMap();
    Saving = false;
  }

  switch (mViewMode)
  {
  case eViewMode_2D:
    displayViewMode_2D(t, dt);
    break;

  case eViewMode_3D:
    displayViewMode_3D(t, dt);
    break;
  }
}

void MapView::save()
{
  gWorld->mapIndex->saveChanged();
}

void MapView::quit()
{
  app.pop = true;
}

void MapView::quitask()
{
  mainGui->escWarning->show();
}

void MapView::resizewindow()
{
  mainGui->resize();
}

void MapView::keypressed(SDL_KeyboardEvent *e)
{
  if (LastClicked && LastClicked->KeyBoardEvent(e)) return;

  if (e->keysym.mod & KMOD_CAPS)
    mainGui->capsWarning->show();
  else
    mainGui->capsWarning->hide();

  if (e->type == SDL_KEYDOWN)
  {


    if (handleHotkeys(e))
      return;

    // key DOWN

    if (e->keysym.sym == SDLK_LSHIFT || e->keysym.sym == SDLK_RSHIFT)
      Environment::getInstance()->ShiftDown = true;

    if (e->keysym.sym == SDLK_LALT || e->keysym.sym == SDLK_RALT)
      Environment::getInstance()->AltDown = true;

    if (e->keysym.sym == SDLK_LCTRL || e->keysym.sym == SDLK_RCTRL)
      Environment::getInstance()->CtrlDown = true;

    if (e->keysym.sym == SDLK_SPACE)
      Environment::getInstance()->SpaceDown = true;

    // movement
    if (e->keysym.sym == SDLK_w)
    {
      key_w = true;
      moving = 1.0f;
    }

    if (e->keysym.sym == SDLK_UP)
    {
      lookat = 0.75f;
    }

    if (e->keysym.sym == SDLK_DOWN)
    {
      lookat = -0.75f;
    }

    if (e->keysym.sym == SDLK_LEFT)
    {
      turn = -0.75f;
    }

    if (e->keysym.sym == SDLK_RIGHT)
    {
      turn = 0.75f;
    }

    // save
    if (e->keysym.sym == SDLK_s)
      moving = -1.0f;

    if (e->keysym.sym == SDLK_a)
      strafing = -1.0f;

    if (e->keysym.sym == SDLK_d)
      strafing = 1.0f;

    if (e->keysym.sym == SDLK_e)
      updown = -1.0f;

    if (e->keysym.sym == SDLK_q)
      updown = 1.0f;

    // position correction with num pad
    if (e->keysym.sym == SDLK_KP8)
      keyx = -1;

    if (e->keysym.sym == SDLK_KP2)
      keyx = 1;

    if (e->keysym.sym == SDLK_KP6)
      keyz = -1;

    if (e->keysym.sym == SDLK_KP4)
      keyz = 1;

    if (e->keysym.sym == SDLK_KP1)
      keyy = -1;

    if (e->keysym.sym == SDLK_KP3)
      keyy = 1;

    if (e->keysym.sym == SDLK_KP7)
      keyr = 1;

    if (e->keysym.sym == SDLK_KP9)
      keyr = -1;

    if (e->keysym.sym == SDLK_KP0)
      if (terrainMode == 6)
      {
        gWorld->setWaterHeight(tile_index(gWorld->camera), 0.0f);
        mainGui->guiWater->updateData();
      }


    // delete object
    if (e->keysym.sym == SDLK_DELETE)
      DeleteSelectedObject(0, 0);

    // copy model to clipboard
    if (e->keysym.sym == SDLK_c)
    {
      if (Environment::getInstance()->CtrlDown && gWorld->GetCurrentSelection())
        mainGui->objectEditor->copy(*gWorld->GetCurrentSelection());
      else if (Environment::getInstance()->AltDown && Environment::getInstance()->CtrlDown)
        mainGui->toggleCursorSwitcher();
      else if (Environment::getInstance()->ShiftDown)
        InsertObject(0, 14);
      else if (Environment::getInstance()->AltDown)
        InsertObject(0, 15);
      else
      {
        if (terrainMode == 9)
        {
          mainGui->objectEditor->copy(*gWorld->GetCurrentSelection());
        }
        else
        {
          Environment::getInstance()->cursorType++;
          Environment::getInstance()->cursorType %= 4;
        }
      }
    }


    // paste model
    if (e->keysym.sym == SDLK_v)
    {
      if (Environment::getInstance()->ShiftDown)
      {
        InsertObject(0, 14);
      }
      else if (Environment::getInstance()->AltDown)
      {
        InsertObject(0, 15);
      }
      else if (terrainMode == 9 || Environment::getInstance()->CtrlDown)
      {
        mainGui->objectEditor->pasteObject();
      }
    }


    // with ctrl toggle detail window
    // without toggle the settings of the current edit mode.
    if (e->keysym.sym == SDLK_x)
    {
      if (Environment::getInstance()->CtrlDown)
      {
        // toggle detail window
        mainGui->guidetailInfos->toggleVisibility();
      }
      else
      {
        // toggle terrainMode window
        if (terrainMode == 2)
        {
          view_texture_palette(0, 0);
        }


      }
    }

    // invert mouse or swap paint modes
    if (e->keysym.sym == SDLK_i)
    {
      if (Environment::getInstance()->CtrlDown)
      {
        // temp till fixe draw texture.
        if (Environment::getInstance()->paintMode) Environment::getInstance()->paintMode = false;
        else Environment::getInstance()->paintMode = true;
      }
      else
      {
        mousedir *= -1.0f;
      }

    }
    // move speed doubling or raw saving
    if (e->keysym.sym == SDLK_p)
    {
      if (Environment::getInstance()->CtrlDown && Environment::getInstance()->ShiftDown)
      {
        Saving = true;
      }
      else
      {
        movespd *= 2.0f;
      }
    }

    if (e->keysym.sym == SDLK_o)
      movespd *= 0.5f;

    // turn around or reset object orientation
    if (e->keysym.sym == SDLK_r)
    {
      if (Environment::getInstance()->CtrlDown)
        ResetSelectedObjectRotation(0, 0);
      else ah += 180.0f;
    }

    if (e->keysym.sym == SDLK_t)
    {
      // toggle flatten angle mode
      if (terrainMode == 1)
      {
        if (Environment::getInstance()->SpaceDown)
        {
          gFlattenTypeGroup->Activate((flattenType + 1) % eFlattenMode_Count);
        }
        else
        {
          toggle_flatten->setState(!(Environment::getInstance()->flattenAngleEnabled));
          toggleFlattenAngle(!(Environment::getInstance()->flattenAngleEnabled), 0);
        }
      }
      else if (terrainMode == 2)
      {
        sprayBrushActive = !sprayBrushActive;
        toggleSpray->setState(sprayBrushActive);
      }
      else if (terrainMode == 3)
      {
        math::vector_3d cam = gWorld->camera;
        if (Environment::getInstance()->AltDown)
        {
          gWorld->addHoleADT(cam.x, cam.z);
        }
        else
        {
          gWorld->removeHoleADT(cam.x, cam.z);
        }
      }
      else if (terrainMode == 9)
      {
        mainGui->objectEditor->togglePasteMode();
      }


    }

    // clip object to ground
    if (e->keysym.sym == SDLK_PAGEDOWN)
      SnapSelectedObjectToGround(0, 0);

    // speed of daytime.
    if (e->keysym.sym == SDLK_n)
      this->mTimespeed += 90.0f;

    if (e->keysym.sym == SDLK_b)
      this->mTimespeed -= 90.0f;

    // no negativ time speed!
    if (this->mTimespeed < 0.0f)  this->mTimespeed = 0.0f;

    if (e->keysym.sym == SDLK_j)
      this->mTimespeed = 0.0f;



    // toggle lightning
    if (e->keysym.sym == SDLK_l)
      gWorld->lighting = !gWorld->lighting;

    // toggle interface
    if (e->keysym.sym == SDLK_TAB)
      _GUIDisplayingEnabled = !_GUIDisplayingEnabled;

    // toggle "terrain texturing mode" / draw models
    if (e->keysym.sym == SDLK_F1)
    {
      if (Environment::getInstance()->ShiftDown)
      {
        if (alloff)
        {
          alloff_models = gWorld->drawmodels;
          alloff_doodads = gWorld->drawdoodads;
          alloff_contour = DrawMapContour;
          alloff_wmo = gWorld->drawwmo;
          alloff_fog = gWorld->drawfog;
          alloff_terrain = gWorld->drawterrain;

          gWorld->drawmodels = false;
          gWorld->drawdoodads = false;
          DrawMapContour = true;
          gWorld->drawwmo = false;
          gWorld->drawterrain = true;
          gWorld->drawfog = false;
        }
        else
        {
          gWorld->drawmodels = alloff_models;
          gWorld->drawdoodads = alloff_doodads;
          DrawMapContour = alloff_contour;
          gWorld->drawwmo = alloff_wmo;
          gWorld->drawterrain = alloff_terrain;
          gWorld->drawfog = alloff_fog;
        }
        alloff = !alloff;
      }
      else
      {
        gWorld->drawmodels = !gWorld->drawmodels;
      }
    }


    // toggle drawing of doodads in WMOs.
    if (e->keysym.sym == SDLK_F2)
      gWorld->drawdoodads = !gWorld->drawdoodads;

    // toggle terrain
    if (e->keysym.sym == SDLK_F3)
      gWorld->drawterrain = !gWorld->drawterrain;

    // toggle better selection mode
    if (e->keysym.sym == SDLK_F4 && Environment::getInstance()->ShiftDown)
    {
      Settings::getInstance()->AutoSelectingMode = !Settings::getInstance()->AutoSelectingMode;
    }

    // toggle draw water
    if (e->keysym.sym == SDLK_F4 && !Environment::getInstance()->ShiftDown)
      gWorld->drawwater = !gWorld->drawwater;

    // toggle Animation
    if (e->keysym.sym == SDLK_F11)
    {
      gWorld->renderAnimations = !gWorld->renderAnimations;
    }

    // toggle chunk limitation lines
    if (e->keysym.sym == SDLK_F7)
    {
      if (Environment::getInstance()->ShiftDown)
      {
        Environment::getInstance()->view_holelines = !Environment::getInstance()->view_holelines;
      }
      else
      {
        gWorld->drawlines = !gWorld->drawlines;
      }
    }

    // toggle drawing of WMOs
    if (e->keysym.sym == SDLK_F6)
      gWorld->drawwmo = !gWorld->drawwmo;

    // toggle showing a lot of information about selected item
    if (e->keysym.sym == SDLK_F8)
    {
      mainGui->guidetailInfos->toggleVisibility();
    }

    // toggle height contours on terrain
    if (e->keysym.sym == SDLK_F9)
      DrawMapContour = !DrawMapContour;



    // toggle help window
    if (e->keysym.sym == SDLK_h)
    {
      if (terrainMode == 9)
      {
        // toggle hidden models visibility
        if (Environment::getInstance()->SpaceDown)
        {
          Environment::getInstance()->showModelFromHiddenList = !Environment::getInstance()->showModelFromHiddenList;
        }
        else if (Environment::getInstance()->ShiftDown)
        {
          gWorld->clearHiddenModelList();
        }
        else
        {
          // toggle selected model visibility
          if (gWorld->HasSelection())
          {
            auto selection = gWorld->GetCurrentSelection();
            if (selection->which() == eEntry_Model)
            {
              boost::get<selected_model_type> (*selection)->model->toggleVisibility();
            }
            else if (selection->which() == eEntry_WMO)
            {
              boost::get<selected_wmo_type> (*selection)->wmo->toggleVisibility();
            }
          }
        }

      }
      else
      {
        mainGui->toggleHelp();
      }
    }



    if (e->keysym.sym == SDLK_f)
    {
      if (terrainMode == 1)
      {
        if (Environment::getInstance()->SpaceDown)
        {
          toggleFlattenLock(!flattenRelativeMode, 0);
          toggle_flatten_relative->setState(flattenRelativeMode);
        }
        else
        {
          flattenRelativePos = Environment::getInstance()->get_cursor_pos();
          flatten_relative_x->value(misc::floatToStr(flattenRelativePos.x));
          flatten_relative_y->value(misc::floatToStr(flattenRelativePos.y));
          flatten_relative_z->value(misc::floatToStr(flattenRelativePos.z));
        }
      }
      else if (terrainMode == 9)
      {
        if (gWorld->HasSelection())
        {
          math::vector_3d pos = Environment::getInstance()->get_cursor_pos();
          auto selection = gWorld->GetCurrentSelection();

          if (selection->which() == eEntry_Model)
          {
            gWorld->updateTilesModel(boost::get<selected_model_type> (*selection));
            boost::get<selected_model_type> (*selection)->pos = pos;
            boost::get<selected_model_type> (*selection)->recalcExtents();
            gWorld->updateTilesModel(boost::get<selected_model_type> (*selection));
          }
          else if (selection->which() == eEntry_WMO)
          {
            gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*selection));
            boost::get<selected_wmo_type> (*selection)->pos = pos;
            boost::get<selected_wmo_type> (*selection)->recalcExtents();
            gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*selection));
          }
        }
      }
      else // draw fog
      {
        gWorld->drawfog = !gWorld->drawfog;
      }
    }


    // reload a map tile STEFF out because of UID recalc. reload could kill all.
    //if( e->keysym.sym == SDLK_j && Environment::getInstance()->ShiftDown )
    //  gWorld->reloadTile( static_cast<int>( gWorld->camera.x ) / TILESIZE, static_cast<int>( gWorld->camera.z ) / TILESIZE );

    // fog distance or brush radius
    if (e->keysym.sym == SDLK_KP_PLUS || e->keysym.sym == SDLK_PLUS)
    {
      if (Environment::getInstance()->AltDown)
      {
        switch (terrainMode)
        {
        case 0:
          groundBrushRadius = std::min(1000.0f, groundBrushRadius + 0.01f);
          ground_brush_radius->setValue(groundBrushRadius / 1000.0f);
          break;
        case 1:
          blurBrushRadius = std::min(1000.0f, blurBrushRadius + 0.01f);
          blur_brush->setValue(blurBrushRadius / 1000.0f);
          break;
        case 2:
          textureBrush.setRadius(std::min(100.0f, textureBrush.getRadius() + 0.1f));
          paint_brush->setValue(textureBrush.getRadius() / 100.0f);
          break;
        case 6:
          tile_index tile(gWorld->camera);
          gWorld->setWaterHeight(tile, std::ceil(gWorld->getWaterHeight(tile) + 2.0f));
          mainGui->guiWater->updateData();
          break;
        }
      }
      else if ((!gWorld->HasSelection() || (gWorld->HasSelection() && gWorld->GetCurrentSelection()->which() == eEntry_MapChunk)))
      {
        if (terrainMode == 6)
        {
          tile_index tile(gWorld->camera);

          if (Environment::getInstance()->ShiftDown)
          {
            gWorld->setWaterTrans(tile, static_cast<unsigned char>(std::ceil(static_cast<float>(gWorld->getWaterTrans(tile)) + 1)));
          }
          else if (Environment::getInstance()->CtrlDown)
          {
            gWorld->setWaterHeight(tile, std::ceil(gWorld->getWaterHeight(tile) + 5.0f));
          }
          else
          {
            gWorld->setWaterHeight(tile, std::ceil(gWorld->getWaterHeight(tile) + 1.0f));
          }

          mainGui->guiWater->updateData();
        }
        else if (Environment::getInstance()->ShiftDown)
        {
          gWorld->fogdistance += 60.0f;// fog change only when no model is selected!
        }
      }
      else
      {
        //change selected model size
        keys = 1;
      }
    }

    if (e->keysym.sym == SDLK_KP_MINUS || e->keysym.sym == SDLK_MINUS)
    {
      if (Environment::getInstance()->AltDown)
      {
        switch (terrainMode)
        {
        case 0:
          groundBrushRadius = std::max(0.0f, groundBrushRadius - 0.01f);
          ground_brush_radius->setValue(groundBrushRadius / 1000.0f);
          break;
        case 1:
          blurBrushRadius = std::max(0.0f, blurBrushRadius - 0.01f);
          blur_brush->setValue(blurBrushRadius / 1000);
          break;
        case 2:
          textureBrush.setRadius(std::max(0.0f, textureBrush.getRadius() - 0.1f));
          paint_brush->setValue(textureBrush.getRadius() / 100);
          break;
        case 6:
          tile_index tile(gWorld->camera);
          gWorld->setWaterHeight(tile, std::ceil(gWorld->getWaterHeight(tile) - 2.0f));
          mainGui->guiWater->updateData();
          break;
        }
      }
      else if ((!gWorld->HasSelection() || (gWorld->HasSelection() && gWorld->GetCurrentSelection()->which() == eEntry_MapChunk)))
      {
        if (terrainMode == 6)
        {
          tile_index tile(gWorld->camera);
          if (Environment::getInstance()->ShiftDown)
          {
            gWorld->setWaterTrans(tile, static_cast<unsigned char>(std::floor(static_cast<float>(gWorld->getWaterTrans(tile))) - 1));
          }
          else if (Environment::getInstance()->CtrlDown)
          {
            gWorld->setWaterHeight(tile, std::ceil(gWorld->getWaterHeight(tile) - 5.0f));
          }
          else
          {
            gWorld->setWaterHeight(tile, std::ceil(gWorld->getWaterHeight(tile) - 1.0f));
          }

          mainGui->guiWater->updateData();
        }
        else if (Environment::getInstance()->ShiftDown)
        {
          gWorld->fogdistance -= 60.0f;// fog change only when no model is selected!
        }

      }
      else
      {
        //change selected model size
        keys = -1;
      }
    }

    // minimap
    if (e->keysym.sym == SDLK_m)
      mainGui->minimapWindow->toggleVisibility();

    if (e->keysym.sym == SDLK_g)
    {
      // write teleport cords to txt file
      std::ofstream f("ports.txt", std::ios_base::app);
      f << "Map: " << gAreaDB.getAreaName(gWorld->getAreaID()) << " on ADT " << std::floor(gWorld->camera.x / TILESIZE) << " " << std::floor(gWorld->camera.z / TILESIZE) << std::endl;
      f << "Trinity:" << std::endl << ".go " << (ZEROPOINT - gWorld->camera.z) << " " << (ZEROPOINT - gWorld->camera.x) << " " << gWorld->camera.y << " " << gWorld->getMapID() << std::endl;
      f << "ArcEmu:" << std::endl << ".worldport " << gWorld->getMapID() << " " << (ZEROPOINT - gWorld->camera.z) << " " << (ZEROPOINT - gWorld->camera.x) << " " << gWorld->camera.y << " " << std::endl << std::endl;
      f.close();
    }


    // toogle between smooth / flat / linear
    if (e->keysym.sym == SDLK_y)
    {
      switch (terrainMode)
      {
      case 0:
        gGroundToggleGroup->Activate((Environment::getInstance()->groundBrushType + 1) % 6);
        break;

      case 1:
        gBlurToggleGroup->Activate((blurBrushType + 1) % 3);
        break;
      }
    }

    // is not used somewere else!!
    //! \todo  what is this?
    if (e->keysym.sym == SDLK_g)
      drawFlags = !drawFlags;

    // toogle tile mode
    if (e->keysym.sym == SDLK_u)
    {
      if (mViewMode == eViewMode_2D)
      {
        mViewMode = eViewMode_3D;
        terrainMode = saveterrainMode;
        // Set the right icon in toolbar
        mainGui->guiToolbar->IconSelect(terrainMode);
      }
      else
      {
        mViewMode = eViewMode_2D;
        saveterrainMode = terrainMode;
        terrainMode = 2;
        // Set the right icon in toolbar
        mainGui->guiToolbar->IconSelect(terrainMode);
      }
    }

    // doodads set
    if (e->keysym.sym >= SDLK_0 && e->keysym.sym <= SDLK_9)
    {
      if (gWorld->IsSelection(eEntry_WMO))
      {
        boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = e->keysym.sym - SDLK_0;
      }
      else if (Environment::getInstance()->ShiftDown)
      {
        if (e->keysym.sym == SDLK_1)
          movespd = 15.0f;
        if (e->keysym.sym == SDLK_2)
          movespd = 50.0f;
        if (e->keysym.sym == SDLK_3)
          movespd = 200.0f;
        if (e->keysym.sym == SDLK_4)
          movespd = 800.0f;
      }
      else if (Environment::getInstance()->AltDown)
      {
        if (e->keysym.sym == SDLK_1)
          mainGui->G1->setValue(0.01f);
        if (e->keysym.sym == SDLK_2)
          mainGui->G1->setValue(0.25f);
        if (e->keysym.sym == SDLK_3)
          mainGui->G1->setValue(0.50f);
        if (e->keysym.sym == SDLK_4)
          mainGui->G1->setValue(0.75f);
        if (e->keysym.sym == SDLK_5)
          mainGui->G1->setValue(0.99f);
      }
      else if (e->keysym.sym >= SDLK_1 && e->keysym.sym <= SDLK_9)
      {
        terrainMode = e->keysym.sym - SDLK_1;
        mainGui->guiToolbar->IconSelect(terrainMode);
      }
      else if (e->keysym.sym == SDLK_0)
      {
        terrainMode = 9; // object editor
        mainGui->guiToolbar->IconSelect(terrainMode);
      }
    }

    // add a new bookmark
    if (e->keysym.sym == SDLK_F5)
    {
      std::ofstream f("bookmarks.txt", std::ios_base::app);
      f << gWorld->getMapID() << " " << gWorld->camera.x << " " << gWorld->camera.y << " " << gWorld->camera.z << " " << ah << " " << av << " " << gWorld->getAreaID() << std::endl;
      f.close();
    }
  }
  else
  {
    // key UP
    if (e->keysym.sym == SDLK_LSHIFT || e->keysym.sym == SDLK_RSHIFT)
      Environment::getInstance()->ShiftDown = false;

    if (e->keysym.sym == SDLK_LALT || e->keysym.sym == SDLK_RALT)
      Environment::getInstance()->AltDown = false;

    if (e->keysym.sym == SDLK_LCTRL || e->keysym.sym == SDLK_RCTRL)
      Environment::getInstance()->CtrlDown = false;

    if (e->keysym.sym == SDLK_SPACE)
      Environment::getInstance()->SpaceDown = false;

    // movement
    if (e->keysym.sym == SDLK_w)
    {
      key_w = false;
      if (!(leftMouse && rightMouse) && moving > 0.0f) {

        moving = 0.0f;
      }
    }

    if (e->keysym.sym == SDLK_s && moving < 0.0f) {

      moving = 0.0f;
    }

    if (e->keysym.sym == SDLK_UP || e->keysym.sym == SDLK_DOWN)
      lookat = 0.0f;

    if (e->keysym.sym == SDLK_LEFT || e->keysym.sym == SDLK_RIGHT)
      turn = 0.0f;

    if (e->keysym.sym == SDLK_d && strafing > 0.0f) {

      strafing = 0.0f;
    }

    if (e->keysym.sym == SDLK_a && strafing < 0.0f) {

      strafing = 0.0f;
    }

    if (e->keysym.sym == SDLK_q && updown > 0.0f)
      updown = 0.0f;

    if (e->keysym.sym == SDLK_e && updown < 0.0f)
      updown = 0.0f;

    if (e->keysym.sym == SDLK_KP8)
      keyx = 0;

    if (e->keysym.sym == SDLK_KP2)
      keyx = 0;

    if (e->keysym.sym == SDLK_KP6)
      keyz = 0;

    if (e->keysym.sym == SDLK_KP4)
      keyz = 0;

    if (e->keysym.sym == SDLK_KP1)
      keyy = 0;

    if (e->keysym.sym == SDLK_KP3)
      keyy = 0;

    if (e->keysym.sym == SDLK_KP7)
      keyr = 0;

    if (e->keysym.sym == SDLK_KP9)
      keyr = 0;

    if (e->keysym.sym == SDLK_KP_MINUS || e->keysym.sym == SDLK_MINUS || e->keysym.sym == SDLK_KP_PLUS || e->keysym.sym == SDLK_PLUS)
      keys = 0;
  }
}

void MapView::inserObjectFromExtern(int model)
{
  InsertObject(0, model);
}


void MapView::mousemove(SDL_MouseMotionEvent *e)
{
  mainGui->minimapWindow->mousemove(e);
  if ((look && !(Environment::getInstance()->ShiftDown || Environment::getInstance()->CtrlDown || Environment::getInstance()->AltDown)) || video.fullscreen())
  {
    ah += e->xrel / XSENS;
    av += mousedir * e->yrel / YSENS;
    if (av < -80.0f)
      av = -80.0f;
    else if (av > 80.0f)
      av = 80.0f;

    mainGui->minimapWindow->changePlayerLookAt(math::degrees (ah));
  }

  if (MoveObj)
  {
    mh = -video.ratio()*e->xrel / static_cast<float>(video.xres());
    mv = -e->yrel / static_cast<float>(video.yres());
  }
  else
  {
    mh = 0.0f;
    mv = 0.0f;
  }

  if (Environment::getInstance()->ShiftDown || Environment::getInstance()->CtrlDown || Environment::getInstance()->AltDown)
  {
    rh = e->xrel / XSENS * 5.0f;
    rv = e->yrel / YSENS * 5.0f;
  }

  if (rightMouse && Environment::getInstance()->AltDown)
  {
    if (terrainMode == 2)
    {
      float hardness = std::max(0.0f, std::min(1.0f, textureBrush.getHardness() + e->xrel / 300.0f));
      setTextureBrushHardness(hardness);
      mainGui->paintHardnessSlider->setValue(hardness);
    }
  }

  if (leftMouse && Environment::getInstance()->AltDown)
  {
    switch (terrainMode)
    {
    case 0:
      groundBrushRadius = std::max(0.0f, std::min(1000.0f, groundBrushRadius + e->xrel / XSENS));
      ground_brush_radius->setValue(groundBrushRadius / 1000.0f);
      break;
    case 1:
      blurBrushRadius = std::max(0.0f, std::min(1000.0f, blurBrushRadius + e->xrel / XSENS));
      blur_brush->setValue(blurBrushRadius / 1000.0f);
      break;
    case 2:
      textureBrush.setRadius(std::max(0.0f, std::min(100.0f, textureBrush.getRadius() + e->xrel / XSENS)));
      paint_brush->setValue(textureBrush.getRadius() / 100.0f);
      break;
    case 8:
      shaderRadius = std::max(0.0f, std::min(1000.0f, shaderRadius + e->xrel / XSENS));
      shader_radius->setValue(shaderRadius / 1000.0f);
      break;
    }
  }

  if (leftMouse && Environment::getInstance()->SpaceDown)
  {
    switch (terrainMode)
    {
    case 0:
      groundBrushSpeed = std::max(0.0f, std::min(10.0f, groundBrushSpeed + e->xrel / 30.0f));
      ground_brush_speed->setValue(groundBrushSpeed / 10.0f);
      break;
    case 1:
      groundBlurSpeed = std::max(0.0f, std::min(10.0f, groundBlurSpeed + e->xrel / 30.0f));
      ground_blur_speed->setValue(groundBlurSpeed / 10.0f);
      break;
    case 2:
      mainGui->paintPressureSlider->setValue(std::max(0.0f, std::min(1.0f, mainGui->paintPressureSlider->value + e->xrel / 300.0f)));
      break;
    case 8:
      shaderSpeed = std::max(0.0f, std::min(10.0f, shaderSpeed + e->xrel / 30.0f));
      shader_speed->setValue(shaderSpeed / 10.0f);
      break;
    }
  }

  if (leftMouse && LastClicked)
  {
    LastClicked->processLeftDrag((float)(e->x - 4), (float)(e->y - 4), (float)(e->xrel), (float)(e->yrel));
  }

  if (mViewMode == eViewMode_2D && leftMouse && Environment::getInstance()->AltDown && Environment::getInstance()->ShiftDown)
  {
    strafing = ((e->xrel / XSENS) / -1) * 5.0f;
    moving = (e->yrel / YSENS) * 5.0f;
  }

  if (mViewMode == eViewMode_2D && rightMouse && Environment::getInstance()->ShiftDown)
  {
    updown = (e->yrel / YSENS);
  }



  Environment::getInstance()->screenX = MouseX = e->x;
  Environment::getInstance()->screenY = MouseY = e->y;
  checkWaterSave();
}

void MapView::addModelFromTextSelection(int id)
{
  InsertObject(0, id);
}

void MapView::selectModel(selection_type entry)
{
  mainGui->objectEditor->copy(entry);
}

void MapView::mouseclick(SDL_MouseButtonEvent *e)
{
  if (e->type == SDL_MOUSEBUTTONDOWN)
  {
    switch (e->button)
    {
    case SDL_BUTTON_LEFT:
      leftMouse = true;
      break;

    case SDL_BUTTON_RIGHT:
      rightMouse = true;
      break;

    case SDL_BUTTON_MIDDLE:
      if (gWorld->HasSelection())
      {
        MoveObj = true;
        auto selection = gWorld->GetCurrentSelection();
        math::vector_3d objPos;
        if (selection->which() == eEntry_WMO)
        {
          objPos = boost::get<selected_wmo_type> (*selection)->pos;
        }
        else if (selection->which() == eEntry_Model)
        {
          objPos = boost::get<selected_model_type> (*selection)->pos;
        }

        objMoveOffset = Environment::getInstance()->get_cursor_pos() - objPos;
      }

      break;

    case SDL_BUTTON_WHEELUP:
      if (terrainMode == 1)
      {
        if (Environment::getInstance()->AltDown)
        {
          flattenOrientation += Environment::getInstance()->CtrlDown ? 1.0f : 10.0f;
          if (flattenOrientation > 360.0f)
            flattenOrientation = 0.0f;
          flatten_orientation->setValue(flattenOrientation / 360);
        }
        else if (Environment::getInstance()->ShiftDown)
        {
          flattenAngle += Environment::getInstance()->CtrlDown ? 0.2f : 2.0f;
          if (flattenAngle > 89.0f)
            flattenAngle = 89.0f;
          flatten_angle->setValue(flattenAngle / 90);
        }
        else if (Environment::getInstance()->SpaceDown)
        {
          flattenRelativePos.y += 1;
          flatten_relative_y->value(misc::floatToStr(flattenRelativePos.y));
        }
      }
      else if (terrainMode == 2)
      {
        if (Environment::getInstance()->SpaceDown)
        {
          brushLevel = std::min(255.0f, brushLevel + 10.0f);
          mainGui->G1->setValue((255.0f - brushLevel) / 255.0f);
        }
        else if (Environment::getInstance()->AltDown)
        {
          brushSpraySize = std::min(40.0f, brushSpraySize + 2.0f);
          spray_size->setValue(brushSpraySize / 40.0f);
        }
        else if (Environment::getInstance()->ShiftDown)
        {
          brushSprayPressure = std::min(100.0f, brushSprayPressure + 2.5f);
          spray_pressure->setValue(brushSprayPressure / 100.0f);
        }
      }
      break;
    case SDL_BUTTON_WHEELDOWN:
      if (terrainMode == 1)
      {
        if (Environment::getInstance()->AltDown)
        {
          flattenOrientation -= Environment::getInstance()->CtrlDown ? 1.0f : 10.0f;
          if (flattenOrientation < 0.0f)
            flattenOrientation = 360.0f;
          flatten_orientation->setValue(flattenOrientation / 360);
        }
        else if (Environment::getInstance()->ShiftDown)
        {
          flattenAngle -= Environment::getInstance()->CtrlDown ? 0.2f : 2.0f;;
          if (flattenAngle < 0.0f)
            flattenAngle = 0.0f;
          flatten_angle->setValue(flattenAngle / 90);
        }
        else if (Environment::getInstance()->SpaceDown)
        {
          flattenRelativePos.y -= 1;
          flatten_relative_y->value(misc::floatToStr(flattenRelativePos.y));
        }
      }
      else if (terrainMode == 2)
      {
        if (Environment::getInstance()->SpaceDown)
        {
          brushLevel = std::max(0.0f, brushLevel - 10.0f);
          mainGui->G1->setValue((255.0f - brushLevel) / 255.0f);
        }
        else if (Environment::getInstance()->AltDown)
        {
          brushSpraySize = std::max(1.0f, brushSpraySize - 2.0f);
          spray_size->setValue(brushSpraySize / 40.0f);
        }
        else if (Environment::getInstance()->ShiftDown)
        {
          brushSprayPressure = std::max(0.0f, brushSprayPressure - 2.5f);
          spray_pressure->setValue(brushSprayPressure / 100.0f);
        }
      }
      break;
    }

    if (leftMouse && rightMouse)
    {
      // Both buttons
      moving = 1.0f;
    }
    else if (leftMouse)
    {
      LastClicked = mainGui->processLeftClick(static_cast<float>(MouseX), static_cast<float>(MouseY));
      if (mViewMode == eViewMode_3D && !LastClicked)
      {
        doSelection(false);
      }
    }
    else if (rightMouse)
    {
      look = true;
    }
  }
  else if (e->type == SDL_MOUSEBUTTONUP)
  {
    switch (e->button)
    {
    case SDL_BUTTON_LEFT:
      leftMouse = false;

      if (LastClicked)
        LastClicked->processUnclick();

      if (!key_w && moving > 0.0f)
        moving = 0.0f;

      if (mViewMode == eViewMode_2D)
      {
        strafing = 0;
        moving = 0;
      }
      break;

    case SDL_BUTTON_RIGHT:
      rightMouse = false;

      look = false;

      if (!key_w && moving > 0.0f)
        moving = 0.0f;

      if (mViewMode == eViewMode_2D)
        updown = 0;

      break;

    case SDL_BUTTON_MIDDLE:
      MoveObj = false;
      break;
    }
  }

  // check menu settings and switch hole mode
  //! \todo why the hell is this here?
  if (terrainMode != 3)
  {
    Environment::getInstance()->view_holelines = Settings::getInstance()->holelinesOn;
  }
}

void MapView::checkWaterSave()
{
  if (gWorld->canWaterSave(tile_index(gWorld->camera)))
  {
    mainGui->waterSaveWarning->hide();
  }
  else
  {
    mainGui->waterSaveWarning->show();
  }
}
