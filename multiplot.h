#pragma once
/*
 * multiplot.h
 * Copyright 2002-2019 by Andre Frank Krause.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "post@andre-krause.net".
 * New versions and bug-fixes under http://www.andre-krause.net
 *	
 */

 /*
INSTALL
	if you want to try multiplot, a simple hello world program:
	
	main.cpp:

	1	int main(int argc, char**argv){ cout<<"hello world";return 1;}


	if that works, remove the above line and type (remove the line numbers):

	1	#define __TEST_THIS_MODULE__
	2	#include "multiplot.h"
	3   int main(int argc, char ** argv) { test_module(); }

	and compile that. this gives you some demos on what you can do with multiplot (see end 
	of that file for the demo - code)
	- Hints:	
		* Linux:
		  try the following line:
			gcc -lfltk -lfltk_gl -o testplot testplots.cpp
		* Windows:
			visual studio:
				1. start visual studio. create a new project (file->new->project..). 
				2. select: win32 -> win32 console application
				3. press ok, then select "application settings", there choos "empty project"
				4. add a new c++ file in the solution explorer and type the above 
			devcpp:
				there seems to be a compiler bug with rtti, so 
				1. disable  rtti with the c++ compiler flag "-fno-rtti"  (project options->parameters)
				2. include the following libraries for opengl: 
				-lglu32 -lglaux -lgdi32 -lglu32 -lopengl32 -lfltk_gl -lfltk -lole32 -luuid -lcomctl32 -lwsock32 -lm
				  

CHANGELOG
	20190210 - v 0.5.5:
	- added namespace multiplot
	- small bugfixes
	20160608 - v 0.5:
	- several small c++ 'modernizations' of the code
	- title can now be a wide string (std::wstring). if you prefer a normal string s, convert it using "wstring(s.begin(), s.end())"
	20090910:
	- added plotting of values stored in a std::vector - plot(const std::vector<T>& v)
	20090621:
	- added different scaling behaviours (auto, auto with equally scaled axes, fixed)
	20061031:
	- redesigned interface. parameters like current drawing color and current trace
	  can be set in an opengl-manner ( example: first, select a trace with m.trace(1),
	  the set the color of that trace with m.color3f(r,g,b) )
	  OR by accessing the wanted trace with m[1].color3f(r,g,b); m[1].plot(x,y)
	- now, you can set for each individual plot point the pointsize and line-width.
	- decoupled from FLTK - library. under win32 you no longer need to have FLTK. just load
	  multiplot.h, create an Instance Multiplot myplot(10,10,300,300) and you will get a nice
	  window at 10,10 with size 300.

	20050304:
	compiling under devc++ (there seems to be a compiler bug with rtti)
	1. disable  rtti with the c++ compiler flag "-fno-rtti"  (project options->parameters)
	2. include the following libraries for opengl: 
	-lglu32 -lglaux -lgdi32 -lglu32 -lopengl32 -lfltk_gl -lfltk -lole32 -luuid -lcomctl32 -lwsock32 -lm

	20041139
	- Andre Krause <post@andre-krause.net>
	- updated visual studio project files

	- Thorsten Roggendorf / Sven Hartmeier
	- Linux bugfixes
	- added a simple build script instead of automake and configure

	20021018
	- Andre Krause <post@andre-krause.net>
	- now you can specify the color of the background and the grid-lines
	- you can define the line-width of the traces and grid-lines
	- you can plot points in combination with lines in your traces, or points only (scatter-plot)

	20021003 
	- Thorsten Roggendorf
	- linux configure - script 
	- Andre Krause <post@andre-krause.net>
	- moved implementation of member functions back to the header file as inline functions to simplify usage. now you simply need to include "multiplot.h".
	- autoscaling grid (linear spacing only at the moment, log spacing will follow)
	20020729 
	- Andre Krause <post@andre-krause.net>
	- initial release
*/


#if defined(MULTIPLOT_FLTK) || defined(MULTIPLOT_WIN32) 
#else
	#ifdef Fl_Object
		#define MULTIPLOT_FLTK
	#else
		#define MULTIPLOT_WIN32		
		//#pragma(warning, "Please define either MULTIPLOT_FLTK or MULTIPLOT_WIN32 before including multiplot.h .")
	#endif
#endif


#include <iostream>
#include <sstream>
#include <float.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>
#include <locale>	// to convert wstring to string
#include <codecvt>	// to convert wstring to string


#ifdef MULTIPLOT_FLTK // tell multiplot to use Fltk to create an Opengl-Window
	#include <FL/gl.h>
	#include <FL/Fl.H>
	#include <FL/Fl_Gl_Window.H>
	#include <FL/fl_draw.H>
#endif



#if defined(_WIN32) && !defined(APIENTRY)
	#define WIN32_LEAN_AND_MEAN 1
	#include <windows.h>
#endif

#ifndef MULTIPLOT_FLTK  // don't include default gl.h, if using fltk, because fltk comes with its own gl.h
	#include <GL/gl.h> 
#endif


namespace multiplot
{

/**
* These are the available Grid Styles.
*/
enum MP_GRIDSTYLE
{
	MP_NO_GRID,
	MP_LINEAR_GRID,
	MP_LOG_GRID
};

enum MP_SCALING
{
	MP_AUTO_SCALE,
	MP_AUTO_SCALE_EQUAL,
	MP_FIXED_SCALE
};



//////////////////////////////////////////////////////////////////////////
// now comes platform specific code for opening a window to draw in

#ifdef MULTIPLOT_FLTK

class Multiplot_base : public Fl_Gl_Window
{
protected:
	unsigned int width = 0;
	unsigned int height = 0;
	std::string caption;
public:
	/**
	*	this constructor tells multiplot where to put the window on the 
	*	desktop in pixel-coordinates(x,y) and with wich width and height (w,h)
	*/
	Multiplot_base(int x, int y, int w, int h, const std::wstring& title_, bool fullscreen_) : Fl_Gl_Window(x,y,w,h) 
	  {
		  width = w;
		  height = h;
		  resizable(*this);
		  set_caption(title_);
		  if(fullscreen_)
			  fullscreen();
	  }

	  bool check()
	  {
		  return (bool)Fl::check();
	  }

	  void draw()
	  {
		  if(!valid())
		  {
			  valid(1);
		  }


		  Fl_Gl_Window::draw();


	  }

	  void set_caption(const std::string& t)
	  {
		  caption = t;
		  this->label(caption.c_str());
	  }
	  void set_caption(const std::wstring& t)
	  {
		  //setup converter
		  using convert_type = std::codecvt_utf8<wchar_t>;
		  caption = std::wstring_convert<convert_type, wchar_t>().to_bytes(t);
		  this->label(caption.c_str());
	  }

}; 
#endif

#ifdef MULTIPLOT_WIN32

#ifdef _WIN32
#pragma message("     _Adding library: opengl32.lib" ) 
#pragma comment ( lib, "opengl32.lib")
#endif

/**
* class Multiplot_base is for low level Window handling
* and creates an OpenGL Context.
*/
class Multiplot_base
{
protected:

	unsigned int width; // window - width
	unsigned int height; // window - height
	bool valid_;

	bool active;
	bool fullscreen;
	HDC			hDC;		// Private GDI Device Context
	HGLRC		hRC;		// Permanent Rendering Context
	HWND		hWnd;		// Holds Our Window Handle
	HINSTANCE	hInstance;	// Holds The Instance Of The Application

	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Multiplot_base* pParent;

		// Get pointer to window
		if(uMsg == WM_CREATE)
		{
			pParent = (Multiplot_base*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA,(LONG_PTR)pParent);
		}
		else
		{
			pParent = (Multiplot_base*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if(!pParent) return DefWindowProc(hWnd,uMsg,wParam,lParam);
		}

		pParent->hWnd = hWnd;
		return pParent->WndProc(uMsg,wParam,lParam);
	}

	LRESULT WndProc(				// Handle For This Window
		UINT	uMsg,			// Message For This Window
		WPARAM	wParam,			// Additional Message Information
		LPARAM	lParam)			// Additional Message Information
	{

		switch (uMsg)									// Check For Windows Messages
		{
		case WM_ACTIVATE:							// Watch For Window Activate Message
			{
				if (!HIWORD(wParam))					// Check Minimization State
				{
					active=TRUE;						// Program Is Active
				}
				else
				{
					active=FALSE;						// Program Is No Longer Active
				}

				return 0;								// Return To The Message Loop
			}

		case WM_SYSCOMMAND:							// Intercept System Commands
			{
				switch (wParam)							// Check System Calls
				{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
					return 0;							// Prevent From Happening
				}
				break;									// Exit
			}

		case WM_CLOSE:								// Did We Receive A Close Message?
			{
				PostQuitMessage(0);						// Send A Quit Message
				return 0;								// Jump Back
			}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
			{
				//keys[wParam] = TRUE;					// If So, Mark It As TRUE
				return 0;								// Jump Back
			}

		case WM_KEYUP:								// Has A Key Been Released?
			{
				//keys[wParam] = FALSE;				// If So, Mark It As FALSE
				return 0;							// Jump Back
			}

		case WM_SIZE:								// Resize The OpenGL Window
			{
				width = LOWORD(lParam);
				height = HIWORD(lParam);
				valid_ = false;						// set flag to tell we need to re-init opengl
				return 0;							// Jump Back
			}
		}

		// Pass All Unhandled Messages To DefWindowProc
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}





	/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
	*	width			- Width Of The GL Window Or Fullscreen Mode				*
	*	height			- Height Of The GL Window Or Fullscreen Mode			*
	*	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
	*/ 
	bool CreateGLWindow(int x, int y, int width, int height, const std::wstring& title, BYTE bits=0, bool fullscreenflag=false)
	{
		GLuint		PixelFormat;			// Holds The Results After Searching For A Match
		WNDCLASS	wc;						// Windows Class Structure
		DWORD		dwExStyle;				// Window Extended Style
		DWORD		dwStyle;				// Window Style
		RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
		WindowRect.left=(long)0;			// Set Left Value To 0
		WindowRect.right=(long)width;		// Set Right Value To Requested Width
		WindowRect.top=(long)0;				// Set Top Value To 0
		WindowRect.bottom=(long)height;		// Set Bottom Value To Requested Height

		fullscreen=fullscreenflag;			// Set The Global Fullscreen Flag

		hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
		wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
		wc.lpfnWndProc		= (WNDPROC) (Multiplot_base::StaticWndProc);// window_handler Handles Messages
		wc.cbClsExtra		= 0;									// No Extra Window Data
		wc.cbWndExtra		= 0;									// No Extra Window Data
		wc.hInstance		= hInstance;							// Set The Instance
		wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
		wc.hbrBackground	= NULL;									// No Background Required For GL
		wc.lpszMenuName		= NULL;									// We Don't Want A Menu
		wc.lpszClassName	= L"OpenGL";								// Set The Class Name


		if (!RegisterClass(&wc))									// Attempt To Register The Window Class
		{
			throw(std::exception("Failed To Register The Window Class."));
		}

		if (fullscreen)												// Attempt Fullscreen Mode?
		{
			DEVMODE dmScreenSettings;								// Device Mode
			memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
			dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
			dmScreenSettings.dmPelsWidth	= width;				// Selected Screen Width
			dmScreenSettings.dmPelsHeight	= height;				// Selected Screen Height
			dmScreenSettings.dmBitsPerPel	= bits;					// Selected Bits Per Pixel
			dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

			// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
			if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
			{
				// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
				std::cerr << "The Requested Fullscreen Mode is not supported by your Video Card. Using Windowed Mode instead." << std::endl;
				fullscreen=false;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
		}

		if (fullscreen)												// Are We Still In Fullscreen Mode?
		{
			dwExStyle=WS_EX_APPWINDOW;								// Window Extended Style
			dwStyle=WS_POPUP;										// Windows Style
			ShowCursor(FALSE);										// Hide Mouse Pointer
		}
		else
		{
			dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
			dwStyle=WS_OVERLAPPEDWINDOW;							// Windows Style
		}

		AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

		// Create The Window
		if (!(hWnd=CreateWindowEx(	dwExStyle,	// Extended Style For The Window
			L"OpenGL",							// Class Name
			title.c_str(),							// Window Title
			dwStyle |							// Defined Window Style
			WS_CLIPSIBLINGS |					// Required Window Style
			WS_CLIPCHILDREN,					// Required Window Style
			x, y,								// Window Position
			WindowRect.right-WindowRect.left,	// Calculate Window Width
			WindowRect.bottom-WindowRect.top,	// Calculate Window Height
			NULL,								// No Parent Window
			NULL,								// No Menu
			hInstance,							// Instance
			this)))								// Dont Pass Anything To WM_CREATE
		{
			//KillGLWindow();								// Reset The Display
			throw(std::exception("Window Creation Error."));
		}

		static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
		{
			sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
			1,											// Version Number
			PFD_DRAW_TO_WINDOW |						// Format Must Support Window
			PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
			PFD_DOUBLEBUFFER,							// Must Support Double Buffering
			PFD_TYPE_RGBA,								// Request An RGBA Format
			bits,										// Select Our Color Depth
			0, 0, 0, 0, 0, 0,							// Color Bits Ignored
			0,											// No Alpha Buffer
			0,											// Shift Bit Ignored
			0,											// No Accumulation Buffer
			0, 0, 0, 0,									// Accumulation Bits Ignored
			16,											// 16Bit Z-Buffer (Depth Buffer)  
			0,											// No Stencil Buffer
			0,											// No Auxiliary Buffer
			PFD_MAIN_PLANE,								// Main Drawing Layer
			0,											// Reserved
			0, 0, 0										// Layer Masks Ignored
		};

		if (!(hDC=GetDC(hWnd)))							// Did We Get A Device Context?
		{
			//KillGLWindow();								// Reset The Display
			throw(std::exception("Can't Create A GL Device Context."));
		}

		if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
		{
			//KillGLWindow();								// Reset The Display
			throw(std::exception("Can't Find A Suitable PixelFormat."));
		}

		if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
		{
			//KillGLWindow();								// Reset The Display
			throw(std::exception("Can't Set The PixelFormat."));
		}

		if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
		{
			throw(std::exception("Can't Create A GL Rendering Context."));
		}

		if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
		{
			throw(std::exception("Can't Activate The GL Rendering Context."));
		}

		ShowWindow(hWnd,SW_SHOW);						// Show The Window
		SetForegroundWindow(hWnd);						// Slightly Higher Priority
		SetFocus(hWnd);									// Sets Keyboard Focus To The Window
		//ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

		/*if (!InitGL())									// Initialize Our Newly Created GL Window
		{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
		}*/

		return true;									// Success
	}

	void DestroyGLWindow()
	{
		// destroy the window and rendering context
		if (fullscreen)										// Are We In Fullscreen Mode?
		{
			ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
			ShowCursor(TRUE);								// Show Mouse Pointer
		}

		if (hRC)											// Do We Have A Rendering Context?
		{
			if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
			{
				throw(std::exception("Release Of DC And RC Failed."));
			}

			if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
			{
				throw(std::exception("Release Rendering Context Failed."));
			}
			hRC=NULL;										// Set RC To NULL
		}

		if (hDC && !ReleaseDC(hWnd,hDC))					// Are We Able To Release The DC
		{
			throw(std::exception("Release Device Context Failed."));
			hDC=NULL;										// Set DC To NULL
		}

		if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
		{
			throw(std::exception("Could Not Release hWnd."));
			hWnd=NULL;										// Set hWnd To NULL
		}

		if (!UnregisterClass(L"OpenGL",hInstance))			// Are We Able To Unregister Class
		{
			throw(std::exception("Could Not Unregister Class."));
			hInstance=NULL;									// Set hInstance To NULL
		}
	}

	static LRESULT CALLBACK window_handler(	HWND	hWnd,			// Handle For This Window
		UINT	uMsg,			// Message For This Window
		WPARAM	wParam,			// Additional Message Information
		LPARAM	lParam)			// Additional Message Information
	{
		switch (uMsg)									// Check For Windows Messages
		{
		case WM_ACTIVATE:							// Watch For Window Activate Message
			{
				if (!HIWORD(wParam))					// Check Minimization State
				{
					//active=TRUE;						// Program Is Active
				}
				else
				{
					//active=FALSE;						// Program Is No Longer Active
				}

				return 0;								// Return To The Message Loop
			}

		case WM_SYSCOMMAND:							// Intercept System Commands
			{
				switch (wParam)							// Check System Calls
				{
				case SC_SCREENSAVE:					// Screensaver Trying To Start?
				case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
					return 0;							// Prevent From Happening
				}
				break;									// Exit
			}

		case WM_CLOSE:								// Did We Receive A Close Message?
			{
				PostQuitMessage(0);						// Send A Quit Message
				return 0;								// Jump Back
			}

		case WM_KEYDOWN:							// Is A Key Being Held Down?
			{
				//keys[wParam] = TRUE;					// If So, Mark It As TRUE
				return 0;								// Jump Back
			}

		case WM_KEYUP:								// Has A Key Been Released?
			{
				//keys[wParam] = FALSE;					// If So, Mark It As FALSE
				return 0;								// Jump Back
			}

		case WM_SIZE:								// Resize The OpenGL Window
			{
				LPARAM x = lParam;
				int xx = LOWORD(lParam);
				//((Multiplot*)x)->width =x;
				// todo... problem with member function pointer
				/*((Multiplot*)wParam)->width = LOWORD(lParam); // LoWord=Width, HiWord=Height
				((Multiplot*)wParam)->height = HIWORD(lParam);
				((Multiplot*)wParam)->initgl();*/
				return 0;								// Jump Back
			}
		}

		// Pass All Unhandled Messages To DefWindowProc
		return DefWindowProc(hWnd,uMsg,wParam,lParam);
	}


	
public:
	virtual ~Multiplot_base(){ DestroyGLWindow(); }

	/**
	*	this constructor tells multiplot where to put the window on the 
	*	desktop in pixel-coordinates(x,y) and with wich width and height (w,h)
	*/
	Multiplot_base(int x, int y, int w, int h, const std::wstring& ttitle, bool fullscreen)
	{ 
		hDC=NULL;
		CreateGLWindow(x, y, w, h, ttitle, 32, fullscreen);
	}

	/**
	*	call show() to make the window visible
	*	only needed if using FLTK as window-creation backend.
	*/
	void show()
	{
	}

	bool check()
	{
		MSG msg;
		bool ok=true;

		if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			ok=false;

		// now, check any messages, so win32 is happy
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))//GetMessage( &msg, NULL, 0, 0 ) )
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
		//else
		//	ok=false;

		return ok;
	}

	unsigned int w() { return width; }
	unsigned int h() { return height; }
	bool valid() { return valid_; }
	void valid(bool v) { valid_=v; }

	virtual void draw()
	{
	}

	void set_caption(const std::wstring& t)
	{
		// TODO
	}

	/**
	* call redraw to refresh the window and to redraw all traces.
	*/
	void redraw() 
	{
		draw(); 
		check();
		SwapBuffers(hDC);
	}
};

#endif

/**
 *	this class creates a window to wich you can add an arbitrary number of 
 *	autoscaling traces.
 */
class Multiplot : public Multiplot_base
{
protected:
	class Point2d
	{
	public:
		float x = 0.0f;
		float y = 0.0f;
		float r = 1.0f;
		float g = 1.0f;
		float b = 1.0f;
		float point_size = 0.0f;
		float line_width = 1.0f; // width of the line from this point to the next point
		
		Point2d() {}
		Point2d(float xx, float yy, float rr=1, float gg=1, float bb=1, float _lwidth=1.0, float _point_size=0.0)
		{
			x=xx;y=yy;
			r=rr;g=gg;b=bb;
			point_size=_point_size;
			line_width=_lwidth;
		}
	};

public:
		// class Trace describes a single trace
		// if scroll=true it works as a ringbuffer.
		// the ringbuffer is filled till max_points.
		// pos stores the actual read/write position in the ringbuffer

		/**
		* class Trace describes a single Trace. A Multiplot-Window can
		* contain an unlimited number of Traces.
		*/
		class Trace : public std::vector<Point2d>
		{
		public:
			unsigned int max_points;
			bool scroll;
			unsigned int pos; // current position in the ringbuffer
			float cur_col[3];
			float cur_line_width;
			float cur_point_size;
			void draw(Point2d& minimum, Point2d& maximum, Point2d& scale, Point2d& offset)
			{
				if(size()==0)return;

				Point2d p,p1,p2;
				int start=0;
				if(scroll)
					start = pos;

				float line_width=at(start%size()).line_width;
				glLineWidth(line_width);
				glBegin(GL_LINES);
				for(unsigned int a=start;a<start+size()-1;a++)
				{
					//if(ps>=size())
					//	ps=0;
					p1=(*this)[a % size()];
					p2=(*this)[(a+1) % size()];

					// TODO: if distance between two points is smaller than a pixel, then 
					// skip and interpolate till we have at least a line with length=1pixel

					if(p1.line_width>0)
					{

						// reduce number of opengl state changes. so begin a new GL_LINES block
						// only if line_width has changed.
						if(p1.line_width != line_width)
						{
							glEnd();
							line_width = p1.line_width;
							glLineWidth(line_width);
							glBegin(GL_LINES);
						}
						glColor3f(p1.r,p1.g,p1.b);
						glVertex2f((p1.x-offset.x)*scale.x,(p1.y-offset.y)*scale.y);
						glColor3f(p2.r,p2.g,p2.b);
						glVertex2f((p2.x-offset.x)*scale.x,(p2.y-offset.y)*scale.y);
					}
					if(p1.x>maximum.x)maximum.x=p1.x;
					if(p1.x<minimum.x)minimum.x=p1.x;
					if(p1.y>maximum.y)maximum.y=p1.y;
					if(p1.y<minimum.y)minimum.y=p1.y;
				}
				glEnd();
				if(p2.x>maximum.x)maximum.x=p2.x;
				if(p2.x<minimum.x)minimum.x=p2.x;
				if(p2.y>maximum.y)maximum.y=p2.y;
				if(p2.y<minimum.y)minimum.y=p2.y;


				float point_size=at(start%size()).line_width;
				glPointSize(point_size);
				glBegin(GL_POINTS);
				for(unsigned int a=start;a<start+size();a++)
				{
					p=(*this)[a%size()];
					if(p.point_size>0.0)
					{
						// reduce number of opengl state changes.
						if(p.point_size != point_size)
						{
							glEnd();
							point_size = p.point_size;
							glPointSize(point_size);
							glBegin(GL_POINTS);
						}
						glColor3f(p.r,p.g,p.b);
						glVertex2f((p.x-offset.x)*scale.x,(p.y-offset.y)*scale.y);
					}
					glEnd();
				}

				/*
				{
				Point2d p;
				// for speedup skip points - this is useful if we have many many points
				float step=1;

				for(unsigned int t=0;t<size();t++)
				{
				if(size()>max_points)
				step=size()/float(max_points);

				glLineWidth(p.line_width);
				glBegin(GL_LINE_STRIP);
				for(float a=0;a<size();a+=step)
				{
				p=(*this)[int(a)];
				if(p.line_width>0)
				{
				glColor3f(p.r,p.g,p.b);
				glVertex2f((p.x-offset.x)*scale.x,(p.y-offset.y)*scale.y);
				}
				if(p.x>maximum.x)maximum.x=p.x;
				if(p.x<minimum.x)minimum.x=p.x;
				if(p.y>maximum.y)maximum.y=p.y;
				if(p.y<minimum.y)minimum.y=p.y;		
				}
				glEnd();

				//glBegin(GL_POINTS);
				for(float a=0;a<size();a+=step)
				{
				p=(*this)[int(a)];
				if(p.point_size>0.0)
				{
				glPointSize(p.point_size);
				glColor3f(p.r,p.g,p.b);
				glBegin(GL_POINTS);
				glVertex2f((p.x-offset.x)*scale.x,(p.y-offset.y)*scale.y);
				glEnd();
				}

				//glColor3f(p.r,p.g,p.b);
				//glVertex2f((p.x-offset.x)*scale.x,(p.y-offset.y)*scale.y);
				}
				}
				}*/
			}

		public:
			Trace()
			{
				cur_col[0]=cur_col[1]=cur_col[2]=1.0;
				cur_line_width=1.0;
				cur_point_size=0.0;
				scroll=false;
				pos=0;
				max_points=10000;//std::max(w,h); <- setting max_points that low gives problems with scatter plots
			}

			/**
			*	plot a point at (x,y) to the currently active trace. 
			*	you may switch the trace with a call to trace(int _trace)
			*/
			void plot(const float x, const float y)
			{ 

				Point2d p(x,y, cur_col[0], cur_col[1], cur_col[2], cur_line_width, cur_point_size);
				if(scroll)
				{
					// this realises a sort of ringbuffer wich is needed for scrolling
					if(pos<size())
						(*this)[pos]=p;
					else
						push_back(p);
					pos++;

					if(pos>=max_points)
						pos=0;
				}
				else
					push_back(p);
			}




			/**
			*	sets the current drawing color in rgb format. 
			*   r,g,b are in the range [0..1]
			*/
			void color3f(float r, float g, float b)
			{
				cur_col[0]=r;
				cur_col[1]=g;
				cur_col[2]=b;
			}

			/**
			*	call set_linewidth to change the thickness of the traces. the default
			*	value is 1 pixel, if you set the linewidth to zero, no lines are drawn. this
			*	is usefull to create scatter-plots.
			*/
			void set_linewidth(float width){ cur_line_width=width; }

			/**
			*	this function sets the size of the plot-points. the default value is zero, so 
			*	no points are drawn at all. if you wish to create a scatter-plot, set the pointsize
			*  to a value bigger than zero and the linesize to zero.
			*/
			void set_pointsize(float psize){ cur_point_size = psize; }


			/**
			*	set the maximum number of points to be plotted. this is useful to
			*	avoid slow drawing of your trace. if you have 1000 plot-points and 
			*	set the number of max_points to 100, then only every tenth point gets plotted.
			*/
			void set_max_points(int mx){ max_points=mx;}

			/**
			*	if you call set_scrolling with a positive number of points to be plotted,
			*	your graph will scroll left out of the plot-window as you add new plot-points.
			*	Zero or a negative number disables scrolling.
			*/
			void set_scrolling(int max_points_to_plot)
			{
				if(max_points_to_plot<=0)
				{
					scroll=false;
					return;
				}

				scroll=true;
				max_points=max_points_to_plot;
				//if(traces[cur_trace].capacity()<max_points)
				//	traces[cur_trace].reserve(max_points);

			}

			/**
			*	clear() removes all points from the trace.
			*	the trace is empty afterwards and can be filled
			*	with plot(x,y) again.
			*/
			void clear() { std::vector<Point2d>::clear(); pos=0; }
		};


		virtual ~Multiplot() {}
        // warning: unused parameter 'x'
        // warning: unused parameter 'y'
		Multiplot(const int x,const int y,const int w,const int h, const std::wstring& ttitle=L"Multiplot - updates on www.andre-krause.net", bool fullscreen=false) : Multiplot_base(x,y,w,h,ttitle,fullscreen)
		{
			cur_point_size=0.0;
			cur_trace=0;
			
			title=ttitle;
			
			// auto scaling vars
			scaling_ = MP_AUTO_SCALE;
			maximum.x=maximum.y=-FLT_MAX;
			minimum.x=minimum.y=FLT_MAX;
			Point2d range_min, range_max;
			
			traces.push_back( Trace() ); // create one trace

			gridx=MP_NO_GRID;
			gridy=MP_NO_GRID;
			gridx_step=-1;
			gridy_step=-1;

			bg_color.r=0.0f;
			bg_color.g=0.0f;
			bg_color.b=0.0f;

			grid_color.r=0.8f;
			grid_color.g=0.8f;
			grid_color.b=0.8f;
			grid_linewidth=1.0f;

		}

		
		/**
		*	Access function. allows direct access to a trace.
		*/
		Trace& operator[](int _trace) { return trace(_trace); }
		/**
		*	Access function. allows direct access to a trace.
		*/
		Trace& operator()(int _trace) { return trace(_trace); }

		/**
		*	sets the current trace.  traces are numbered
		*	from zero to N. memory for the traces is automatically allocated. 
		*/
		Trace& trace(int _trace)
		{
			cur_trace = _trace;
			if (cur_trace < 0) { cur_trace = 0; }
			if(traces.size() <= cur_trace )
			{
				for(size_t a = 0; a < cur_trace - traces.size() + 1; a++)
					traces.push_back(Trace());
			}
			return traces[_trace];
		}
		
		/**
		* plots a point at x,y to the currently active trace.
		* select a trace with a call to trace(int _tracenumber);
		*/
		void plot(const float x, const float y) {  traces[cur_trace].plot(x,y); }
		
		/**
		* plots a vector of values to the currently active trace.
		* the x value is running from 0 .. vector.size()-1
		* select a trace with a call to trace(int _tracenumber);
		*/
		template<class T> void plot(const std::vector<T>& v)
		{
			for(size_t x=0;x<v.size();x++)
			{
				traces[cur_trace].plot(float(x), float(v[x]));
			}
		}

		/**
		* change current drawing color for current trace.
		*/
		void color3f(float r, float g, float b) { traces[cur_trace].color3f(r,g,b); }
		
		/**
		* sets the window title.
		*/
		void set_title(const std::wstring& title_){ title=title_; }
		
		/**
		* changes current line width.
		*/
		void set_linewidth(float width){ traces[cur_trace].set_linewidth(width); }
		/**
		* changes current point size.
		*/
		void set_pointsize(float psize){ traces[cur_trace].set_pointsize(psize); }
		/**
		* changes scrolling behaviour for current trace - see class Trace for details.
		*/
		void set_scrolling(int max_points_to_plot){ traces[cur_trace].set_scrolling(max_points_to_plot); }

		/**
		* changes the (auto-)scaling behaviour of the multiplot window. you can choose between 
		* MP_AUTO_SCALE
		* MP_AUTO_SCALE_EQUAL
		* MP_FIXED_SCALE
		*/
		void set_scaling(enum MP_SCALING sc, float x_min=-10, float x_max= 10, float y_min=-10, float y_max=10)
		{
			scaling_ = sc;
			range_min.x = x_min;
			range_min.y = y_min;
			range_max.x = x_max;
			range_max.y = y_max;
		}

		
		/**
		 *	call this function if you wish a grid to be plotted in your graph.
		 *	by default, no grids are plotted. call this function with the first 
		 *	two arguments set to either MP_NO_GRID,	MP_LINEAR_GRID or MP_LOG_GRID.
		 *	the next two arguments gridx_step and gridy_step specify the grid spacing.
		 *	Zero or a negative value like -1 enables auto - spacing.
		 *	The last parameter w sets the grid-linewidth. the default is 1 pixel.
		 */
		void set_grid(enum MP_GRIDSTYLE ggridx=MP_LINEAR_GRID, enum MP_GRIDSTYLE ggridy=MP_LINEAR_GRID, float ggridx_step=-1.0, float ggridy_step=-1.0, float w=1.0)
		{
			gridx=ggridx;
			gridy=ggridy;
			gridx_step=ggridx_step;
			gridy_step=ggridy_step;
			grid_linewidth=w;
		}
		
		/**
		*	sets the background color
		*/
		void set_bg_color(float r, float g, float b)
		{
			bg_color.r=r;
			bg_color.g=g;
			bg_color.b=b;
			glClearColor(bg_color.r, bg_color.g, bg_color.b, 1);		// Set The background color
		}


		/**
		*	sets the grid color
		*/
		void set_grid_color(float r, float g, float b)
		{
			grid_color.r=r;
			grid_color.g=g;
			grid_color.b=b;
		}


		/**
		 *	this function call simply clears all traces
		 */
		void clear_all()
		{
			for(unsigned int a=0;a<traces.size();a++)
				traces[a].clear();
		}

		
		/**
		*	this function call clears trace number t
		*/
		void clear(int trace) { traces[trace].clear(); traces[trace].pos=0; }
	protected:
		float cur_point_size;
		unsigned int cur_trace;
		std::wstring title;		// stores the user-title, so we can add ranges
		Point2d bg_color;
		Point2d grid_color;

		// scaling behaviour
		int scaling_;
		Point2d range_min, range_max;
		Point2d minimum, maximum;
		Point2d scale;
		Point2d offset;


		std::vector< Trace > traces;

		// grid - vars
		int gridx;
		int gridy;
		float gridx_step, gridy_step, grid_linewidth;		
		Point2d grid_spacing;

		void initgl()
		{
			glViewport(0 , 0,width ,height);	// Set Up A Viewport
			glMatrixMode(GL_PROJECTION);								// Select The Projection Matrix
			glLoadIdentity();											// Reset The Projection Matrix
			glOrtho( 0, width, 0, height, -1, 1 );							// Select Ortho Mode (640x480)
			//gluPerspective(50, (float)w()/(float)h(), 5,  2000); // Set Our Perspective
			glMatrixMode(GL_MODELVIEW);									// Select The Modelview Matrix
			glLoadIdentity();											// Reset The Modelview Matrix
			glDisable(GL_DEPTH_TEST);									// Enable Depth Testing
			glDisable(GL_LIGHTING);
			glShadeModel(GL_SMOOTH);									// Select Smooth Shading
			glClearColor(bg_color.r, bg_color.g, bg_color.b, 1);		// Set The background color
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// Clear The Screen And Depth Buffer
		}	

		Point2d draw_grid()
		{
			double xstep=0;
			double ystep=0;
			if(gridx==MP_LINEAR_GRID)
			{
				double diff=maximum.x - minimum.x;
				if(diff==0)return Point2d(0,0);
				double exp=floor(log10(fabs(diff)));
				double shiftscale=pow(10.0,exp);
				// get the starting point for the grid
				double startx=shiftscale*floor(minimum.x / shiftscale);

				if(gridx_step>0)
				{
					xstep=gridx_step;
				}
				else	// auto grid size
				{			
					xstep=shiftscale*1.0;
					if(diff/xstep < 4) // draw more lines
						xstep*=0.5;

					/*
					xstep=floor(maximum.x / shiftscale) - floor(minimum.x / shiftscale);
					xstep=2*floor(0.5*xstep);// if uneven, make even, because uneven stepsizes will cause uneven distributed lines around the (0,0) koordinate frame axis
					xstep=shiftscale*xstep / 4.0;
					*/

				}
				double x=startx;

				glLineWidth(grid_linewidth);
				glColor3f(grid_color.r, grid_color.g, grid_color.b);
				glBegin(GL_LINES);
				int bailout=0; // bailout is a safety to avoid endless recursions caused maybe through numerical errors..
				while(x<maximum.x && bailout<100)
				{
					x+=xstep;
					bailout++;
					glVertex2f((GLfloat)(x-offset.x)*scale.x	,0.0f  );
					glVertex2f((GLfloat)(x-offset.x)*scale.x	,(float)height);
				}
				glEnd();
			}
			if(gridy==MP_LINEAR_GRID)
			{
				double diff=maximum.y - minimum.y;
				if(diff==0)return Point2d(0,0);
				double exp=floor(log10(fabs(diff)));
				double shiftscale=pow(10.0,exp);
				// get the starting point for the grid
				double starty=shiftscale*floor(minimum.y / shiftscale);

				if(gridy_step>0)
				{
					ystep=gridy_step;
				}
				else	// auto grid size
				{
					ystep=shiftscale*1.0;
					if(diff/ystep < 4) // draw more lines
						ystep*=0.5;

					/*
					ystep=floor(maximum.y / shiftscale) - floor(minimum.y / shiftscale);
					ystep=2*floor(0.5*ystep); // if uneven, make even, because uneven stepsizes will cause uneven distributed lines around the (0,0) koordinate frame axis
					ystep=shiftscale*ystep / 4.0;
					*/

				}
				double y=starty;
				glLineWidth(1.0);
				glColor3f(grid_color.r, grid_color.g, grid_color.b);
				glBegin(GL_LINES);
				int bailout=0; // bailout is a safety to avoid endless recursions caused maybe through numerical errors..
				while(y<maximum.y && bailout<100)
				{
					y+=ystep;
					bailout++;
					glVertex2f(0.0f					,(GLfloat)(y-offset.y)*scale.y);
					glVertex2f((float)width			,(GLfloat)(y-offset.y)*scale.y);
				}
				glEnd();

				#ifdef MULTIPLOT_FLTK // gl_font is only available with fltk...
                //********************************************************************
                // I don't know why I can't just stick the below
                // glRasterPos2f() && gl_draw() calls in the above loop, but
                // It doesn't work if I do.  Must be due to the glBegin() thingy?
                // So, I'm left to do the above loop again (for now)!
                // This hack also has the undesireable side effect of putting
                // The Axis Label in the Plot area itself, not in a so-called margin.
                //********************************************************************
                gl_font(1, 10);
                y=starty;
                bailout=0;
				std::string s;
                while(y<maximum.y && bailout<100)
                {
                    y+=ystep;
                    bailout++;
					s = std::to_string(y);
                    glRasterPos2f(0.5f, (GLfloat)(y-offset.y)*scale.y + 1 ); 
                    gl_draw(s.c_str(), int(s.length()));
                }
                //********************************************************************
				#endif
			}

			return Point2d((float)xstep,(float)ystep);
		}

		virtual void draw()
		{
			if(!valid() )
			{
				width=w();
				height=h();
				initgl();
				valid(true);
			}

			Multiplot_base::draw();

			glClear(GL_COLOR_BUFFER_BIT);// | GL_DEPTH_BUFFER_BIT);			// Clear The Screen And Depth Buffer


			// draw the grid
			grid_spacing=draw_grid();

			// draw the coordinate cross with center (0,0)
			glLineWidth(2.0f*grid_linewidth);
			glColor3f(grid_color.r, grid_color.g, grid_color.b);
			glBegin(GL_LINES);
			glVertex2f(0.0f					,0-offset.y*scale.y);
			glVertex2f((float)width			,0-offset.y*scale.y);
			glVertex2f(0-offset.x*scale.x	,0.0f  );
			glVertex2f(0-offset.x*scale.x	,(float)height);		
			glEnd();


			
			maximum.x=maximum.y=-FLT_MAX;
			minimum.x=minimum.y=FLT_MAX;

			for(unsigned int t=0;t<traces.size();t++)
			{
				traces[t].draw(minimum, maximum, scale, offset);
			}

				

			// do the (auto-) scaling
			offset.x=minimum.x;
			offset.y=minimum.y;
			float diff_x = maximum.x-minimum.x;
			float diff_y = maximum.y-minimum.y;
			switch(scaling_)
			{
			case MP_AUTO_SCALE:
				if(diff_x!=0.0)scale.x=width/diff_x;
				if(diff_y!=0.0)scale.y=height/diff_y;
				break;
			case MP_AUTO_SCALE_EQUAL:
				if(std::max(diff_x, diff_y) != 0)
				{
					scale.x=scale.y=std::min(width, height) / std::max(diff_x, diff_y);
					//scale.y=height/diff;
				}
				break;
			case MP_FIXED_SCALE:
				scale.x = width /(range_max.x - range_min.x);
				scale.y = height/(range_max.y - range_min.y);
				offset.x = range_min.x;
				offset.y = range_min.y;
				break;
			}


			// possible performance issue?
			std::wstringstream ss;
			ss << "x=[" <<  minimum.x << "; " << maximum.x << "] ";
			ss << "y=[" <<  minimum.y << "; " << maximum.y << "] ";
			if(gridx != MP_NO_GRID || gridy != MP_NO_GRID)
			{
				ss << "dx=[" <<  grid_spacing.x << "] ";
				ss << "dy=[" <<  grid_spacing.y << "] ";
			}
			set_caption( (ss.str() + title).c_str() );
		}
};

} // end namespace



#ifdef __TEST_THIS_MODULE__
// some tests and demos for multiplot

#include <vector>
#include <iostream>

#include <math.h>

using namespace std; 
using namespace multiplot;

// These are  compiler directives that includes libraries (For Visual Studio)
// You can manually include the libraries in the "Project->settings" menu under
// the "Link" tab.  You need these libraries to compile this program.
// this project file is configured in "multithreaded dll" mode. if your fltk was build using 
// "multithreaded" mode, you must change the code generation compiler settings.

#ifdef MULTIPLOT_FLTK
#ifdef _DEBUG
#pragma comment(lib, "fltk14/bin/lib/Debug/fltkd.lib")
#pragma comment(lib, "fltk14/bin/lib/Debug/fltk_gld.lib")
//#pragma comment(lib, "fltk14/bin/lib/Debug/fltk_imaged.lib")
#else
#pragma comment(lib, "fltk14/bin/lib/Release/fltk.lib")
#pragma comment(lib, "fltk14/bin/lib/Release/fltk_gl.lib")
#endif
#endif

#ifdef _WIN32
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "User32.lib")
//#if (_MSC_VER > 1700)
//#pragma comment (lib, "legacy_stdio_definitions.lib")
//#endif
#endif 





// most simple usage of Multiplot
void demo1()
{
	using namespace multiplot;

	// create a multiplot window
	// at position (x,y)=(10,10) and with 
	// a width and height of 300 pixels
	Multiplot m(10, 10, 600, 300);

	// make it visible
	m.show();

	// and plot a nice sine wave
	for(int x=0;x<300;x++)
	{
		// add point at (x,y) coordinates
		m.plot(float(x), 0.1f*x*sin(0.1f*x));

		m.redraw();

		// force event propagation thus redrawing
		if (!m.check()) { break; }
	}

	// wait till window is closed by user
	while(m.check())
	{
		m.redraw();
	}

}



// plot two scrolling traces in a window,
// set colors and draw a background grid
void demo2()
{
	Multiplot m(10,10,600,300);

	// draw a linear spaced grid for x and y-axis
	m.set_grid(MP_LINEAR_GRID,MP_LINEAR_GRID);		

	// set background color
	m.set_bg_color(0,0,0.5);

	// enable scrolling for both traces:
	m[0].set_scrolling(100);		// the last  100 added points of the plot will be drawn 
	m[1].set_scrolling(100);		// the last  100 added points of the plot will be drawn 

	m.show();


	for(int x=0;x<300;x++)
	{
		// set trace to draw to and drawing color:
		m.trace(0);
		m.color3f(1,1,0);
		m.plot(float(x), 0.1f*x*sin(x/5.0f));

		m.trace(1);
		m.color3f(1,0,0);
		m.plot(float(x), 0.1f*x*sin(x/6.0f));


		m.redraw();
		if (!m.check()) { break; }
	}

	while(m.check())
	{
		m.redraw();
	}

}


// demo3 shows more advanced settings
void demo3()
{
	Multiplot m(10,10,600,600);

	m.set_grid(MP_LINEAR_GRID, MP_LOG_GRID);

	m.set_scrolling(100);		// the last  50 added points of the plot will be drawn 
	m.set_linewidth(2);	// first trace has line-width of 2	
	m.set_pointsize(4);		// set the point size of the first trace to 4

	m.show();


	for(int x=0;x<1000;x++)
	{
		// instead of calling m.trace(0), you may also 
		// use the brackets operator to access individual traces
		m[0].set_pointsize(4.0f + (x%10));
		m[0].color3f(1, 0.01f*(x%100), 0.05f*(x%20) );
		m[0].plot(0.1f*x*cos(x/5.0f), 0.1f*x*sin(x/5.0f));

		m[1].color3f(0.1f*(x%10), 0.05f*(x%20), 0.01f*(x%100) );
		m[1].plot(0.1f*x*cos(x/20.0f), 0.1f*x*sin(x/5.0f));

		m.redraw();
		if (!m.check()) { break; }
	}

	while(m.check())
	{
		m.redraw();
	}
}

// demo4: how to do scatter-plots
void demo4()
{
	Multiplot m(10,10,600,600);
	m.show();

	m.set_linewidth(0.0); // disable lines by setting line width to zero

	for(int x=0;x<3000;x++)
	{
		m.color3f(1, 0.01f*(x%100), 0.05f*(x%20) );
		
		

		for(int b=0;b<10;b++)
		{
			// two dimensional gauss - distribution
			float rx = -5 + 10.0f*rand()/RAND_MAX;
			float ry = -5 + 10.0f*rand()/RAND_MAX;

			float gval = exp(-(rx*rx + ry*ry));
			if(1.0*rand()/RAND_MAX < gval)
			{
				m.set_pointsize(1 + 5*gval);
				m.plot(rx, ry);
			}
		}

		m.redraw();
		if (!m.check()) { break; }
	}
	while(m.check())
	{
		m.redraw();
	}
}

// fullscreen (win32 only)
void demo5()
{
	// create a multiplot window
	// last parameter says if fullscreen or not
	Multiplot m(0, 0, 800, 600, L"", true);
	m.show();
	for(int x=0;x<1000;x++)
	{
		m.plot(float(x), 0.1f*x*sin(0.1f*x));
		m.redraw();
		if (!m.check()) { break; }
	}

	while(m.check())
	{
		m.redraw();
	}
}

void demo6()
{
	vector<float> v1,v2;
	for(int a=0;a<100;a++)
	{
		v1.push_back( sin(0.3f*a) );
		v2.push_back( cos(0.3f*a)+2.5f );
	}
	Multiplot mp(20,20,500,500);
	mp.show();
	mp.plot(v1);
	mp.trace(1);
	mp.plot(v2);
	while(mp.check())
	{
		mp.redraw();
	}
}

void demo7()
{
	Multiplot m(10, 10, 600, 300);
	m.set_scaling(MP_FIXED_SCALE, 0, 300, -20, 20);
	m.show();

	for (int x = 0; x < 300; x++)
	{
		m.plot(float(x), 0.1f*x*sin(0.1f*x));
		m.redraw();
		if (!m.check()) { break; }
	}
	while (m.check()) { m.redraw(); }
}


void test_module()
{
	std::cout << "\nSeveral multiplot demos. press ESC inside the multiplot window to stop a demo and test the next demo.\n";

	int demo_number = 0;
menu:
	std::cout << "\n\n === Menu ===";
	std::cout << "\n(1) demo: most basic/ simple usage. plots a sine wave.";
	std::cout << "\n(2) demo: two traces in a multiplot window with colors and scrolling.";
	std::cout << "\n(3) demo: more settings (point size, line-width and grid).";
	std::cout << "\n(4) demo: how to do a scatter plot (random gauss distribution).";
	std::cout << "\n(5) demo: fullscreen (win32 only at the moment)";
	std::cout << "\n(6) demo: plot values in a std::vector<float>";
	std::cout << "\n(7) demo: no auto-scaling, set fixed scaling of both x and y axis.";
	std::cout << "\n(8) exit.";
	std::cout << "\nenter number of demo (1..7):";
	std::cin >> demo_number;
	switch (demo_number)
	{
	case 1:demo1(); break;
	case 2:demo2(); break;
	case 3:demo3(); break;
	case 4:demo4(); break;
	case 5:demo5(); break;
	case 6:demo6(); break;
	case 7:demo7(); break;
	case 8:return;  break;
	default:demo1(); break;
	}
	goto menu; // goto is not evil. at least in this case...
	return;
}


#endif