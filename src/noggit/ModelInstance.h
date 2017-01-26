// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <math/ray.hpp>
#include <math/vector_3d.hpp> // math::vector_3d
#include <noggit/MPQ.h> // MPQFile
#include <noggit/MapHeaders.h> // ENTRY_MDDF
#include <noggit/ModelManager.h>
#include <noggit/Selection.h>
#include <noggit/tile_index.hpp>

class Frustum;
class Model;

class ModelInstance
{
public:
  scoped_model_reference model;
  math::vector_3d extents[2];

  math::vector_3d pos, dir;

  //! \todo  Get this out and do somehow else.
  unsigned int d1;

  float w, sc;

  math::vector_3d ldir;
  math::vector_3d lcol;

  explicit ModelInstance(std::string const& filename);
  explicit ModelInstance(std::string const& filename, MPQFile* f);
  explicit ModelInstance(std::string const& filename, ENTRY_MDDF *d);

  ModelInstance(ModelInstance const& other) = default;
  ModelInstance& operator= (ModelInstance const& other) = default;

  ModelInstance (ModelInstance&& other)
    : model (std::move (other.model))
    // , extents (other.extents)
    , pos (other.pos)
    , dir (other.dir)
    , d1 (other.d1)
    , w (other.w)
    , sc (other.sc)
    , ldir (other.ldir)
    , lcol (other.lcol)
  {
    std::swap (extents, other.extents);
  }
  ModelInstance& operator= (ModelInstance&& other)
  {
    std::swap (model, other.model);
    std::swap (extents, other.extents);
    std::swap (pos, other.pos);
    std::swap (dir, other.dir);
    std::swap (d1, other.d1);
    std::swap (w, other.w);
    std::swap (sc, other.sc);
    std::swap (ldir, other.ldir);
    std::swap (lcol, other.lcol);
    return *this;
  }

  void draw (Frustum const&);
  void drawMapTile();
  //  void drawHighlight();
  void intersect (math::ray const&, selection_result*);
  void draw2(const math::vector_3d& ofs, const math::degrees, Frustum const&);

  void resetDirection();

  bool isInsideRect(math::vector_3d rect[2]);

  void recalcExtents();
};
