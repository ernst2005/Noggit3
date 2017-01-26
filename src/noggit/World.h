// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/Frustum.h> // Frustum
#include <noggit/Model.h> // ModelManager
#include <noggit/Selection.h>
#include <noggit/Sky.h> // Skies, OutdoorLighting, OutdoorLightStats
#include <noggit/WMO.h> // WMOManager
#include <noggit/tile_index.hpp>

#include <map>
#include <string>

namespace OpenGL
{
  class Texture;
};
namespace opengl
{
  class call_list;
}

class Brush;
class MapTile;
class MapIndex;

static const float detail_size = 8.0f;
static const float highresdistance = 384.0f;
static const float mapdrawdistance = 998.0f;
static const float modeldrawdistance = 384.0f;
static const float doodaddrawdistance = 64.0f;

using StripType = uint16_t;

class World
{
public:
  // Which tile are we over / entering?
  int ex;
  int ez;
  int cx;
  int cz;

  MapIndex *mapIndex;

  // Information about the currently selected model / WMO / triangle.
  boost::optional<selection_type> mCurrentSelection;
  bool SelectionMode;

  // Call lists for the low resolution heightmaps.
  opengl::call_list *lowrestiles[64][64];

  // Temporary variables for loading a WMO, if we have a global WMO.
  std::string mWmoFilename;
  ENTRY_MODF mWmoEntry;

  // Vertex Buffer Objects for coordinates used for drawing.
  GLuint detailtexcoords;
  GLuint alphatexcoords;

  // Map ID of this World.
  unsigned int mMapId;

  // The lighting used.
  OutdoorLighting *ol;

  // Light attenuation related parameters.
  float l_const;
  float l_linear;
  float l_quadratic;


  void initMinimap();
  void initLowresTerrain();

  void outdoorLighting();
  void outdoorLighting2();

  unsigned int getMapID();
  // Do we draw *? Should be moved somewhere else, these are not World related.
  bool drawdoodads;
  bool drawfog;
  bool drawlines;
  bool drawmodels;
  bool drawterrain;
  bool drawwater;
  bool drawwmo;
  bool drawwireframe;
  bool draw_mfbo;
  bool lighting;
  bool renderAnimations;
  // Time of the day.
  float animtime;
  float time;

  //! \brief Name of this map.
  std::string basename;

  // Dynamic distances for rendering. Actually, these should be the same..
  float fogdistance;
  float culldistance;

  float minX;
  float maxX;
  float minY;
  float maxY;
  float zoom;

  Skies *skies;

  bool loading;

  bool autoheight;

  //! \todo  Get these managed? ._.
  std::map<int, ModelInstance> mModelInstances;
  std::map<int, WMOInstance> mWMOInstances;

  OutdoorLightStats outdoorLightStats;

  GLuint minimap;

  StripType *mapstrip;
  StripType *mapstrip2;

  math::vector_3d camera;
  math::vector_3d lookat;

  explicit World(const std::string& name);
  ~World();

  void initDisplay();

  void tick(float dt);
  void draw(float brushRadius, float hardness = 0.0f);

  void outdoorLights(bool on);
  void setupFog();

  //! \brief Get the area ID of the tile on which the camera currently is on.
  unsigned int getAreaID();
  void setAreaID(float x, float z, int id, bool adt);

  selection_result intersect (math::ray const&, bool only_map);
  void drawTileMode(float ah);

  void initGlobalVBOs(GLuint* pDetailTexCoords, GLuint* pAlphaTexCoords);

  bool HasSelection();

  // Selection related methods.
  bool IsSelection(int pSelectionType);
  boost::optional<selection_type> GetCurrentSelection() { return mCurrentSelection; }
  void SetCurrentSelection (boost::optional<selection_type> entry) { mCurrentSelection = entry; }
  void ResetSelection() { mCurrentSelection.reset(); }

  bool GetVertex(float x, float z, math::vector_3d *V);

  // check if the cursor is under map or in an unloaded tile
  bool isUnderMap(float x, float z, float h);

  template<typename Fun>
    bool for_all_chunks_in_range ( float x
                                 , float z
                                 , float radius
                                 , Fun&& /* MapChunk* -> bool changed */
                                 );
  template<typename Fun, typename Post>
    bool for_all_chunks_in_range ( float x
                                 , float z
                                 , float radius
                                 , Fun&& /* MapChunk* -> bool changed */
                                 , Post&& /* MapChunk* -> void; called for all changed chunks */
                                 );
  template<typename Fun>
    void for_all_chunks_on_tile (float x, float z, Fun&&);

  template<typename Fun>
    void for_chunk_at(float x, float z, Fun&&);

  void changeTerrain(float x, float z, float change, float radius, int BrushType);
  void changeShader(float x, float z, float change, float radius, bool editMode);
  void flattenTerrain(float x, float z, float remain, float radius, int BrushType, int flattenType, const math::vector_3d& origin, math::degrees angle, math::degrees orientation);
  void blurTerrain(float x, float z, float remain, float radius, int BrushType);
  bool paintTexture(float x, float z, Brush *brush, float strength, float pressure, OpenGL::Texture* texture);
  bool sprayTexture(float x, float z, Brush *brush, float strength, float pressure, float spraySize, float sprayPressure, OpenGL::Texture* texture);
  
  void eraseTextures(float x, float z);
  void overwriteTextureAtCurrentChunk(float x, float z, OpenGL::Texture* oldTexture, OpenGL::Texture* newTexture);
  void setBaseTexture(float x, float z);
  void swapTexture(float x, float z, OpenGL::Texture *tex);
  void removeTexDuplicateOnADT(float x, float z);

  void setHole(float x, float z, bool big, bool hole);
  void setHoleADT(float x, float z, bool hole);

  void addModel(selection_type, math::vector_3d newPos, bool copyit);
  void addM2(std::string const& filename, math::vector_3d newPos, bool copyit);
  void addWMO(std::string const& filename, math::vector_3d newPos, bool copyit);


  void updateTilesEntry(selection_type const& entry);
  void updateTilesWMO(WMOInstance* wmo);
  void updateTilesModel(ModelInstance* m2);

  void clearHiddenModelList();

  void jumpToCords(math::vector_3d pos);
  void saveMap();

  void deleteModelInstance(int pUniqueID);
  void deleteWMOInstance(int pUniqueID);

  void delete_duplicate_model_and_wmo_instances();

	static bool IsEditableWorld(int pMapId);
	void clearHeight(int id, const tile_index& tile);
	void clearHeight(int id, const tile_index& tile, int _cx, int _cz);
  void clearHeight(float x, float z);

  void saveWDT();
  void clearAllModelsOnADT(const tile_index& tile);

  bool canWaterSave(const tile_index& tile);

  void setWaterHeight(const tile_index& tile, float h);
  float getWaterHeight(const tile_index& tile);
  float HaveSelectWater(const tile_index& tile);
  void CropWaterADT(const tile_index& tile);
  void setWaterTrans(const tile_index& tile, unsigned char value);
  unsigned char getWaterTrans(const tile_index& tile);

  void setWaterType(const tile_index& tile, int type);
  int getWaterType(const tile_index& tile);

  void deleteWaterLayer(const tile_index& tile);
  void ClearShader(const tile_index& tile);

  void addWaterLayer(const tile_index& tile);
  void addWaterLayer(const tile_index& tile, float height, unsigned char trans);
  void addWaterLayerChunk(const tile_index& tile, int i, int j);
  void delWaterLayerChunk(const tile_index& tile, int i, int j);

  void autoGenWaterTrans(const tile_index& tile, int factor);
  void AddWaters(const tile_index& tile);

  void fixAllGaps();

  void convertMapToBigAlpha();

  // get the real cursor pos in the world, TODO: get the correct pos on models/wmos
  math::vector_3d getCursorPosOnModel();
private:
  void getSelection();
};

extern World *gWorld;
