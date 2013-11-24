/*
 * iFrameBufferObject.h
 *
 *  Created on: 19/11/2013
 *      Author: WhatIsXO
 */

#ifndef HPL_IFRAMEBUFFEROBJECT_H_
#define HPL_IFRAMEBUFFEROBJECT_H_

namespace hpl {

	class iTexture;

	class iFrameBufferObject {
	public:
		virtual ~iFrameBufferObject() {};

		virtual bool Init(int alWidth, int alHeight)=0;
		virtual void Bind()=0;
		virtual bool AttachColorBuffer(unsigned int alAttachmentPoint, iTexture* colourBufferTexture)=0;
};

} /* namespace hpl */
#endif /* HPL_IFRAMEBUFFEROBJECT_H_ */
