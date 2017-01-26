// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#undef _UNICODE

#include <noggit/Brush.h> // brush
#include <noggit/ConfigFile.h>
#include <noggit/DBC.h>
#include <noggit/Environment.h>
#include <noggit/Environment.h>
#include <noggit/FreeType.h> // freetype::
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/MapView.h>
#include <noggit/Misc.h>
#include <noggit/ModelManager.h> // ModelManager
#include <noggit/Project.h>
#include <noggit/Settings.h>
#include <noggit/TextureManager.h> // TextureManager, Texture
#include <noggit/WMOInstance.h> // WMOInstance
#include <noggit/World.h>
#include <noggit/application.h> // app.getStates(), gPop, gFPS, app.getArial14(), morpheus40, arial...
#include <noggit/map_index.hpp>
#include <noggit/ui/AppInfo.h> // appInfo
#include <noggit/ui/CapsWarning.h>
#include <noggit/ui/CheckBox.h> // UICheckBox
#include <noggit/ui/CursorSwitcher.h> // UICursorSwitcher
#include <noggit/ui/DetailInfos.h> // detailInfos
#include <noggit/ui/ExitWarning.h>
#include <noggit/ui/FlattenTool.hpp>
#include <noggit/ui/Gradient.h> // UIGradient
#include <noggit/ui/HelperModels.h>
#include <noggit/ui/MapViewGUI.h> // UIMapViewGUI
#include <noggit/ui/MenuBar.h> // UIMenuBar, menu items, ..
#include <noggit/ui/MinimapWindow.h> // UIMinimapWindow
#include <noggit/ui/ModelImport.h>
#include <noggit/ui/ObjectEditor.h>
#include <noggit/ui/RotationEditor.h>
#include <noggit/ui/shader_tool.hpp>
#include <noggit/ui/Slider.h> // UISlider
#include <noggit/ui/StatusBar.h> // statusBar
#include <noggit/ui/terrain_tool.hpp>
#include <noggit/ui/Text.h> // UIText
#include <noggit/ui/Texture.h> // textureUI
#include <noggit/ui/TexturePicker.h>
#include <noggit/ui/TextureSwitcher.h>
#include <noggit/ui/TexturingGUI.h>
#include <noggit/ui/ToggleGroup.h> // UIToggleGroup
#include <noggit/ui/Toolbar.h> // UIToolbar
#include <noggit/ui/ToolbarIcon.h> // ToolbarIcon
#include <noggit/ui/Water.h>
#include <noggit/ui/WaterSaveWarning.h>
#include <noggit/ui/WaterTypeBrowser.h>
#include <noggit/ui/ZoneIDBrowser.h>
#include <opengl/matrix.hpp>
#include <opengl/scoped.hpp>

#include <boost/filesystem.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

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

UIToggleGroup * gFlagsToggleGroup;

UIWindow *settings_paint;


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



void change_settings_window(int oldid, int newid)
{
  if (oldid + 1 == newid || !mainGui || !mainGui->terrainTool || !mainGui->flattenTool || !settings_paint
    || !mainGui->shaderTool || !mainGui->guiWater || !mainGui->objectEditor)
  {
    return;
  }

  mainGui->guiWaterTypeSelector->hide();
  mainGui->terrainTool->hide();
  mainGui->flattenTool->hide();
  settings_paint->hide();
  mainGui->shaderTool->hide();
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
    tool_settings_x = (int)mainGui->terrainTool->x();
    tool_settings_y = (int)mainGui->terrainTool->y();
    break;
  case 2:
    tool_settings_x = (int)mainGui->flattenTool->x();
    tool_settings_y = (int)mainGui->flattenTool->y();
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
    tool_settings_x = (int)mainGui->shaderTool->x();
    tool_settings_y = (int)mainGui->shaderTool->y();
    break;
  }
  // set new win pos and make visible
  switch (newid)
  {
  case 1:
    mainGui->terrainTool->x((const float)tool_settings_x);
    mainGui->terrainTool->y((const float)tool_settings_y);
    mainGui->terrainTool->show();
    break;
  case 2:
    mainGui->flattenTool->x((const float)tool_settings_x);
    mainGui->flattenTool->y((const float)tool_settings_y);
    mainGui->flattenTool->show();
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
    mainGui->shaderTool->x((const float)tool_settings_x);
    mainGui->shaderTool->y((const float)tool_settings_y);
    mainGui->shaderTool->show();
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

void openURL(int target)
{
#if defined(_WIN32) || defined(WIN32)
  if (target == 1)  ShellExecute(nullptr, "open", "http://modcraft.superparanoid.de", nullptr, nullptr, SW_SHOWNORMAL);
  if (target == 2)  ShellExecute(nullptr, "open", "http://modcraft.superparanoid.de/wiki/index.php5?title=Noggit_user_manual", nullptr, nullptr, SW_SHOWNORMAL);
#endif
}

void ResetSelectedObjectRotation()
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

void SnapSelectedObjectToGround()
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


void DeleteSelectedObject()
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

/*!
\brief Import a new model form a text file or a hard coded one.
Imports a model from the import.txt (or the ImportFile from the config), the wowModelViewer log or just insert some hard coded testing models.
\param id the id switch the import kind
*/
void InsertObject(int id)
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
  mainGui->guiCurrentTexture->current_texture->setClickFunc ([] { mainGui->TexturePalette->toggleVisibility(); });

  mainGui->ZoneIDBrowser->setMapID(gWorld->getMapID());
  mainGui->ZoneIDBrowser->setChangeFunc(changeZoneIDValue);
  tool_settings_x = video.xres() - 186;
  tool_settings_y = 38; 

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
  mainGui->G1->setClickFunc ([] (float f) { brushLevel = (1.0f - f)*255.0f; });
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
  B1 = new UIButton(6.0f, 230.0f, 170.0f, 30.0f, "Texture swapper", "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp", "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp", []
                   {
                     mainGui->TextureSwitcher->show();
                     settings_paint->hide();
                   });
  settings_paint->addChild(B1);

  UIButton* rmDup = new UIButton( 6.0f, 255.0f, 170.0f, 30.0f
                                , "Remove texture duplicates"
                                , "Interface\\BUTTONS\\UI-DialogBox-Button-Disabled.blp"
                                , "Interface\\BUTTONS\\UI-DialogBox-Button-Down.blp"
                                , [] { gWorld->removeTexDuplicateOnADT(gWorld->camera.x, gWorld->camera.z); }
                                );
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

  // mbar->GetMenu( "File" )->AddMenuItemButton( "CTRL+SHIFT+S Save current", [] { gWorld->mapIndex->saveTile(tile_index(gWorld->camera)); });
  mbar->GetMenu("File")->AddMenuItemButton("CTRL+S Save", [] { gWorld->mapIndex->saveChanged(); });
  addHotkey (SDLK_s, MOD_ctrl, [this] { save(); });
  addHotkey (SDLK_s, MOD_meta, [this] { save(); });
  // mbar->GetMenu( "File" )->AddMenuItemButton( "SHIFT+J Reload tile", [] { gWorld->mapIndex->reloadTile(tile_index(gWorld->camera)); });
  //  mbar->GetMenu( "File" )->AddMenuItemSeperator( "Import and Export" );
  // mbar->GetMenu( "File" )->AddMenuItemButton( "Export heightmap", exportPNG, 1 );
  // mbar->GetMenu( "File" )->AddMenuItemButton( "Import heightmap", importPNG, 1 );
  mbar->GetMenu("File")->AddMenuItemSeperator(" ");
  mbar->GetMenu("File")->AddMenuItemButton("ESC Exit", [] { mainGui->escWarning->show(); });
  addHotkey (SDLK_ESCAPE, MOD_none, [this] { quitask(); });

  mbar->GetMenu("Edit")->AddMenuItemSeperator("selected object");
  mbar->GetMenu("Edit")->AddMenuItemButton("DEL delete", DeleteSelectedObject);
  mbar->GetMenu("Edit")->AddMenuItemButton("CTRL + R reset rotation", ResetSelectedObjectRotation);
  mbar->GetMenu("Edit")->AddMenuItemButton("PAGE DOWN set to ground", SnapSelectedObjectToGround);

  mbar->GetMenu("Edit")->AddMenuItemSeperator("Options");
  mbar->GetMenu("Edit")->AddMenuItemToggle("Auto select mode", &Settings::getInstance()->AutoSelectingMode, false);


  mbar->GetMenu("Assist")->AddMenuItemSeperator("Model");
  mbar->GetMenu("Assist")->AddMenuItemButton("Last M2 from MV", [] { InsertObject (14); });
  mbar->GetMenu("Assist")->AddMenuItemButton("Last WMO from MV", [] { InsertObject (15); });
  mbar->GetMenu("Assist")->AddMenuItemButton("Helper models", [] { mainGui->HelperModels->show(); });
  mbar->GetMenu("Assist")->AddMenuItemSeperator("ADT");
  mbar->GetMenu("Assist")->AddMenuItemButton ( "Set Area ID"
                                             , []
                                               {
                                                 if (Environment::getInstance()->selectedAreaID)
                                                 {
                                                   gWorld->setAreaID(gWorld->camera.x, gWorld->camera.z, Environment::getInstance()->selectedAreaID, true);
                                                 }
                                               }
                                             );
  mbar->GetMenu("Assist")->AddMenuItemButton ( "Clear height map"
                                             , []
                                               {
                                                 if (Environment::getInstance()->selectedAreaID)
                                                 {
                                                   gWorld->clearHeight(gWorld->camera.x, gWorld->camera.z);
                                                 }
                                               }
                                             );

  mbar->GetMenu("Assist")->AddMenuItemButton ( "Clear texture"
                                             , [] { gWorld->setBaseTexture(gWorld->camera.x, gWorld->camera.z); }
                                             );
  mbar->GetMenu("Assist")->AddMenuItemButton ( "Clear models"
                                             , [] { gWorld->clearAllModelsOnADT(tile_index(gWorld->camera)); }
                                             );
  mbar->GetMenu("Assist")->AddMenuItemButton ( "Clear duplicate models"
                                             , [] { gWorld->delete_duplicate_model_and_wmo_instances(); }
                                             );
  mbar->GetMenu("Assist")->AddMenuItemButton ( "Clear water"
                                             , [] { gWorld->deleteWaterLayer(tile_index(gWorld->camera)); }
                                             );
  mbar->GetMenu("Assist")->AddMenuItemButton ( "Create water"
                                             , [] { gWorld->addWaterLayer(tile_index(gWorld->camera)); }
                                             );
  mbar->GetMenu("Assist")->AddMenuItemButton("Fix gaps (all loaded adts)", [] { gWorld->fixAllGaps(); });
  mbar->GetMenu("Assist")->AddMenuItemButton("Clear standard shader", [] { gWorld->ClearShader(tile_index(gWorld->camera)); });
  mbar->GetMenu("Assist")->AddMenuItemButton("Map to big alpha", [] { gWorld->convertMapToBigAlpha(); });

  mbar->GetMenu("View")->AddMenuItemSeperator("Windows");
  mbar->GetMenu("View")->AddMenuItemToggle("Toolbar", mainGui->guiToolbar->hidden_evil(), true);

  mbar->GetMenu("View")->AddMenuItemToggle("Texture palette", mainGui->TexturePalette->hidden_evil(), true);
  mbar->GetMenu("View")->AddMenuItemButton("Cursor options", [] { mainGui->showCursorSwitcher(); });
  mbar->GetMenu("View")->AddMenuItemSeperator("Toggle");
  mbar->GetMenu("View")->AddMenuItemToggle("F1 M2s", &gWorld->drawmodels);
  addHotkey (SDLK_F1, MOD_none, [] { gWorld->drawmodels = !gWorld->drawmodels; });
  mbar->GetMenu("View")->AddMenuItemToggle("F2 WMO doodadsets", &gWorld->drawdoodads);
  addHotkey (SDLK_F2, MOD_none, [] { gWorld->drawdoodads = !gWorld->drawdoodads; });
  mbar->GetMenu("View")->AddMenuItemToggle("F3 Terrain", &gWorld->drawterrain);
  addHotkey (SDLK_F3, MOD_none, [] { gWorld->drawterrain = !gWorld->drawterrain; });
  mbar->GetMenu("View")->AddMenuItemToggle("F4 Water", &gWorld->drawwater);
  addHotkey (SDLK_F4, MOD_none, [] { gWorld->drawwater = !gWorld->drawwater; });
  mbar->GetMenu("View")->AddMenuItemToggle("F6 WMOs", &gWorld->drawwmo);
  addHotkey (SDLK_F6, MOD_none, [] { gWorld->drawwmo = !gWorld->drawwmo; });
  mbar->GetMenu("View")->AddMenuItemToggle("F7 Lines", &gWorld->drawlines);
  addHotkey (SDLK_F7, MOD_none, [] { gWorld->drawlines = !gWorld->drawlines; });
  mbar->GetMenu("View")->AddMenuItemToggle("F8 Detail infos", mainGui->guidetailInfos->hidden_evil(), true);
  addHotkey (SDLK_F8, MOD_none, [this] { mainGui->guidetailInfos->toggleVisibility(); });
  mbar->GetMenu("View")->AddMenuItemToggle("F9 Map contour infos", &DrawMapContour);
  addHotkey (SDLK_F9, MOD_none, [] { DrawMapContour = !DrawMapContour; });
  mbar->GetMenu("View")->AddMenuItemToggle("F11 Toggle Animation", &gWorld->renderAnimations);
  addHotkey (SDLK_F11, MOD_none, [] { gWorld->renderAnimations = !gWorld->renderAnimations; });
  mbar->GetMenu("View")->AddMenuItemToggle("Flight Bounds", &gWorld->draw_mfbo);
  mbar->GetMenu("View")->AddMenuItemToggle("F Fog", &gWorld->drawfog);
  mbar->GetMenu("View")->AddMenuItemToggle("Hole lines always on", &Environment::getInstance()->view_holelines, false);
  mbar->GetMenu("View")->AddMenuItemToggle("Wireframe", &gWorld->drawwireframe);
  mbar->GetMenu("View")->AddMenuItemToggle("Models with box", &Settings::getInstance()->renderModelsWithBox);

  mbar->GetMenu("Help")->AddMenuItemButton("Key Bindings F10", [] { mainGui->showHelp(); });
  mbar->GetMenu("Help")->AddMenuItemButton("Manual online", [] { openURL (2); });
  mbar->GetMenu("Help")->AddMenuItemButton("Homepage", [] { openURL (1); });

  mainGui->addChild(mbar);

  addHotkey (SDLK_m, MOD_none, [this] { mainGui->minimapWindow->toggleVisibility(); });

  addHotkey ( SDLK_F1
            , MOD_shift
            , []
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
            );

  addHotkey ( SDLK_F5
            , MOD_none
            , [this]
              {
                std::ofstream f("bookmarks.txt", std::ios_base::app);
                f << gWorld->getMapID() << " " << gWorld->camera.x << " " << gWorld->camera.y << " " << gWorld->camera.z << " " << ah << " " << av << " " << gWorld->getAreaID() << std::endl;
              }
            );

  addHotkey (SDLK_PAGEDOWN, MOD_none, [] { SnapSelectedObjectToGround(); });

  addHotkey (SDLK_n, MOD_none, [this] { mTimespeed += 90.0f; });
  addHotkey (SDLK_b, MOD_none, [this] { mTimespeed = std::max (0.0f, mTimespeed - 90.0f); });
  addHotkey (SDLK_j, MOD_none, [this] { mTimespeed = 0.0f; });

  addHotkey (SDLK_l, MOD_none, [] { gWorld->lighting = !gWorld->lighting; });

  addHotkey (SDLK_TAB, MOD_none, [this] { _GUIDisplayingEnabled = !_GUIDisplayingEnabled; });

  addHotkey (SDLK_DELETE, MOD_none, [] { DeleteSelectedObject(); });

  addHotkey (SDLK_v, MOD_shift, [] { InsertObject (14); });
  addHotkey (SDLK_v, MOD_alt, [] { InsertObject (15); });
  addHotkey (SDLK_v, MOD_ctrl, [] { mainGui->objectEditor->pasteObject(); });
  addHotkey ( SDLK_v
            , MOD_none
            , [] { mainGui->objectEditor->pasteObject(); }
            , [] { return terrainMode == 9; }
            );

  addHotkey ( SDLK_x
            , MOD_none
            , [] { mainGui->TexturePalette->toggleVisibility(); }
            , [] { return terrainMode == 2; }
            );

  addHotkey (SDLK_F4, MOD_shift, [] { Settings::getInstance()->AutoSelectingMode = !Settings::getInstance()->AutoSelectingMode; });
  addHotkey (SDLK_F7, MOD_shift, [] { Environment::getInstance()->view_holelines = !Environment::getInstance()->view_holelines; });

  addHotkey (SDLK_x, MOD_ctrl, [] { mainGui->guidetailInfos->toggleVisibility(); });

  addHotkey (SDLK_i, MOD_none, [this] { mousedir *= -1.f; });

  addHotkey (SDLK_i, MOD_ctrl, [] { Environment::getInstance()->paintMode = !Environment::getInstance()->paintMode; });

  addHotkey (SDLK_o, MOD_none, [this] { movespd *= 0.5f; });
  addHotkey (SDLK_p, MOD_none, [this] { movespd *= 2.0f; });

  addHotkey (SDLK_p, MOD_shift | MOD_ctrl, [] { Saving = true; });

  addHotkey (SDLK_r, MOD_none, [this] { ah += 180.f; });
  addHotkey (SDLK_r, MOD_ctrl, [] { ResetSelectedObjectRotation(); });

  addHotkey ( SDLK_g
            , MOD_none
            , []
              {
                // write teleport cords to txt file
                std::ofstream f("ports.txt", std::ios_base::app);
                f << "Map: " << gAreaDB.getAreaName(gWorld->getAreaID()) << " on ADT " << std::floor(gWorld->camera.x / TILESIZE) << " " << std::floor(gWorld->camera.z / TILESIZE) << std::endl;
                f << "Trinity:" << std::endl << ".go " << (ZEROPOINT - gWorld->camera.z) << " " << (ZEROPOINT - gWorld->camera.x) << " " << gWorld->camera.y << " " << gWorld->getMapID() << std::endl;
                f << "ArcEmu:" << std::endl << ".worldport " << gWorld->getMapID() << " " << (ZEROPOINT - gWorld->camera.z) << " " << (ZEROPOINT - gWorld->camera.x) << " " << gWorld->camera.y << " " << std::endl << std::endl;
                f.close();
              }
            );

  addHotkey ( SDLK_y
            , MOD_none
            , [] { mainGui->terrainTool->nextType(); }
            , [] { return terrainMode == 0; }
            );

  addHotkey ( SDLK_y
            , MOD_none
            , [] { mainGui->flattenTool->nextFlattenMode(); }
            , [] { return terrainMode == 1; }
            );

  addHotkey ( SDLK_u
            , MOD_none
            , [this]
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
            );


  // ESC warning
  mainGui->escWarning = new UIExitWarning(this);
  mainGui->escWarning->hide();
  mainGui->addChild(mainGui->escWarning);

  // CAPS warning
  mainGui->capsWarning = new UICapsWarning;
  mainGui->capsWarning->hide();
  mainGui->addChild(mainGui->capsWarning);

  // Water unable to save warning
  mainGui->waterSaveWarning = new UIWaterSaveWarning;
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
      mainGui->terrainTool->setTabletControlValue((float)app.pressure);
    case 1:
      mainGui->flattenTool->setRadius((float)app.pressure / 20.0f);
      break;
    case 2:
      mainGui->paintPressureSlider->setValue(std::max(0.0f, std::min(1.0f, (float)app.pressure / 2048.0f)));
      mainGui->paintPressureSlider->setValue(mainGui->paintPressureSlider->value);
      break;
    case 8:
      mainGui->shaderTool->setTabletControlValue((float)app.pressure);
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

    if (_mod_shift_down)
    {
      dirUp.x = 0.0f;
      dirUp.y = 1.0f;
      dirRight *= 0.0f; //! \todo  WAT?
    }
    else if (_mod_ctrl_down)
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
      if (_mod_ctrl_down && _mod_shift_down)  moveratio = 0.1f;
      else if (_mod_shift_down) moveratio = 0.01f;
      else if (_mod_ctrl_down) moveratio = 0.005f;
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
        ObjPos.x = std::abs(ObjPos.x);
      }

      // moving and scaling objects
      //! \todo  Alternatively automatically align it to the terrain.
      if (MoveObj && canMoveObj)
      {
        ObjPos.x = 80.0f;
        if (Selection->which() == eEntry_WMO)
        {
          gWorld->updateTilesWMO(boost::get<selected_wmo_type> (*Selection));

          if (_mod_shift_down)
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
          if (_mod_alt_down)
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
            if (_mod_shift_down)
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
          lModify = _mod_shift_down | _mod_ctrl_down | _mod_alt_down;
          if (_mod_shift_down)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.y;
          else if (_mod_ctrl_down)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.x;
          else if (_mod_alt_down)
            lTarget = &boost::get<selected_model_type> (*Selection)->dir.z;

        }
        else if (Selection->which() == eEntry_WMO)
        {
          lModify = _mod_shift_down | _mod_ctrl_down | _mod_alt_down;
          if (_mod_shift_down)
            lTarget = &boost::get<selected_wmo_type> (*Selection)->dir.y;
          else if (_mod_ctrl_down)
            lTarget = &boost::get<selected_wmo_type> (*Selection)->dir.x;
          else if (_mod_alt_down)
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
            if (_mod_shift_down)
            {
              mainGui->terrainTool->changeTerrain(7.5f * dt);
            }
            else if (_mod_ctrl_down)
            {
              mainGui->terrainTool->changeTerrain(-7.5f * dt);
            }
          }
          break;
        case 1:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              mainGui->flattenTool->flatten(dt);
            }
            else if (_mod_ctrl_down)
            {
              mainGui->flattenTool->blur(dt);
            }
          }
          break;
        case 2:
          if (_mod_shift_down && _mod_ctrl_down && _mod_alt_down)
          {
            // clear chunk texture
            if (mViewMode == eViewMode_3D && !underMap)
              gWorld->eraseTextures(xPos, zPos);
            else if (mViewMode == eViewMode_2D)
              gWorld->eraseTextures(CHUNKSIZE * 4.0f * video.ratio() * (static_cast<float>(MouseX) / static_cast<float>(video.xres()) - 0.5f) / gWorld->zoom + gWorld->camera.x, CHUNKSIZE * 4.0f * (static_cast<float>(MouseY) / static_cast<float>(video.yres()) - 0.5f) / gWorld->zoom + gWorld->camera.z);
          }
          else if (_mod_ctrl_down)
          {
            // Pick texture
            mainGui->TexturePicker->getTextures(*gWorld->GetCurrentSelection());
          }
          else  if (_mod_shift_down)
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
            if (_mod_shift_down)
            {
              auto pos (boost::get<selected_chunk_type> (*Selection).position);
              gWorld->setHole(pos.x, pos.z, _mod_alt_down, false);
            }
            else if (_mod_ctrl_down && !underMap)
            {
              gWorld->setHole(xPos, zPos, _mod_alt_down, true);
            }              
          }
          break;
        case 4:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              // draw the selected AreaId on current selected chunk
              gWorld->setAreaID(xPos, zPos, Environment::getInstance()->selectedAreaID, false);
            }
            else if (_mod_ctrl_down)
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
            if (_mod_shift_down)
            {
              gWorld->mapIndex->setFlag(true, xPos, zPos);
            }
            else if (_mod_ctrl_down)
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

            if (_mod_shift_down)
            {
              gWorld->addWaterLayerChunk(chnk->mt->index, chnk->px, chnk->py);
            }
            if (_mod_ctrl_down && !_mod_alt_down)
            {
              gWorld->delWaterLayerChunk(chnk->mt->index, chnk->px, chnk->py);
            }
            if (_mod_alt_down && !_mod_ctrl_down)
            {
              gWorld->mapIndex->setWater(true, xPos, zPos);
            }
            if (_mod_alt_down && _mod_ctrl_down)
              gWorld->mapIndex->setWater(false, xPos, zPos);
          }
          break;
        case 8:
          if (mViewMode == eViewMode_3D && !underMap)
          {
            if (_mod_shift_down)
            {
              mainGui->shaderTool->changeShader(dt, true);
            }
            if (_mod_ctrl_down)
            {
              mainGui->shaderTool->changeShader(dt, false);
            }
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
  math::vector_3d const pos
    ( ( ( math::look_at (gWorld->camera, gWorld->lookat, {0.0f, 1.0f, 0.0f}).transposed()
        * math::perspective (video.fov(), video.ratio(), video.nearclip(), video.farclip()).transposed()
        ).inverted().transposed()
      * video.normalized_device_coords ( Environment::getInstance()->screenX
                                       , Environment::getInstance()->screenY
                                       )
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

  if (results.empty())
  {
    gWorld->SetCurrentSelection (boost::none);
  }
  else
  {
    auto const& hit (results.front().second);
    gWorld->SetCurrentSelection (hit);

    auto const pos ( hit.which() == eEntry_Model ? boost::get<selected_model_type> (hit)->pos
                   : hit.which() == eEntry_WMO ? boost::get<selected_wmo_type> (hit)->pos
                   : hit.which() == eEntry_MapChunk ? boost::get<selected_chunk_type> (hit).position
                   : throw std::logic_error ("bad variant")
                   );
    Environment::getInstance()->Pos3DX = pos.x;
    Environment::getInstance()->Pos3DY = pos.y;
    Environment::getInstance()->Pos3DZ = pos.z;
  }
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

  //! \ todo: make the current tool return the radius
  float radius = 0.0f, hardness = 0.0f;

  switch (terrainMode)
  {
    case 0: radius = mainGui->terrainTool->brushRadius(); break;
    case 1: radius = mainGui->flattenTool->brushRadius(); break;
    case 2: 
      radius = textureBrush.getRadius(); 
      hardness = textureBrush.getHardness();
      break;
    case 6: break; //! \ todo: water: get radius 
    case 8: radius = mainGui->shaderTool->brushRadius(); break;
  }

  gWorld->draw(radius, hardness);

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

void MapView::keyPressEvent (SDL_KeyboardEvent *e)
{
  if (LastClicked && LastClicked->key_down (e->keysym.sym, e->keysym.unicode))
  {
    return;
  }

  if (e->keysym.mod & KMOD_CAPS)
    mainGui->capsWarning->show();
  else
    mainGui->capsWarning->hide();

  if (handleHotkeys(e))
    return;

  if (e->keysym.sym == SDLK_LSHIFT || e->keysym.sym == SDLK_RSHIFT)
    _mod_shift_down = true;

  if (e->keysym.sym == SDLK_LALT || e->keysym.sym == SDLK_RALT)
    _mod_alt_down = true;

  if (e->keysym.sym == SDLK_LCTRL || e->keysym.sym == SDLK_RCTRL)
    _mod_ctrl_down = true;

  if (e->keysym.sym == SDLK_SPACE)
    _mod_space_down = true;

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

  // copy model to clipboard
  if (e->keysym.sym == SDLK_c)
  {
    if (_mod_ctrl_down && gWorld->GetCurrentSelection())
      mainGui->objectEditor->copy(*gWorld->GetCurrentSelection());
    else if (_mod_alt_down && _mod_ctrl_down)
      mainGui->toggleCursorSwitcher();
    else if (_mod_shift_down)
      InsertObject(14);
    else if (_mod_alt_down)
      InsertObject(15);
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

  if (e->keysym.sym == SDLK_t)
  {
    // toggle flatten angle mode
    if (terrainMode == 1)
    {
      if (_mod_space_down)
      {
        mainGui->flattenTool->nextFlattenMode();
      }
      else
      {
        mainGui->flattenTool->toggleFlattenAngle();
      }
    }
    else if (terrainMode == 2)
    {
      sprayBrushActive = !sprayBrushActive;
      toggleSpray->setState(sprayBrushActive);
    }
    else if (terrainMode == 3)
    {
      gWorld->setHoleADT(gWorld->camera.x, gWorld->camera.z, _mod_alt_down);
    }
    else if (terrainMode == 9)
    {
      mainGui->objectEditor->togglePasteMode();
    }
  }

  // toggle help window
  if (e->keysym.sym == SDLK_h)
  {
    if (terrainMode == 9)
    {
      // toggle hidden models visibility
      if (_mod_space_down)
      {
        Environment::getInstance()->showModelFromHiddenList = !Environment::getInstance()->showModelFromHiddenList;
      }
      else if (_mod_shift_down)
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
      if (_mod_space_down)
      {
        mainGui->flattenTool->toggleFlattenLock();
      }
      else
      {
        mainGui->flattenTool->lockPos();
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
  //if( e->keysym.sym == SDLK_j && _mod_shift_down )
  //  gWorld->reloadTile( static_cast<int>( gWorld->camera.x ) / TILESIZE, static_cast<int>( gWorld->camera.z ) / TILESIZE );

  // fog distance or brush radius
  if (e->keysym.sym == SDLK_KP_PLUS || e->keysym.sym == SDLK_PLUS)
  {
    if (_mod_alt_down)
    {
      switch (terrainMode)
      {
      case 0:
        mainGui->terrainTool->changeRadius(0.01f);
        break;
      case 1:
        mainGui->flattenTool->changeRadius(0.01f);
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

        if (_mod_shift_down)
        {
          gWorld->setWaterTrans(tile, static_cast<unsigned char>(std::ceil(static_cast<float>(gWorld->getWaterTrans(tile)) + 1)));
        }
        else if (_mod_ctrl_down)
        {
          gWorld->setWaterHeight(tile, std::ceil(gWorld->getWaterHeight(tile) + 5.0f));
        }
        else
        {
          gWorld->setWaterHeight(tile, std::ceil(gWorld->getWaterHeight(tile) + 1.0f));
        }

        mainGui->guiWater->updateData();
      }
      else if (_mod_shift_down)
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
    if (_mod_alt_down)
    {
      switch (terrainMode)
      {
      case 0:
        mainGui->terrainTool->changeRadius(-0.01f);
        break;
      case 1:
        mainGui->flattenTool->changeRadius(-0.01f);
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
        if (_mod_shift_down)
        {
          gWorld->setWaterTrans(tile, static_cast<unsigned char>(std::floor(static_cast<float>(gWorld->getWaterTrans(tile))) - 1));
        }
        else if (_mod_ctrl_down)
        {
          gWorld->setWaterHeight(tile, std::ceil(gWorld->getWaterHeight(tile) - 5.0f));
        }
        else
        {
          gWorld->setWaterHeight(tile, std::ceil(gWorld->getWaterHeight(tile) - 1.0f));
        }

        mainGui->guiWater->updateData();
      }
      else if (_mod_shift_down)
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

  // doodads set
  if (e->keysym.sym >= SDLK_0 && e->keysym.sym <= SDLK_9)
  {
    if (gWorld->IsSelection(eEntry_WMO))
    {
      boost::get<selected_wmo_type> (*gWorld->GetCurrentSelection())->doodadset = e->keysym.sym - SDLK_0;
    }
    else if (_mod_shift_down)
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
    else if (_mod_alt_down)
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
}

void MapView::keyReleaseEvent (SDL_KeyboardEvent* e)
{
  if (e->keysym.mod & KMOD_CAPS)
    mainGui->capsWarning->show();
  else
    mainGui->capsWarning->hide();

  if (e->keysym.sym == SDLK_LSHIFT || e->keysym.sym == SDLK_RSHIFT)
    _mod_shift_down = false;

  if (e->keysym.sym == SDLK_LALT || e->keysym.sym == SDLK_RALT)
    _mod_alt_down = false;

  if (e->keysym.sym == SDLK_LCTRL || e->keysym.sym == SDLK_RCTRL)
    _mod_ctrl_down = false;

  if (e->keysym.sym == SDLK_SPACE)
    _mod_space_down = false;

  // movement
  if (e->keysym.sym == SDLK_w)
  {
    key_w = false;
    if (!(leftMouse && rightMouse) && moving > 0.0f)
    {
      moving = 0.0f;
    }
  }

  if (e->keysym.sym == SDLK_s && moving < 0.0f)
  {
    moving = 0.0f;
  }

  if (e->keysym.sym == SDLK_UP || e->keysym.sym == SDLK_DOWN)
    lookat = 0.0f;

  if (e->keysym.sym == SDLK_LEFT || e->keysym.sym == SDLK_RIGHT)
    turn = 0.0f;

  if (e->keysym.sym == SDLK_d && strafing > 0.0f)
  {
    strafing = 0.0f;
  }

  if (e->keysym.sym == SDLK_a && strafing < 0.0f)
  {
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

void MapView::inserObjectFromExtern(int model)
{
  InsertObject (model);
}


void MapView::mousemove(SDL_MouseMotionEvent *e)
{
  if ((look && !(_mod_shift_down || _mod_ctrl_down || _mod_alt_down)) || video.fullscreen())
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

  if (_mod_shift_down || _mod_ctrl_down || _mod_alt_down)
  {
    rh = e->xrel / XSENS * 5.0f;
    rv = e->yrel / YSENS * 5.0f;
  }

  if (rightMouse && _mod_alt_down)
  {
    if (terrainMode == 2)
    {
      float hardness = std::max(0.0f, std::min(1.0f, textureBrush.getHardness() + e->xrel / 300.0f));
      setTextureBrushHardness(hardness);
      mainGui->paintHardnessSlider->setValue(hardness);
    }
  }

  if (leftMouse && _mod_alt_down)
  {
    switch (terrainMode)
    {
    case 0:
      mainGui->terrainTool->changeRadius(e->xrel / XSENS);
      break;
    case 1:
      mainGui->flattenTool->changeRadius(e->xrel / XSENS);
      break;
    case 2:
      textureBrush.setRadius(std::max(0.0f, std::min(100.0f, textureBrush.getRadius() + e->xrel / XSENS)));
      paint_brush->setValue(textureBrush.getRadius() / 100.0f);
      break;
    case 8:
      mainGui->shaderTool->changeRadius(e->xrel / XSENS);
      break;
    }
  }

  if (leftMouse && _mod_space_down)
  {
    switch (terrainMode)
    {
    case 0:
      mainGui->terrainTool->changeSpeed(e->xrel / 30.0f);
      break;
    case 1:
      mainGui->flattenTool->changeSpeed(e->xrel / 30.0f);
      break;
    case 2:
      mainGui->paintPressureSlider->setValue(std::max(0.0f, std::min(1.0f, mainGui->paintPressureSlider->value + e->xrel / 300.0f)));
      break;
    case 8:
      mainGui->shaderTool->changeSpeed(e->xrel / XSENS);
      break;
    }
  }

  if (leftMouse && LastClicked)
  {
    LastClicked->processLeftDrag((float)(e->x - 4), (float)(e->y - 4), (float)(e->xrel), (float)(e->yrel));
  }

  if (mViewMode == eViewMode_2D && leftMouse && _mod_alt_down && _mod_shift_down)
  {
    strafing = ((e->xrel / XSENS) / -1) * 5.0f;
    moving = (e->yrel / YSENS) * 5.0f;
  }

  if (mViewMode == eViewMode_2D && rightMouse && _mod_shift_down)
  {
    updown = (e->yrel / YSENS);
  }

  mainGui->mouse_moved (e->x, e->y);

  Environment::getInstance()->screenX = MouseX = e->x;
  Environment::getInstance()->screenY = MouseY = e->y;
  checkWaterSave();
}

void MapView::selectModel(selection_type entry)
{
  mainGui->objectEditor->copy(entry);
}

void MapView::mousePressEvent (SDL_MouseButtonEvent *e)
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
      if (_mod_alt_down)
      {
        mainGui->flattenTool->changeOrientation(_mod_ctrl_down ? 1.0f : 10.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->flattenTool->changeAngle(_mod_ctrl_down ? 0.2f : 2.0f);
      }
      else if (_mod_space_down)
      {
        mainGui->flattenTool->changeHeight(1.0f);
      }
    }
    else if (terrainMode == 2)
    {
      if (_mod_space_down)
      {
        brushLevel = std::min(255.0f, brushLevel + 10.0f);
        mainGui->G1->setValue((255.0f - brushLevel) / 255.0f);
      }
      else if (_mod_alt_down)
      {
        brushSpraySize = std::min(40.0f, brushSpraySize + 2.0f);
        spray_size->setValue(brushSpraySize / 40.0f);
      }
      else if (_mod_shift_down)
      {
        brushSprayPressure = std::min(100.0f, brushSprayPressure + 2.5f);
        spray_pressure->setValue(brushSprayPressure / 100.0f);
      }
    }
    break;
  case SDL_BUTTON_WHEELDOWN:
    if (terrainMode == 1)
    {
      if (_mod_alt_down)
      {
        mainGui->flattenTool->changeOrientation(_mod_ctrl_down ? -1.0f : -10.0f);
      }
      else if (_mod_shift_down)
      {
        mainGui->flattenTool->changeAngle(_mod_ctrl_down ? -0.2f : -2.0f);
      }
      else if (_mod_space_down)
      {
        mainGui->flattenTool->changeHeight(-1.0f);
      }
    }
    else if (terrainMode == 2)
    {
      if (_mod_space_down)
      {
        brushLevel = std::max(0.0f, brushLevel - 10.0f);
        mainGui->G1->setValue((255.0f - brushLevel) / 255.0f);
      }
      else if (_mod_alt_down)
      {
        brushSpraySize = std::max(1.0f, brushSpraySize - 2.0f);
        spray_size->setValue(brushSpraySize / 40.0f);
      }
      else if (_mod_shift_down)
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

void MapView::mouseReleaseEvent (SDL_MouseButtonEvent* e)
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
