// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/TextureManager.h>
#include <noggit/Video.h>
#include <noggit/Log.h> // LogDebug
#include <opengl/context.hpp>

#include <algorithm>

decltype (TextureManager::_) TextureManager::_;

void TextureManager::report()
{
  std::string output = "Still in the Texture manager:\n";
  _.apply ( [&] (std::string const& key, blp_texture const&)
            {
              output += " - " + key + "\n";
            }
          );
  LogDebug << output;
}

std::vector<scoped_blp_texture_reference> TextureManager::getAllTexturesMatching(bool(*function)(const std::string& name))
{
  std::vector<scoped_blp_texture_reference> results;
  _.apply ( [&] (std::string const& key, blp_texture const&)
            {
              if (function (key))
              {
                results.emplace_back (key);
              }
            }
          );
  return results;
}

#include <stdint.h>
//! \todo Cross-platform syntax for packed structs.
#pragma pack(push,1)
struct BLPHeader
{
  int32_t magix;
  int32_t version;
  uint8_t attr_0_compression;
  uint8_t attr_1_alphadepth;
  uint8_t attr_2_alphatype;
  uint8_t attr_3_mipmaplevels;
  int32_t resx;
  int32_t resy;
  int32_t offsets[16];
  int32_t sizes[16];
};
#pragma pack(pop)

#include <boost/thread.hpp>
#include <noggit/MPQ.h>

void blp_texture::loadFromUncompressedData(BLPHeader* lHeader, char* lData)
{
  unsigned int * pal = reinterpret_cast<unsigned int*>(lData + sizeof(BLPHeader));

  unsigned char *buf;
  unsigned int *buf2 = new unsigned int[_width*_height];
  unsigned int *p;
  unsigned char *c, *a;

  int alphabits = lHeader->attr_1_alphadepth;
  bool hasalpha = alphabits != 0;

  for (int i = 0; i<16; ++i)
  {
    _width = std::max(1, _width);
    _height = std::max(1, _height);

    if (lHeader->offsets[i] && lHeader->sizes[i])
    {
      buf = reinterpret_cast<unsigned char*>(&lData[lHeader->offsets[i]]);

      int cnt = 0;
      p = buf2;
      c = buf;
      a = buf + _width*_height;
      for (int y = 0; y<_height; y++)
      {
        for (int x = 0; x<_width; x++)
        {
          unsigned int k = pal[*c++];
          k = ((k & 0x00FF0000) >> 16) | ((k & 0x0000FF00)) | ((k & 0x000000FF) << 16);
          int alpha = 0xFF;
          if (hasalpha)
          {
            if (alphabits == 8)
            {
              alpha = (*a++);
            }
            else if (alphabits == 1)
            {
              alpha = (*a & (1 << cnt++)) ? 0xff : 0;
              if (cnt == 8)
              {
                cnt = 0;
                a++;
              }
            }
          }

          k |= alpha << 24;
          *p++ = k;
        }
      }

      gl.texImage2D(GL_TEXTURE_2D, i, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf2);

    }
    else
    {
      return;
    }

    _width >>= 1;
    _height >>= 1;
  }

  delete[] buf2;
  delete[] buf;
}

void blp_texture::loadFromCompressedData(BLPHeader* lHeader, char* lData)
{
  //                         0 (0000) & 3 == 0                1 (0001) & 3 == 1                    7 (0111) & 3 == 3
  const int alphatypes[] = { GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT };
  const int blocksizes[] = { 8, 16, 0, 16 };

  int lTempAlphatype = lHeader->attr_2_alphatype & 3;
  GLint format = alphatypes[lTempAlphatype];
  int blocksize = blocksizes[lTempAlphatype];
  format = format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? (lHeader->attr_1_alphadepth == 1 ? GL_COMPRESSED_RGBA_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT) : format;

  // do every mipmap level
  for (int i = 0; i < 16; ++i)
  {
    _width = std::max(1, _width);
    _height = std::max(1, _height);

    if (lHeader->offsets[i] && lHeader->sizes[i])
    {
      gl.compressedTexImage2D(GL_TEXTURE_2D, i, format, _width, _height, 0, ((_width + 3) / 4) * ((_height + 3) / 4) * blocksize, reinterpret_cast<char*>(lData + lHeader->offsets[i]));
    }
    else
    {
      return;
    }

    _width >>= 1;
    _height >>= 1;
  }
}

const std::string& blp_texture::filename()
{
  return _filename;
}

blp_texture::blp_texture(const std::string& filenameArg)
{
  //! \todo Unload if there already is a model loaded?
  _filename = filenameArg;

  bind();

  MPQFile f(_filename);
  if (f.isEof())
  {
    LogError << "file not found: '" << _filename << "'" << std::endl;
    throw std::runtime_error ("bad filename");
  }

  char* lData = f.getPointer();
  BLPHeader* lHeader = reinterpret_cast<BLPHeader*>(lData);
  _width = lHeader->resx;
  _height = lHeader->resy;

  if (lHeader->attr_0_compression == 1)
  {
    loadFromUncompressedData(lHeader, lData);
  }
  else if (lHeader->attr_0_compression == 2)
  {
    loadFromCompressedData(lHeader, lData);
  }

  f.close();

  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
