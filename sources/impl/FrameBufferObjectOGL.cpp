/*
 * cFrameBufferObject.cpp
 *
 *  Created on: 15/11/2013
 *      Author: WhatIsXO
 */

#include "impl/FrameBufferObjectOGL.h"
#include "impl/LowLevelGraphicsSDL.h"
#include "system/LowLevelSystem.h"

namespace hpl {

	cFrameBufferObjectOGL::cFrameBufferObjectOGL(iLowLevelGraphics* apLowLevelGraphics)
	{
	}

	cFrameBufferObjectOGL::~cFrameBufferObjectOGL()
	{
		glDeleteFramebuffers(1, &mlFrameBufferHandle);
	}

	bool cFrameBufferObjectOGL::Init(int alWidth, int alHeight)
	{
		// Create and bind frame buffer
		glGenFramebuffers(1, &mlFrameBufferHandle);
		glBindFramebuffer(GL_FRAMEBUFFER, mlFrameBufferHandle);
		GLenum foo = glGetError();
		if (foo)
		{
			Warning("GL_Error on frame buffer object init");
			return false;
		}

		// Create and bind depth&stencil render buffer
		glGenRenderbuffers(1, &mlDepthStencilHandle);
		glBindRenderbuffer(GL_RENDERBUFFER, mlDepthStencilHandle);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, alWidth, alHeight);

		// Attach depth&stencil to frame buffer at depth and stencil attachment points
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mlDepthStencilHandle);

		// Check no errors, frame buffer object not yet complete though - needs colour
		foo = glGetError();
		if (foo)
		{
			Warning("GL_Error on frame buffer object init");
			return false;
		}

		return true;
	}

	void cFrameBufferObjectOGL::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mlFrameBufferHandle);
	}

	bool cFrameBufferObjectOGL::AttachColorBuffer(unsigned int alAttachmentPoint, iTexture* colourBufferTexture)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, mlFrameBufferHandle);

		// Attach texture to colour attachment point
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + alAttachmentPoint, GL_TEXTURE_2D, (GLuint)colourBufferTexture->GetHandle(), 0);

        // Frame buffer object should now be considered complete - if not check texture initialisation
        return CheckFrameBufferComplete();
	}

	bool cFrameBufferObjectOGL::CheckFrameBufferComplete()
	{
		// Verify Frame Buffer Object is complete - texture, depth
		glBindFramebuffer(GL_FRAMEBUFFER, mlFrameBufferHandle);

		GLenum status;
		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		bool initSuccess = false;
		switch(status)
		{
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				{
					Warning("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT is returned if the framebuffer does not have at least one image attached to it.");
					break;
				}
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			{
					Warning("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT is returned if any of the framebuffer attachment points are framebuffer incomplete.");
					break;
				}
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				{
					Warning("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER​ is returned if the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE​ is GL_NONE​ for any color attachment point(s) named by GL_DRAW_BUFFERi​.");
					break;
				}
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				{
					Warning("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER​ is returned if GL_READ_BUFFER​ is not GL_NONE​ and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE​ is GL_NONE​ for the color attachment point named by GL_READ_BUFFER​.");
					break;
				}
			case GL_FRAMEBUFFER_UNSUPPORTED:
				{
					Warning("GL_FRAMEBUFFER_UNSUPPORTED​ is returned if the combination of internal formats of the attached images violates an implementation-dependent set of restrictions.");
					break;
				}
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				{
					Warning("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE​ is returned if the value of GL_RENDERBUFFER_SAMPLES​ is not the same for all attached renderbuffers; if the value of GL_TEXTURE_SAMPLES​ is the not same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of GL_RENDERBUFFER_SAMPLES​ does not match the value of GL_TEXTURE_SAMPLES​.");
					break;
				}
			case GL_FRAMEBUFFER_COMPLETE:
				{
					initSuccess = true;
					break;
				}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return initSuccess;
	}


} /* namespace hpl */
