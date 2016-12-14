/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2016 Naver Corp
 * Copyright © 2005,2010 Red Hat, Inc
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is Red Hat, Inc.
 *
 * Contributor(s):
 *	Daewoong Jang <daewoong.jang@navercorp.com>
 */

#include "cairoint.h"

#include "cairo-gl.h"
#include "cairo-win32.h"
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2platform.h>
#include <GLES2/gl2ext.h>

SOFT_LINK(libEGL, eglQueryDeviceAttribEXT, EGLBoolean, EGLAPIENTRY, (EGLDeviceEXT device, EGLint attribute, EGLAttrib *value), (device, attribute, value));
SOFT_LINK(libEGL, eglQueryDisplayAttribEXT, EGLBoolean, EGLAPIENTRY, (EGLDisplay dpy, EGLint attribute, EGLAttrib *value), (dpy, attribute, value));

SOFT_LINK(libGLESv2, glQueryTextureAttribEXT, GLboolean, GL_APIENTRY, (GLenum target, GLint attribute, void **value), (target, attribute, value));

cairo_surface_t *
cairo_gl_surface_create (cairo_device_t *device,
			 cairo_content_t content,
			 int width, int height)
{
    return 0;
}

cairo_surface_t *
cairo_gl_surface_create_for_texture (cairo_device_t *abstract_device,
				     cairo_content_t content,
				     unsigned int tex,
                                     int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_USAGE_ANGLE, GL_FRAMEBUFFER_ATTACHMENT_ANGLE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_USAGE_ANGLE, GL_RENDERTARGET_D2D_ANGLE);

    struct ID3D11Texture2D* texture = 0;
    if (!glQueryTextureAttribEXT(GL_TEXTURE_2D, GL_TEXTURE_D3D11_ANGLE, &texture))
        return 0;

    return cairo_d2d_surface_create_for_texture(abstract_device, texture, content);
}

void
cairo_gl_surface_set_size (cairo_surface_t *surface, int width, int height)
{
}

int
cairo_gl_surface_get_width (cairo_surface_t *abstract_surface)
{
    return 0;
}

int
cairo_gl_surface_get_height (cairo_surface_t *abstract_surface)
{
    return 0;
}

void
cairo_gl_surface_swapbuffers (cairo_surface_t *surface)
{
}

void
cairo_gl_device_set_thread_aware (cairo_device_t	*device,
				  cairo_bool_t		 thread_aware)
{
}

#if CAIRO_HAS_GLX_FUNCTIONS
#include <GL/glx.h>

cairo_device_t *
cairo_glx_device_create (Display *dpy, GLXContext gl_ctx)
{
    return 0;
}

Display *
cairo_glx_device_get_display (cairo_device_t *device)
{
    return 0;
}

GLXContext
cairo_glx_device_get_context (cairo_device_t *device)
{
    return 0;
}

cairo_surface_t *
cairo_gl_surface_create_for_window (cairo_device_t *device,
				    Window win,
				    int width, int height)
{
    return 0;
}
#endif

#if CAIRO_HAS_WGL_FUNCTIONS
#include <windows.h>

cairo_device_t *
cairo_wgl_device_create (HGLRC rc)
{
    return 0;
}

HGLRC
cairo_wgl_device_get_context (cairo_device_t *device)
{
    return 0;
}

cairo_surface_t *
cairo_gl_surface_create_for_dc (cairo_device_t		*device,
				HDC			 dc,
				int			 width,
				int			 height)
{
    return 0;
}
#endif

#if CAIRO_HAS_EGL_FUNCTIONS
#include <EGL/egl.h>

cairo_device_t *
cairo_egl_device_create (EGLDisplay dpy, EGLContext egl)
{
    EGLDeviceEXT device = 0;
    if (!eglQueryDisplayAttribEXT(dpy, EGL_DEVICE_EXT, (EGLAttrib *)&device))
        return 0;

    EGLAttrib d3d11Device = 0;
    if (!eglQueryDeviceAttribEXT(device, EGL_D3D11_DEVICE_ANGLE, &d3d11Device))
        return 0;

    return cairo_d2d_create_device_from_d3d11device((struct ID3D11Device *)d3d11Device);
}

cairo_surface_t *
cairo_gl_surface_create_for_egl (cairo_device_t	*device,
				 EGLSurface	 egl,
				 int		 width,
				 int		 height)
{
    return 0;
}
#endif
