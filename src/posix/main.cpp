#include <cairo.h>
#include<cairo-xlib.h>
#include<X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../framebuffer.h"
#include "../program.h"
#include <pthread.h>

bool requestExit = false;

struct render_context
{
    pthread_mutex_t mutex;
    cairo_surface_t * surface;
    cairo_t * cairoContext;
    ProgramInterface * program;
};

void * processThread(void * ptr)
{
    render_context * ctx = reinterpret_cast<render_context*>(ptr);
    while (!requestExit)
    {
        usleep( 16666/*60 fps*/ );
        pthread_mutex_lock(&ctx->mutex);
        ctx->program->Render();
        cairo_set_source_surface(ctx->cairoContext,ctx->surface,0,0); 
		cairo_paint(ctx->cairoContext);
        pthread_mutex_unlock(&ctx->mutex);
    }
    return 0;
    
}

void start_process_thread(pthread_t * processThreadHandle, render_context * ctx)
{
    pthread_create(processThreadHandle, NULL, processThread, ctx); 
}

int main (int argc, char *argv[])
{
    pthread_t processThreadHandle;
    ProgramInterface * p = ProgramInterface::create();
    p->OnStart();
	Display *dpy;
	Window rootwin;
	Window win;
	XEvent e;
	int scr;
	cairo_surface_t *cs;

	if(!(dpy=XOpenDisplay(NULL))) {
		fprintf(stderr, "ERROR: Could not open display\n");
		exit(1);
	}

	scr=DefaultScreen(dpy);
	rootwin=RootWindow(dpy, scr);

	win=XCreateSimpleWindow(dpy, rootwin, 1, 1, p->GetFrame()->GetW(), p->GetFrame()->GetH(), 0, BlackPixel(dpy, scr), BlackPixel(dpy, scr));

	XStoreName(dpy, win, p->GetName());
	XSelectInput(dpy, win, ExposureMask|ButtonPressMask|CWEventMask);
	XMapWindow(dpy, win);

    cs=cairo_xlib_surface_create(dpy, win, DefaultVisual(dpy, 0), p->GetFrame()->GetW(), p->GetFrame()->GetH());

	Atom wmDelete=XInternAtom(dpy, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(dpy, win, &wmDelete, 1);
    cairo_t *cr = cairo_create (cs);
	cairo_surface_t * imageSurface = cairo_image_surface_create_for_data(
            reinterpret_cast<unsigned char *>(p->GetFrame()->GetBuffer()), CAIRO_FORMAT_ARGB32, p->GetFrame()->GetW() , p->GetFrame()->GetH(), cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, p->GetFrame()->GetW()));
    render_context renderContext; 
    renderContext.surface = imageSurface;
    renderContext.cairoContext = cr;
    renderContext.program = p;
    pthread_mutex_init(&renderContext.mutex, NULL);
	bool started = false;	
	while(!requestExit) {
		XNextEvent(dpy, &e);
        pthread_mutex_lock(&renderContext.mutex);
		if(e.type==Expose && e.xexpose.count<1) {
            if (!started) {
                start_process_thread(&processThreadHandle, &renderContext);
                started = true;
            }
		} else if(e.type==ButtonPress) {

        } else if (e.type == ClientMessage) {
            requestExit = true;
		}
        pthread_mutex_unlock(&renderContext.mutex);
	}
    pthread_join(processThreadHandle, NULL);
    pthread_mutex_destroy(&renderContext.mutex);
    cairo_surface_destroy(imageSurface);
	cairo_surface_destroy(cs);
	XCloseDisplay(dpy);
    p->OnEnd();
    delete p;
    return 0;
}
