// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include "Video.h"

#include <cassert>
#include <SDL.h>
#include <string>

#include "Settings.h"
#include "Log.h"
#include <opengl/matrix.hpp>

Video video;

void Video::updateProjectionMatrix()
{
	gl.matrixMode(GL_PROJECTION);
	gl.loadIdentity();
	gl.viewport(0.0f, 0.0f, xres(), yres());
  opengl::matrix::perspective (fov(), ratio(), nearclip(), farclip());
	gl.matrixMode(GL_MODELVIEW);
	gl.loadIdentity();
}

void Video::resize(int xres_, int yres_)
{
	_xres = xres_;
	_yres = yres_;
	_ratio = static_cast<float>(xres()) / static_cast<float>(yres());

	LogDebug << "resize(" << xres() << ", " << yres() << ");" << std::endl;

	_primary = SDL_SetVideoMode(xres(), yres(), 0, _primary->flags);
	updateProjectionMatrix();
}

bool Video::init(int xres_, int yres_, bool fullscreen_, bool doAntiAliasing_)
{
	_xres = xres_;
	_yres = yres_;
	_ratio = static_cast<float>(xres()) / static_cast<float>(yres());

	_fov = math::degrees (45.0f);
	_nearclip = 1.0f;
	_farclip = Settings::getInstance()->FarZ;

	_fullscreen = fullscreen_;
	_doAntiAliasing = doAntiAliasing_;

	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO))
	{
		LogError << "SDL: " << SDL_GetError() << std::endl;
		exit(1);
	}

	int flags = SDL_OPENGL | SDL_HWSURFACE | SDL_ANYFORMAT | SDL_DOUBLEBUF | SDL_RESIZABLE;
	if (fullscreen())
	{
		flags |= SDL_FULLSCREEN;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	if (doAntiAliasing())
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		//! \todo Make sample count configurable.
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	}

	_primary = SDL_SetVideoMode(_xres, _yres, 0, flags);

	if (!_primary)
	{
		LogError << "SDL: " << SDL_GetError() << std::endl;
		exit(1);
	}

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		LogError << "GLEW: " << glewGetErrorString(err) << std::endl;
		return false;
	}

	gl.viewport(0.0f, 0.0f, xres(), yres());

	gl.enableClientState(GL_VERTEX_ARRAY);
	gl.enableClientState(GL_NORMAL_ARRAY);
	gl.enableClientState(GL_TEXTURE_COORD_ARRAY);

	mSupportCompression = GLEW_ARB_texture_compression;
	mSupportShaders = GLEW_ARB_vertex_program && GLEW_ARB_fragment_program;

	LogDebug << "GL: Version: " << gl.getString(GL_VERSION) << std::endl;
	LogDebug << "GL: Vendor: " << gl.getString(GL_VENDOR) << std::endl;
	LogDebug << "GL: Renderer: " << gl.getString(GL_RENDERER) << std::endl;

	return mSupportCompression;
}

void Video::close()
{
	SDL_FreeSurface(_primary);
	SDL_Quit();
}

void Video::flip() const
{
	SDL_GL_SwapBuffers();
}

void Video::clearScreen() const
{
	gl.clearColor(0.0f, 0.0f, 0.0f, 0.0f);
	gl.clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Video::set3D() const
{
	gl.matrixMode(GL_PROJECTION);
	gl.loadIdentity();
  opengl::matrix::perspective (fov(), ratio(), nearclip(), farclip());
	gl.matrixMode(GL_MODELVIEW);
	gl.loadIdentity();
	if (doAntiAliasing())
		gl.enable(GL_MULTISAMPLE);
}

void Video::set3D_select() const
{
	gl.matrixMode(GL_PROJECTION);
  opengl::matrix::perspective (fov(), ratio(), nearclip(), farclip());
	gl.matrixMode(GL_MODELVIEW);
	gl.loadIdentity();
	if (doAntiAliasing())
		gl.disable(GL_MULTISAMPLE);
}

void Video::set2D() const
{
	gl.matrixMode(GL_PROJECTION);
	gl.loadIdentity();
	gl.ortho(0.0f, xres(), yres(), 0.0f, -1.0f, 1.0f);
	gl.matrixMode(GL_MODELVIEW);
	gl.loadIdentity();
	if (doAntiAliasing())
		gl.disable(GL_MULTISAMPLE);
}

void Video::setTileMode() const
{
	gl.matrixMode(GL_PROJECTION);
	gl.loadIdentity();
	gl.ortho(-2.0f * ratio(), 2.0f * ratio(), 2.0f, -2.0f, -100.0f, 300.0f);
	gl.matrixMode(GL_MODELVIEW);
	gl.loadIdentity();
	if (doAntiAliasing())
		gl.enable(GL_MULTISAMPLE);
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
#include "MPQ.h"

namespace OpenGL
{
	Texture::Texture()
		: ManagedItem()
    , opengl::texture()
		, _width(0)
		, _height(0)
		, _filename("")
	{}

	void Texture::loadFromUncompressedData(BLPHeader* lHeader, char* lData)
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

	void Texture::loadFromCompressedData(BLPHeader* lHeader, char* lData)
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

	const std::string& Texture::filename()
	{
		return _filename;
	}

	void Texture::loadFromBLP(const std::string& filenameArg)
	{
		//! \todo Unload if there already is a model loaded?
		_filename = filenameArg;

		bind();

		MPQFile f(_filename);
		if (f.isEof())
		{
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
}
