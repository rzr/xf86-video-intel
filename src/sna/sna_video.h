/***************************************************************************

Copyright 2000 Intel Corporation.  All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL INTEL, AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifndef SNA_VIDEO_H
#define SNA_VIDEO_H

#include <xf86_OSproc.h>
#include <xf86xv.h>
#include <fourcc.h>

#if defined(XvMCExtension) && defined(ENABLE_XVMC)
#define SNA_XVMC 1
#endif

/*
 * Below, a dummy picture type that is used in XvPutImage
 * only to do an overlay update.
 * Introduced for the XvMC client lib.
 * Defined to have a zero data size.
 */
#define XVMC_YUV { \
	FOURCC_XVMC, XvYUV, LSBFirst, \
	{'X', 'V', 'M', 'C', 0x00, 0x00, 0x00, 0x10, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}, \
	12, XvPlanar, 3, 0, 0, 0, 0, 8, 8, 8, 1, 2, 2, 1, 2, 2, \
	{'Y', 'V', 'U', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, \
	XvTopToBottom \
}

struct sna_video {
	struct sna *sna;

	int brightness;
	int contrast;
	int saturation;
	xf86CrtcPtr desired_crtc;

	RegionRec clip;

	uint32_t gamma0;
	uint32_t gamma1;
	uint32_t gamma2;
	uint32_t gamma3;
	uint32_t gamma4;
	uint32_t gamma5;

	int color_key;
	int color_key_changed;

	/** YUV data buffers */
	struct kgem_bo *old_buf[2];
	struct kgem_bo *buf;

	int alignment;
	bool tiled;
	bool textured;
	Rotation rotation;
	int plane;

	int SyncToVblank;	/* -1: auto, 0: off, 1: on */
};

struct sna_video_frame {
	struct kgem_bo *bo;
	uint32_t id;
	uint32_t size;
	uint32_t UBufOffset;
	uint32_t VBufOffset;

	uint16_t width, height;
	uint16_t pitch[2];

	/* extents */
	BoxRec image;
	BoxRec src;
};

static inline XvScreenPtr to_xv(ScreenPtr screen)
{
	return dixLookupPrivate(&screen->devPrivates, XvGetScreenKey());
}

void sna_video_init(struct sna *sna, ScreenPtr screen);
void sna_video_overlay_setup(struct sna *sna, ScreenPtr screen);
void sna_video_sprite_setup(struct sna *sna, ScreenPtr screen);
void sna_video_textured_setup(struct sna *sna, ScreenPtr screen);
void sna_video_destroy_window(WindowPtr win);

XvAdaptorPtr sna_xv_adaptor_alloc(struct sna *sna);
int sna_xv_fixup_formats(ScreenPtr screen,
			 XvFormatPtr formats,
			 int num_formats);
int sna_xv_alloc_port(unsigned long port, XvPortPtr in, XvPortPtr *out);
int sna_xv_free_port(XvPortPtr port);

#define FOURCC_XVMC     (('C' << 24) + ('M' << 16) + ('V' << 8) + 'X')

static inline int xvmc_passthrough(int id)
{
	return id == FOURCC_XVMC;
}

static inline int is_planar_fourcc(int id)
{
	switch (id) {
	case FOURCC_YV12:
	case FOURCC_I420:
	case FOURCC_XVMC:
		return 1;
	case FOURCC_UYVY:
	case FOURCC_YUY2:
	default:
		return 0;
	}
}

bool
sna_video_clip_helper(ScrnInfoPtr scrn,
		      struct sna_video *adaptor_priv,
		      struct sna_video_frame *frame,
		      xf86CrtcPtr * crtc_ret,
		      BoxPtr dst,
		      short src_x, short src_y,
		      short drw_x, short drw_y,
		      short src_w, short src_h,
		      short drw_w, short drw_h,
		      RegionPtr reg);

void
sna_video_frame_init(struct sna_video *video,
		     int id, short width, short height,
		     struct sna_video_frame *frame);

struct kgem_bo *
sna_video_buffer(struct sna_video *video,
		 struct sna_video_frame *frame);

bool
sna_video_copy_data(struct sna_video *video,
		    struct sna_video_frame *frame,
		    const uint8_t *buf);

void sna_video_buffer_fini(struct sna_video *video);

void sna_video_free_buffers(struct sna_video *video);

static inline XvPortPtr
sna_window_get_port(WindowPtr window)
{
	return ((void **)__get_private(window, sna_window_key))[2];
}

static inline void
sna_window_set_port(WindowPtr window, XvPortPtr port)
{
	((void **)__get_private(window, sna_window_key))[2] = port;
}

#endif /* SNA_VIDEO_H */
