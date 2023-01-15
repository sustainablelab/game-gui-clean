#include "mg_colors.h"
#include "SDL.h"
constexpr bool DEBUG = true;
namespace GameArt
{
    SDL_Texture* tex{};
    int pixel_size = 2;
    constexpr int scale = 20;
    namespace AspectRatio
    {
        constexpr int w = 16;
        constexpr int h = 9;
    }
    constexpr int w = AspectRatio::w * scale;
    constexpr int h = AspectRatio::h * scale;
}
namespace GameWin
{ // Size of actual game in OS window
    int w = GameArt::pixel_size * GameArt::w;
    int h = GameArt::pixel_size * GameArt::h;
}
namespace GtoW
{ // Coordinate transform from GameArt coordinates to Window coordinates
    /* *************DOC***************
     * W = (k*G) + Offset
     * G = (W - Offset)/k
     * Offset = W - (k*G)
    *******************************/
    namespace Offset
    {
        int x=0;
        int y=0;
    }
    int scale = GameArt::pixel_size;
}
namespace UI
{
    namespace Flags
    {
        bool window_size_changed{true};                 // True : initial win size unknown
    }
}
struct WindowInfo
{ // OS Window size and flags
    int x,y,w,h;
    Uint32 flags;
};

SDL_Window* win{};
SDL_Renderer* ren{};

void shutdown(void)
{
    SDL_DestroyTexture(GameArt::tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}
int main(int argc, char* argv[])
{
    WindowInfo wI{};
    { // Window setup
        { // Default values
            if(GameArt::pixel_size < 1)
            { // Pixel size must be >= 1 -- fix it and debug print about it
                if(DEBUG) printf("%d : GameArt::pixel_size was %d, changed to 1\n",
                        __LINE__, GameArt::pixel_size);
                GameArt::pixel_size = 1;
            }
            wI.x = 10;
            wI.y = 60;
            wI.w = GameWin::w;
            wI.h = GameWin::h;
            wI.flags = SDL_WINDOW_RESIZABLE;
        }
        { // If ;w<Space> (run as a Vim window overlay): get x,y,w,h from Vim
            if(argc>1) wI.x = atoi(argv[1]);
            if(argc>2) wI.y = atoi(argv[2]);
            if(argc>3) wI.w = atoi(argv[3]);
            if(argc>4) wI.h = atoi(argv[4]);
        }
        if(argc > 1)
        { // If ;w<Space> (run as a Vim window overlay): borderless, always on top
            wI.flags = SDL_WINDOW_BORDERLESS
                     | SDL_WINDOW_ALWAYS_ON_TOP
                     | SDL_WINDOW_INPUT_GRABBED;
        }
        if(DEBUG)
        { // Print window x,y,w,h
            printf("Window (x,y): (%d,%d)\n", wI.x, wI.y);
            printf("Window W x H: %d x %d\n", wI.w, wI.h);
        }
    }
    { // SDL Setup
        SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
        { // Video
            win = SDL_CreateWindow(argv[0], wI.x, wI.y, wI.w, wI.h, wI.flags);
            if(win == NULL)
            { // Cannot create a window, quit
                if(DEBUG) printf("%d : SDL error msg: %s\n", __LINE__, SDL_GetError());
                shutdown(); return EXIT_FAILURE;
            }
            if(DEBUG)
            {
                SDL_Rect d;
                SDL_GetDisplayBounds(SDL_GetWindowDisplayIndex(win), &d);
                printf("%d : Display size: %d x %d\n", __LINE__, d.w, d.h);
            }
            ren = SDL_CreateRenderer(win,-1,
                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
            if(ren == NULL)
            { // Cannot create a renderer, quit
                if(DEBUG) printf("%d : SDL error msg: %s\n", __LINE__, SDL_GetError());
                shutdown(); return EXIT_FAILURE;
            }
            if(argc==1)
            { // Try setting window opacity to 50% (when run with ;r<Space>)
                if( SDL_SetWindowOpacity(win, 0.5) < 0 )
                { // Not a big deal if this fails.
                    if(DEBUG) printf("%d : SDL error msg: %s\n",__LINE__,SDL_GetError());
                }
            }
            ///////////
            // GAME ART
            ///////////
            if(SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND) < 0)
            { // Cannot set blend mode to blend, quit
                if(DEBUG) printf("%d : SDL error msg: %s\n",__LINE__,SDL_GetError());
                shutdown(); return EXIT_FAILURE;
            }
            GameArt::tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                    SDL_TEXTUREACCESS_TARGET, GameArt::w, GameArt::h);
            if(GameArt::tex == NULL)
            { // Cannot create a game art texture, quit
                if(DEBUG) printf("%d : SDL error msg: %s\n",__LINE__,SDL_GetError());
                shutdown(); return EXIT_FAILURE;
            }
        }
    }
    srand(0);
    bool quit = false;
    while(!quit)
    {
        /////
        // UI
        /////
        SDL_Event e; while(SDL_PollEvent(&e))
        {
            switch(e.type)
            { // See SDL_EventType
                case SDL_QUIT: quit=true; break;

                // e.key
                case SDL_KEYDOWN:
                    switch(e.key.keysym.sym)
                    {
                        case SDLK_q: quit=true; break;
                    }
                    break;

                // e.window
                case SDL_WINDOWEVENT:
                    switch(e.window.event)
                    { // Respond to window resize
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            UI::Flags::window_size_changed = true;
                            if(DEBUG)
                            { // Print event name, timestamp, window size, game art size
                                printf("%d : e.window.event \"SDL_WINDOWEVENT_SIZE_CHANGED\" at %dms\n", __LINE__, e.window.timestamp);
                                printf("BEFORE: \tWindow W x H: %d x %d\tGameArt W x H: %d x %d\tGameWin W x H: %d x %d\tGtoW::scale: %d\n", wI.w, wI.h, GameArt::w, GameArt::h, GameWin::w, GameWin::h, GtoW::scale);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
            }
        }

        //////////
        // PHYSICS
        //////////
        if(UI::Flags::window_size_changed)
        { // Update stuff that depends on window size
            UI::Flags::window_size_changed = false;
            SDL_GetWindowSize(win, &wI.w, &wI.h);
            { // Resize game art to fit window
                /* *************Resize***************
                 * Use largest pixel size for GameArt to fit in Window.
                 * If window is smaller than GameArt, use GameArt w x h and let it clip.
                 * *******************************/
                if((wI.w<GameArt::w)||(wI.h<GameArt::h)) GtoW::scale = 1;
                else
                {
                    int ratio_w = wI.w/GameArt::w;
                    int ratio_h = wI.h/GameArt::h;
                    // Use the smaller of the two ratios as the scaling factor
                    GtoW::scale = (ratio_w > ratio_h) ? ratio_h : ratio_w;
                }
                GameWin::w = GtoW::scale * GameArt::w;
                GameWin::h = GtoW::scale * GameArt::h;
            }
            { // Recenter game art in window
                /* *************Recenter***************
                 * If window is bigger, recenter game art.
                 * If window is smaller, pin game art topleft to window topleft.
                 * *******************************/
                if(wI.w>GameWin::w) GtoW::Offset::x = (wI.w-GameWin::w)/2;
                else GtoW::Offset::x = 0;
                if(wI.h>GameWin::h) GtoW::Offset::y = (wI.h-GameWin::h)/2;
                else GtoW::Offset::y = 0;
            }
            if(DEBUG) printf("AFTER: \tWindow W x H: %d x %d\tGameArt W x H: %d x %d\tGameWin W x H: %d x %d\tGtoW::scale: %d\n", wI.w, wI.h, GameArt::w, GameArt::h, GameWin::w, GameWin::h, GtoW::scale);
        }
        /////////
        // RENDER
        /////////
        SDL_SetRenderTarget(ren, GameArt::tex);
        ///////////
        // GAME ART
        ///////////

        ////////////
        // OS WINDOW
        ////////////
        SDL_SetRenderTarget(ren, NULL);
        { // Set background color of window to match my Vim background color
            SDL_Color c = Colors::blackestgravel;
            SDL_SetRenderDrawColor(ren, c.r,c.g,c.b,c.a);
            SDL_RenderClear(ren);
        }
        { // Stretch game art to window to get chunky pixels
            SDL_Rect src = SDL_Rect{0,0,GameArt::w,GameArt::h};
            SDL_Rect dst;                               // Destination window
            dst.x = GtoW::Offset::x;
            dst.y = GtoW::Offset::y;
            dst.w = GameWin::w;
            dst.h = GameWin::h;
            if(SDL_RenderCopy(ren, GameArt::tex, &src, &dst))
            {
                if(DEBUG) printf("%d : SDL error msg: %s\n",__LINE__,SDL_GetError());
            }
        }
        SDL_RenderPresent(ren);
        if(DEBUG) fflush(stdout);
    }
    shutdown(); return EXIT_SUCCESS;
}
