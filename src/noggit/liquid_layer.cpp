// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBC.h>
#include <noggit/liquid_layer.hpp>
#include <noggit/Log.h>
#include <noggit/MapChunk.h>
#include <noggit/Misc.h>
#include <opengl/call_list.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <algorithm>
#include <string>


liquid_layer::liquid_layer(math::vector_3d const& base, float height, int liquid_id)
  : _liquid_id(liquid_id)
  , _liquid_vertex_format(0)
  , _minimum(height)
  , _maximum(height)
  , _subchunks(0)
  , pos(base)
  , texRepeats(4.0f)
  , _render()
{
  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      _depth.emplace_back(1.0f);
      _vertices.emplace_back(pos.x + UNITSIZE * x
        , height
        , pos.z + UNITSIZE * z
      );
    }
  }

  changeLiquidID(_liquid_id);
}

liquid_layer::liquid_layer(math::vector_3d const& base, MH2O_Information const& info, MH2O_HeightMask const& heightmask, std::uint64_t infomask)
  : _liquid_vertex_format(info.liquid_vertex_format)
  , _minimum(info.minHeight)
  , _maximum(info.maxHeight)
  , _liquid_id(info.liquid_id)
  , _subchunks(0)
  , pos(base)
  , texRepeats(4.0f)
  , _render()
{
  int offset = 0;
  for (int z = 0; z < info.height; ++z)
  {
    for (int x = 0; x < info.width; ++x)
    {
      setSubchunk(x + info.xOffset, z + info.yOffset, (infomask >> offset) & 1);
      offset++;
    }
  }

  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      _depth.emplace_back(heightmask.mTransparency[z][x]/255.0f);
      _vertices.emplace_back( pos.x + UNITSIZE * x
                            , heightmask.mHeightValues[z][x]
                            , pos.z + UNITSIZE * z
                            );
    }
  }
  
  changeLiquidID(_liquid_id);
}

liquid_layer::liquid_layer(liquid_layer const& other)
  : _liquid_id(other._liquid_id)
  , _liquid_vertex_format(other._liquid_vertex_format)
  , _minimum(other._minimum)
  , _maximum(other._maximum)
  , _subchunks(other._subchunks)
  , _vertices(other._vertices)
  , _depth(other._depth)
  , pos(other.pos)
  , texRepeats(other.texRepeats)
{
  changeLiquidID(_liquid_id);
}

liquid_layer& liquid_layer::operator=(liquid_layer const& other)
{
  changeLiquidID(other._liquid_id);
  _liquid_vertex_format = other._liquid_vertex_format;  
  _minimum = other._minimum;
  _maximum = other._maximum;
  _subchunks = other._subchunks;
  _vertices = other._vertices;
  _depth = other._depth;
  pos = other.pos;
  texRepeats = other.texRepeats;
  
  return *this;
}

void liquid_layer::save(sExtendableArray& adt, int base_pos, int& info_pos, int& current_pos) const
{
  int min_x = 9, min_z = 9, max_x = 0, max_z = 0;
  bool filled = true;

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      if (hasSubchunk(x, z))
      {
        min_x = std::min(x, min_x);
        min_z = std::min(z, min_z);
        max_x = std::max(x + 1, max_x);
        max_z = std::max(z + 1, max_z);
      }
      else
      {
        filled = false;
      }
    }
  }

  MH2O_Information info;
  std::uint64_t mask = 0;

  info.liquid_id = _liquid_id;
  info.liquid_vertex_format = _liquid_vertex_format;
  info.minHeight = _minimum;
  info.maxHeight = _maximum;
  info.xOffset = min_x;
  info.yOffset = min_z;
  info.width = max_x - min_x;
  info.height = max_z - min_z;

  if (filled)
  {
    info.ofsInfoMask = 0;
  }
  else
  {
    std::uint64_t value = 1;
    for (int z = info.yOffset; z < info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x < info.xOffset + info.width; ++x)
      {
        if (hasSubchunk(x, z))
        {
          mask |= value;
        }
        value <<= 1;
      }
    }

    if (mask > 0)
    {
      info.ofsInfoMask = current_pos - base_pos;
      adt.Insert(current_pos, 8, reinterpret_cast<char*>(&mask));
      current_pos += 8;
    }
  } 

  info.ofsHeightMap = current_pos - base_pos;

  std::size_t heighmap_size = (info.width + 1) * (info.height +1) * (sizeof(char) + ((_liquid_id != 2) ? sizeof(float) : 0));
  adt.Extend(heighmap_size);

  if (_liquid_vertex_format != 2)
  {
    for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
    {
      for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
      {
        memcpy(adt.GetPointer<char>(current_pos), &_vertices[z * 9 + x].y, sizeof(float));
        current_pos += sizeof(float);
      }
    }
  }

  for (int z = info.yOffset; z <= info.yOffset + info.height; ++z)
  {
    for (int x = info.xOffset; x <= info.xOffset + info.width; ++x)
    {
      unsigned char depth = static_cast<unsigned char>(_depth[z * 9 + x] * 255.0f);
      memcpy(adt.GetPointer<char>(current_pos), &depth, sizeof(char));
      current_pos += sizeof(char);
    }
  }

  memcpy(adt.GetPointer<char>(info_pos), &info, sizeof(MH2O_Information));
  info_pos += sizeof(MH2O_Information);
}

void liquid_layer::changeLiquidID(int id)
{
  _liquid_id = id;

  try
  {
    DBCFile::Record lLiquidTypeRow = gLiquidTypeDB.getByID(_liquid_id);
    _render.setTextures(lLiquidTypeRow.getString(LiquidTypeDB::TextureFilenames - 1));

    // !\ todo: handle lava (type == 2) that use uv_mapping
    switch (lLiquidTypeRow.getInt(LiquidTypeDB::Type))
    {
    case 1: // ocean
      _liquid_vertex_format = 2;
      break;
    default:
      _liquid_vertex_format = 0;
      break;
    }
    
    //! \todo  Get texRepeats too.
  }
  catch (...)
  {
    // Fallback, when there is no information.
    _render.setTextures("XTextures\\river\\lake_a.%d.blp");
  }
}

void liquid_layer::updateRender()
{
  depths.clear();
  tex_coords.clear();
  indices.clear();
  vertices.clear();

  std::uint16_t index (0);

  for (int j = 0; j < 8; ++j)
  {
    for (int i = 0; i < 8; ++i)
    {
      if (!hasSubchunk(i, j))
      {
        continue;
      }

      size_t p = j * 9 + i;

      depths.emplace_back (1.f - _depth[p]);
      tex_coords.emplace_back (i + 0.f, j + 0.f);
      vertices.emplace_back (_vertices[p]);
      indices.emplace_back (index++);

      depths.emplace_back (1.f - _depth[p + 9]);
      tex_coords.emplace_back (i + 0.f, j + 1.f);
      vertices.emplace_back (_vertices[p + 9]);
      indices.emplace_back (index++);

      depths.emplace_back (1.f - _depth[p + 10]);
      tex_coords.emplace_back (i + 1.f, j + 1.f);
      vertices.emplace_back (_vertices[p + 10]);
      indices.emplace_back (index++);

      depths.emplace_back (1.f - _depth[p + 1]);
      tex_coords.emplace_back (i + 1.f, j + 0.f);
      vertices.emplace_back (_vertices[p + 1]);
      indices.emplace_back (index++);
    }
  }

  gl.bufferData<GL_ELEMENT_ARRAY_BUFFER>
    ( _index_buffer[0]
    , indices.size() * sizeof (*indices.data())
    , indices.data()
    , GL_STATIC_DRAW
    );
}

void liquid_layer::draw (opengl::scoped::use_program& water_shader)
{
  _render.prepare_draw (water_shader);

  water_shader.attrib ("position", vertices);
  water_shader.attrib ("tex_coord", tex_coords);
  water_shader.attrib ("depth", depths);
  water_shader.uniform ("tex_repeat", texRepeats);

  gl.drawElements (GL_QUADS, _index_buffer[0], indices.size(), GL_UNSIGNED_SHORT, nullptr);
}

void liquid_layer::crop(MapChunk* chunk)
{
  bool changed = false;

  if (_maximum < chunk->getMinHeight())
  {
    _subchunks = 0;
    changed = true;
  }
  else
  {
    for (int z = 0; z < 8; ++z)
    {
      for (int x = 0; x < 8; ++x)
      {
        if (hasSubchunk(x, z))
        {
          int water_index = 9 * z + x, terrain_index = 17 * z + x;

          if (_vertices[water_index].y < chunk->mVertices[terrain_index].y
            && _vertices[water_index + 1].y < chunk->mVertices[terrain_index + 1].y
            && _vertices[water_index + 9].y < chunk->mVertices[terrain_index + 17].y
            && _vertices[water_index + 10].y < chunk->mVertices[terrain_index + 18].y
            )
          {
            setSubchunk(x, z, false);
            changed = true;
          }
        }
      }
    }
  }

  update_min_max();
}

void liquid_layer::update_opacity(MapChunk* chunk, float factor)
{
  for (int z = 0; z < 9; ++z)
  {
    for (int x = 0; x < 9; ++x)
    {
      update_vertex_opacity(x, z, chunk, factor);
    }
  }
}

bool liquid_layer::hasSubchunk(int x, int z) const
{
  return (_subchunks >> (z * 8 + x)) & 1;
}

void liquid_layer::setSubchunk(int x, int z, bool water)
{
  std::uint64_t v = std::uint64_t(1) << (z*8+x);
  _subchunks = water ? (_subchunks | v) : (_subchunks & ~v);
}

void liquid_layer::paintLiquid( math::vector_3d const& pos
                              , float radius
                              , bool add
                              , math::radians const& angle
                              , math::radians const& orientation
                              , bool lock
                              , math::vector_3d const& origin
                              , bool override_height
                              , MapChunk* chunk
                              , float opacity_factor
                              )
{
  bool ocean = _liquid_vertex_format == 2;
  math::vector_3d ref ( lock
                      ? origin
                      : math::vector_3d (pos.x, pos.y + 1.0f, pos.z)
                      );

  // make sure the ocean layers are flat
  if (add && ocean)
  {
    for (math::vector_3d& v : _vertices)
    {
      v.y = ref.y;
    }
    _minimum = ref.y;
    _maximum = ref.y;

    update_opacity(chunk, opacity_factor);
  }

  int id = 0;  

  for (int z = 0; z < 8; ++z)
  {
    for (int x = 0; x < 8; ++x)
    {
      if (misc::getShortestDist(pos, _vertices[id], UNITSIZE) <= radius)
      {
        if (add && !ocean)
        {
          for (int index : {id, id + 1, id + 9, id + 10})
          {
            bool no_subchunk = !hasSubchunk(x, z);
            bool in_range = misc::dist(pos, _vertices[index]) <= radius;

            if (no_subchunk || (in_range && override_height))
            {
              _vertices[index].y = misc::angledHeight(ref, _vertices[index], angle, orientation);
            }
            if (no_subchunk || in_range)
            {
              update_vertex_opacity(index % 9, index / 9, chunk, opacity_factor);
            }            
          }
        }
        setSubchunk(x, z, add);
      }

      id++;
    }
    // to go to the next row of subchunks
    id++;
  }

  // update done earlier for oceans
  if (!ocean)
  {
    update_min_max();
  }

  
}

void liquid_layer::update_min_max()
{
  _minimum = std::numeric_limits<float>::max();
  _maximum = std::numeric_limits<float>::lowest();
  int x = 0, z = 0;

  for (math::vector_3d& v : _vertices)
  {
    if (hasSubchunk(std::min(x, 7), std::min(z, 7)))
    {
      _maximum = std::max(_maximum, v.y);
      _minimum = std::min(_minimum, v.y);
    }

    if (++x == 9)
    {
      z++;
      x = 0;
    }
  }

  // LVF 2 => flat water (ocean)
  if (_liquid_vertex_format == 2)
  {
    _maximum = _minimum;
  }
}

void liquid_layer::copy_subchunk_height(int x, int z, liquid_layer const& from)
{
  int id = 9 * z + x;

  for (int index : {id, id + 1, id + 9, id + 10})
  {
    _vertices[index].y = from._vertices[index].y;
  }

  setSubchunk(x, z, true);
}

void liquid_layer::update_vertex_opacity(int x, int z, MapChunk* chunk, float factor)
{
  float diff = _vertices[z * 9 + x].y - chunk->mVertices[z * 17 + x].y;
  _depth[z * 9 + x] = diff < 0.0f ? 0.0f : (std::min(1.0f, std::max(0.0f, (diff + 1.0f) * factor)));
}
