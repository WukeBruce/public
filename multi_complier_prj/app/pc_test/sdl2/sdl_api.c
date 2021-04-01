
/*
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Simple test of the SDL MessageBox API */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "SDL.h"
#include "SDL_thread.h"
#include "sdl_api.h"
#include "debug.h"


/* Call this instead of exit(), so we can clean up SDL: atexit() is evil. */
static void
quit(int rc)
{
    SDL_Quit();
    exit(rc);
}

static int
button_messagebox(void *eventNumber)
{
    const SDL_MessageBoxButtonData buttons[] = {
        {
            SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
            0,
            "OK"
        },{
            SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,
            1,
            "Cancel"
        },
    };

    SDL_MessageBoxData data = {
        SDL_MESSAGEBOX_INFORMATION,
        NULL, /* no parent window */
        "Custom MessageBox",
        "This is a custom messagebox",
        2,
        buttons,
        NULL /* Default color scheme */
    };

    int button = -1;
    int success = 0;
    if (eventNumber) {
        data.message = "This is a custom messagebox from a background thread.";
    }

    success = SDL_ShowMessageBox(&data, &button);
    if (success == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error Presenting MessageBox: %s\n", SDL_GetError());
        if (eventNumber) {
            SDL_UserEvent event;
            event.type = (intptr_t)eventNumber;
            SDL_PushEvent((SDL_Event*)&event);
            return 1;
        } else {
            quit(2);
        }
    }
    SDL_Log("Pressed button: %d, %s\n", button, button == 1 ? "Cancel" : "OK");

    if (eventNumber) {
        SDL_UserEvent event;
        event.type = (intptr_t)eventNumber;
        SDL_PushEvent((SDL_Event*)&event);
    }

    return 0;
}



static void
print_string(char **text, size_t *maxlen, const char *fmt, ...)
{
    int len;
    va_list ap;

    va_start(ap, fmt);
    len = SDL_vsnprintf(*text, *maxlen, fmt, ap);
    if (len > 0) {
        *text += len;
        if ( ((size_t) len) < *maxlen ) {
            *maxlen -= (size_t) len;
        } else {
            *maxlen = 0;
        }
    }
    va_end(ap);
}

static void
PrintText(char *text)
{
    char *spot, expanded[1024];

    expanded[0] = '\0';
    for ( spot = text; *spot; ++spot )
    {
        size_t length = SDL_strlen(expanded);
        SDL_snprintf(expanded + length, sizeof(expanded) - length, "\\x%.2x", (unsigned char)*spot);
    }
    SDL_Log("Text (%s): \"%s%s\"\n", expanded, *text == '"' ? "\\" : "", text);
}

static int
ApiKeyFunction(SDL_Keysym * sym, SDL_bool pressed, SDL_bool repeat)
{

	char message[512];
	char *spot;
	size_t left;

	spot = message;
	left = sizeof(message);

	if(sym == NULL ||(pressed == 1))
		return -1;
	if(sym->sym){
		switch (sym->sym) {
			case SDLK_ESCAPE:
			{
			    SDL_Log("--->exit %d\n",SDLK_ESCAPE);
			    return 1;
			    break;
			}
			case SDLK_0:
			{
			    SDL_Log("--->SDLK_0: %d\n",SDLK_0);
			    break;
			}
			case SDLK_1:
			{
			    SDL_Log("--->SDLK_0: %d\n",SDLK_1);
			    break;
			}
			case SDLK_2:
			{
			    break;
			}
			case SDLK_3:
			{
			    break;
			}
			case SDLK_4:
			{
			    break;
			}
			case SDLK_5:
			{
			    break;
			}
			case SDLK_6:
			{
			    break;
			}
			case SDLK_7:
			{
			    break;
			}
			case SDLK_8:
			{
			    break;
			}
			case SDLK_9:
			{
			    break;
			}

			default:
			    break;
		}

	}else{

		    /* Print the keycode, name and state */
	        print_string(&spot, &left,
	                "Unknown Key (scancode %d = %s) %s ",
	                sym->scancode,
	                SDL_GetScancodeName(sym->scancode),
	                pressed ? "pressed " : "released");
			return -1;
	}

	return   0;
}

static void
DrawKeyBoard(SDL_Renderer * renderer)
{
	int row = 0,coloum = 0,x = 0;
	SDL_Rect rect, darea;

	/* Get the Size of drawing surface */
	SDL_RenderGetViewport(renderer, &darea);

	for(row = 0; row < 8; row++)
	{
		coloum = row%2;
		x = x + coloum;
		for(coloum = 0; coloum < 4+(row%2); coloum++)
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);

			rect.w = darea.w/8;
			rect.h = darea.h/8;
			rect.x = x * rect.w;
			rect.y = row * rect.h;
			x = x + 2;
			SDL_RenderFillRect(renderer, &rect);
		}
		x=0;
	}
}

int sdl_api_test(void)
{
#if 0
	SDL_Window *window;
	SDL_Surface *surface;
	SDL_Renderer *renderer;


	SDL_Event event;
	int done;
	int select_ret = 0;


	/* Initialize SDL */
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		COM_INFO("Couldn't initialize SDL: %s\n", SDL_GetError());
	    return (1);
	}

	/* Set 640x480 video mode */
	window = SDL_CreateWindow("main window",
	                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                          640, 480, SDL_WINDOW_SHOWN);
	if (!window) {
	    COM_INFO("Couldn't create 640x480 window: %s\n",SDL_GetError());
	    quit(2);
	}


	{
		surface = SDL_GetWindowSurface(window);
		renderer = SDL_CreateSoftwareRenderer(surface);
		if(!renderer)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Render creation for surface fail : %s\n",SDL_GetError());
			return 1;
		}

		/* Clear the rendering surface with the specified color */
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		DrawKeyBoard(renderer);
		/* Got everything on rendering surface,
			   now Update the drawing image on window screen */
		SDL_UpdateWindowSurface(window);
	}


	SDL_StartTextInput();

	/* Watch keystrokes */
	done = 0;
	while (!done) {
		/* Check for events */
		SDL_WaitEvent(&event);

		switch (event.type) {
		    case SDL_KEYDOWN:
		    case SDL_KEYUP:
		    {
			select_ret = ApiKeyFunction(&event.key.keysym, (event.key.state == SDL_PRESSED) ? SDL_TRUE : SDL_FALSE, (event.key.repeat) ? SDL_TRUE : SDL_FALSE);
			break;
		    }
		    case SDL_TEXTINPUT:
		    {
		        PrintText(event.text.text);
		        break;
		    }
		    //case SDL_MOUSEBUTTONDOWN:
		        /* Any button press quits the app... */
		    case SDL_QUIT:
		    {
		        done = 1;
		        break;
		    }
		    default:
		        break;
		    }

		if(select_ret  == 1){
		 	done = 1;
		}

	}

	SDL_Quit();
    return (0);
#else
{

#define IMAGE_PATH "/home/wk/hub/brucehuge/multi_complier_prj/app/pc_test/theme/INDIA_HD/image/media/main/music_focus.bmp"

    SDL_Window *window;                    // Declare a pointer
    SDL_Surface *surface;
    SDL_Surface *image;


    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

    // Create an application window with the following settings:
    window = SDL_CreateWindow(
        "An SDL2 window",                  // window title
        SDL_WINDOWPOS_UNDEFINED,           // initial x position
        SDL_WINDOWPOS_UNDEFINED,           // initial y position
        640,                               // width, in pixels
        480,                               // height, in pixels
        SDL_WINDOW_OPENGL                  // flags - see below
    );

    // Check that the window was successfully created
    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    // but instead of creating a renderer, we can draw directly to the screen
    surface = SDL_GetWindowSurface(window);

    // let's just show some classic code for reference
    image = SDL_LoadBMP(IMAGE_PATH); // loads image
    SDL_BlitSurface(image, NULL, surface, NULL); // blit it to the screen
    SDL_FreeSurface(image);

    // this works just like SDL_Flip() in SDL 1.2
    SDL_UpdateWindowSurface(window);

    /// The window is open: could enter program loop here (see SDL_PollEvent())
    SDL_Delay(3000);  /// Pause execution for 3000 milliseconds, for example

    /// Close and destroy the window
    SDL_DestroyWindow(window);

    /// Clean up
    SDL_Quit();
    return 0;
}

#endif
}
