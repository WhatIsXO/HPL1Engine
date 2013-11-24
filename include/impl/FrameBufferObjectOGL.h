/*
 * cFrameBufferObject.h
 *
 *  Created on: 15/11/2013
 *      Author: WhatIsXO
 */

#ifndef HPL_CFRAMEBUFFEROBJECT_H_
#define HPL_CFRAMEBUFFEROBJECT_H_

#include <GL/GLee.h>
#include <vector>

#include "graphics/GraphicsTypes.h"
#include "graphics/FrameBufferObject.h"

namespace hpl
{
	class iLowLevelGraphics;

	class cFrameBufferObjectOGL : public iFrameBufferObject
	{
	public:
		cFrameBufferObjectOGL(iLowLevelGraphics* apLowLevelGraphics);
		~cFrameBufferObjectOGL();

		bool Init(int alWidth, int alHeight);
		void Bind();
		bool AttachColorBuffer(unsigned int alAttachmentPoint, iTexture* colourBufferTexture);

	private:
		bool CheckFrameBufferComplete();

		bool mbShareObjects;
		iLowLevelGraphics* mpLowLevelGraphics;
		GLuint mlFrameBufferHandle;
		GLuint mlDepthStencilHandle;
	};
}
#endif /* HPL_CFRAMEBUFFEROBJECT_H_ */
