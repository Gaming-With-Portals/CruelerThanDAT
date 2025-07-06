#pragma once

#ifdef __cplusplus
extern "C" {
#endif

const unsigned int INVPTR = 0xFFFFFFFFFFFFFFFFUL;

typedef enum DdzPlatform : unsigned int {
	DDZ_PLATFORM_PC_LE,
	DDZ_PLATFORM_XBOX_360,
} DdzPlatform;

typedef enum DdzFormat : unsigned int {
	DDZ_FORMAT_RGBA32,
	DDZ_FORMAT_DXT1,
	DDZ_FORMAT_DXT5,
} DdzFormat;

void DdzInit();

void DdzKill();

void *DdzAlloc(const unsigned int);

void DdzFree(void **, const unsigned int);

void *DdzConvert(unsigned int *out_length, const void *in, const unsigned int in_length,
	const DdzPlatform out_platform, const DdzPlatform in_platform,
	const DdzFormat out_format, const DdzFormat in_format,
	const unsigned int width, const unsigned int height);

#ifdef __cplusplus
}
#endif
