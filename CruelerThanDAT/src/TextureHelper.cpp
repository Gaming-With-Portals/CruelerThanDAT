#include "pch.hpp"
#include "TextureHelper.h"

#include "BinaryHandler.h"
#include "Log.h"

#include <glad/glad.h>
#include <gli/gli.hpp>
#include <ddz.h>

void unswizzle(uint8_t *dst, uint8_t *src, int32_t w, int32_t h, int32_t bpp) {
	uint32_t maskU = 0;
	uint32_t maskV = 0;
	int32_t i = 1;
	int32_t j = 1;
	int32_t c;
	do{
		c = 0;
		if(i < w){
			maskU |= j;
			j <<= 1;
			c = j;
		}
		if(i < h){
			maskV |= j;
			j <<= 1;
			c = j;
		}
		i <<= 1;
	}while(c);
	int32_t x, y, u, v;
	v = 0;
	for(y = 0; y < h; y++){
		u = 0;
		for(x = 0; x < w; x++){
			memcpy(&dst[(y*w + x)*bpp], &src[(u|v)*bpp], bpp);
			u = (u - maskU) & maskU;
		}
		v = (v - maskV) & maskV;
	}
}

void TextureHelper::LoadData(BinaryReader& wta, BinaryReader& wtp, std::unordered_map<unsigned int, unsigned int>& textureMap) {
	bool isConsole = false;
	WTB_EXDATA exData = EXDATA_NONE;
	if (wta.GetSize() < 8) {
		return;
	}

	wta.SetEndianess(false);
	wtp.SetEndianess(false);
	uint32_t magic = wta.ReadUINT32();
	if (magic == 4346967) {
		isConsole = false; // needed to justify having this if statement, sue me
	}
	else if (magic == 1465139712) { // crazy fuckup by platinum ngl
		wta.SetEndianess(true);
		wtp.SetEndianess(true);
		isConsole = true;
	}
	else {
		return;
	}

	uint32_t version = wta.ReadUINT32();


	if (version != 1 && version != 3 && version != 0) {
		return;
	}
	uint32_t textureCount = wta.ReadUINT32();

	uint32_t offsetsOffset = wta.ReadUINT32();
	uint32_t sizeOffset = wta.ReadUINT32();
	uint32_t flagOffset = wta.ReadUINT32();
	uint32_t idOffset = wta.ReadUINT32();
	uint32_t exOffset = wta.ReadUINT32();

	if (exOffset != 0) {
		wta.Seek(exOffset);
		if (wta.ReadUINT32() == 3232856 && version == 3) {
			exData = EXDATA_XT1; 
		}
		else {
			exData = EXDATA_XBOX; // I don't think the PS3 has ExData, system is expandable incase
		}
			
	}		
	wta.Seek(offsetsOffset);
	std::vector<uint32_t> offsets = wta.ReadUINT32Array(textureCount);
	wta.Seek(sizeOffset);
	std::vector<uint32_t> sizes = wta.ReadUINT32Array(textureCount);
	wta.Seek(flagOffset);
	std::vector<uint32_t> flags = wta.ReadUINT32Array(textureCount);
	std::vector<uint32_t> ids;
	if (idOffset == 0) {
		
		for (uint32_t i = 0; i < textureCount; i++) { ids.push_back(i); };
	}
	else {
		wta.Seek(idOffset);
		ids = wta.ReadUINT32Array(textureCount);
	}


	for (uint32_t i = 0; i < textureCount; i++) {
		if (sizes[i] == 0) {continue;}


		unsigned int glTextureID = 0;
		wtp.Seek(offsets[i]);
		uint32_t identifier = wtp.ReadUINT32();
		if (identifier == 542327876) { // Microsoft DirectDraw Surface
			wtp.Seek(offsets[i]);
			std::vector<char> data = wtp.ReadBytes(sizes[i]);
			glGenTextures(1, &glTextureID);
			gli::texture tex = gli::load(data.data(), data.size());
			gli::gl GL(gli::gl::PROFILE_GL33);
			gli::gl::format const Format = GL.translate(tex.format(), tex.swizzles());
			GLenum target = GL.translate(tex.target());
			glBindTexture(target, glTextureID);
			for (std::size_t Level = 0; Level < tex.levels(); ++Level) {
				glm::tvec3<GLsizei> extent(tex.extent(Level));
				glCompressedTexImage2D(target, static_cast<GLint>(Level), Format.Internal, extent.x, extent.y, 0, static_cast<GLsizei>(tex.size(Level)), tex.data(0, 0, Level));
			}
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		
			glGenerateMipmap(target);
		}
		else {
			if (exData == EXDATA_XBOX) {
				// XPR?
				wta.Seek((exOffset + (i*52)) + 28);
				uint16_t flag = wta.ReadUINT16();
				wta.ReadUINT16();
				uint32_t texFmt = wta.ReadUINT32();
				uint16_t height = ((wta.ReadUINT16() & 1023) + 1) << 3;
				uint16_t width = (((wta.ReadUINT16() >> 5) & 127) + 1) << 5;					
				wtp.Seek(offsets[i]);
				std::vector<char> data = wtp.ReadBytes(sizes[i]);

				glGenTextures(1, &glTextureID);
				glBindTexture(GL_TEXTURE_2D, glTextureID);

				GLenum internalFormat;
				switch (texFmt) {
				case 82: internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; break;
				case 83: internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
				case 84: internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
				default:
					std::cerr << "Unsupported format" << std::endl;
					return;
				}

			/*
				switch (internalFormat) {
					case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: {
						printf("GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: %x\n", GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
						std::ofstream file("DXT1.rds", std::ios::binary);
						const auto f = static_cast<uint32_t>(internalFormat);
						const auto s = static_cast<uint32_t>(data.size());
						const auto w = static_cast<uint32_t>(width);
						const auto h = static_cast<uint32_t>(height);
						file.write(reinterpret_cast<const char *>(&f), sizeof(uint32_t));
						file.write(reinterpret_cast<const char *>(&s), sizeof(uint32_t));
						file.write(reinterpret_cast<const char *>(&w), sizeof(uint32_t));
						file.write(reinterpret_cast<const char *>(&h), sizeof(uint32_t));
						file.write(data.data(), data.size() * sizeof(char));
						file.close();
					} break;
					case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: {
						std::ofstream file("DXT3.rds", std::ios::binary);
						const auto f = static_cast<uint32_t>(internalFormat);
						const auto s = static_cast<uint32_t>(data.size());
						const auto w = static_cast<uint32_t>(width);
						const auto h = static_cast<uint32_t>(height);
						file.write(reinterpret_cast<const char *>(&f), sizeof(uint32_t));
						file.write(reinterpret_cast<const char *>(&s), sizeof(uint32_t));
						file.write(reinterpret_cast<const char *>(&w), sizeof(uint32_t));
						file.write(reinterpret_cast<const char *>(&h), sizeof(uint32_t));
						file.write(data.data(), data.size() * sizeof(char));
						file.close();
					} break;
					case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: {
						std::ofstream file("DXT5.rds", std::ios::binary);
						const auto f = static_cast<uint32_t>(internalFormat);
						const auto s = static_cast<uint32_t>(data.size());
						const auto w = static_cast<uint32_t>(width);
						const auto h = static_cast<uint32_t>(height);
						file.write(reinterpret_cast<const char *>(&f), sizeof(uint32_t));
						file.write(reinterpret_cast<const char *>(&s), sizeof(uint32_t));
						file.write(reinterpret_cast<const char *>(&w), sizeof(uint32_t));
						file.write(reinterpret_cast<const char *>(&h), sizeof(uint32_t));
						file.write(data.data(), data.size() * sizeof(char));
						file.close();
					} break;
				}

				size_t blockCount = sizes[i] / 16;
				for (size_t j = 0; j < blockCount; ++j) {
					uint8_t *block = reinterpret_cast<uint8_t *>(&data[j * 16]);
					std::swap(block[0], block[1]);
					std::swap(block[8], block[9]);
					std::swap(block[10], block[11]);
				}
				std::vector<char> final_;
				final_.resize(sizes[i]);
				unswizzle((uint8_t *)final_.data(), (uint8_t *)data.data(), width, height,
					(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT == internalFormat) ? 8 : 16);
			*/

				switch (internalFormat) {
				case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: {
					unsigned int len = 0;
					auto ptr = DdzConvert(&len, data.data(), data.size(),
						DDZ_PLATFORM_PC_LE, DDZ_PLATFORM_XBOX_360,
						DDZ_FORMAT_RGBA32, DDZ_FORMAT_DXT1,
						width, height);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
						width, height, 0,
						GL_RGBA, GL_UNSIGNED_BYTE,
						ptr);
					//glCompressedTexImage2D(
					//	GL_TEXTURE_2D,   
					//	0,               
					//	internalFormat, 
					//	width, height,    
					//	0,        
					//	len,  
					//	ptr              
					//);
					GLenum err = glGetError();
					if (0 != err) {
						printf("GL error (DXT1 case): %d\n", err);
					}
				} break;
				case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: {
					unsigned int len = 0;
					auto ptr = DdzConvert(&len, data.data(), data.size(),
						DDZ_PLATFORM_PC_LE, DDZ_PLATFORM_XBOX_360,
						DDZ_FORMAT_RGBA32, DDZ_FORMAT_DXT5,
						width, height);
					std::ofstream dump("converted_texture.raw", std::ios::binary);
					dump.write(reinterpret_cast<char*>(ptr), len);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
						width, height, 0,
						GL_RGBA, GL_UNSIGNED_BYTE,
						ptr);
					//glCompressedTexImage2D(
					//	GL_TEXTURE_2D,   
					//	0,               
					//	internalFormat, 
					//	width, height,    
					//	0,        
					//	len,  
					//	ptr              
					//);
					GLenum err = glGetError();
					if (0 != err) {
						printf("GL error (DXT1 case): %d\n", err);
					}
				} break;
				default: {
					glCompressedTexImage2D(
						GL_TEXTURE_2D,   
						0,               
						internalFormat, 
						width, height,    
						0,        
						sizes[i],  
						data.data()              
					);
					GLenum err = glGetError();
					if (0 != err) {
						printf("GL error (default case): %d\n", err);
					}
				} break;
				}
			}
			else if (exData == EXDATA_XT1) {
				// Bayonetta 3 and possibly other games
				wta.Seek((exOffset + (i * 52)) + 16);
				uint32_t headerSize = wta.ReadUINT32();
				uint32_t mipCount = wta.ReadUINT32();
				uint32_t type = wta.ReadUINT32();
				uint32_t format = wta.ReadUINT32();
				uint32_t width = wta.ReadUINT32();
				uint32_t height = wta.ReadUINT32();
				uint32_t depth = wta.ReadUINT32();
				wta.ReadUINT32();
				uint32_t textureLayout = wta.ReadUINT32();
				uint32_t textureLayout2 = wta.ReadUINT32();

				wtp.Seek(offsets[i]);
				std::vector<char> data = wtp.ReadBytes(sizes[i]);

				

				std::cout << " ";
			}
		}
		if (glTextureID != 0) {
			textureMap[ids[i]] = glTextureID;
		}
		else {
			CTDLog::Log::getInstance().LogError("Failed to load texture!");
		}			

	}		

}
