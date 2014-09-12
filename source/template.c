#include <stdlib.h>
#include <time.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#include <SDL/sdl.h>
#include <SDL/sdl_image.h>
#include <SDL/sdl_ttf.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

// upper left = x,y
// upper right = x2,y
// lower left = x,y2
// lower right = x2,y2

int JOE_X = 250; // upper left
int JOE_Y = 220; // upper left
int JOE_X2 = 250 + 100; // upper right
int JOE_Y2 = 220 + 180; // lower left

int COIN_X = 400;
int COIN_Y = 270;
int COIN_X2 = 400 + 50;
int COIN_Y2 = 270 + 50;

int SCROLL_X = 0;

int DONE_X = 100;
int DONE_Y = 50;

int scoreNum = 0;

bool closeRequested = false;
bool coinCollected = false;

TTF_Font *font = NULL;
SDL_Color textColor = {0,0,0};

SDL_Surface *screen = NULL;
SDL_Surface *background = NULL;
SDL_Surface *joe = NULL;
SDL_Surface *score = NULL;
SDL_Surface *coin = NULL;
SDL_Surface *done = NULL;

SDL_Surface *load_image( char* filename )
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized image that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = SDL_LoadBMP( filename );

    //If the image loaded
    if( loadedImage != NULL )
    {
        //Create an optimized image
        optimizedImage = SDL_DisplayFormat( loadedImage );

        //Free the old image
        SDL_FreeSurface( loadedImage );

        //If the image was optimized just fine
        if( optimizedImage != NULL )
        {
            //Map the color key
            Uint32 colorkey = SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0 );

            //Set all pixels of color R 0, G 0xFF, B 0 to be transparent
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, colorkey );
        }
    }

    //Return the optimized image
    return optimizedImage;
}

void init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		SDL_Delay(5000);
		exit(EXIT_FAILURE);
	}
	WPAD_Init();
	atexit(SDL_Quit);
	SDL_ShowCursor(SDL_DISABLE);
	
	screen = SDL_SetVideoMode(640,480,16,SDL_DOUBLEBUF);
	if (!screen)
	{
		fprintf(stderr, "Unable to set video: %s\n", SDL_GetError());
		SDL_Delay(5000);
		exit(EXIT_FAILURE);
	}
	
	if (TTF_Init() == -1)
	{
		fprintf(stderr, "Unable to init TTF. %s\n", SDL_GetError());
		SDL_Delay(5000);
		exit(EXIT_FAILURE);
	}
	
	font = TTF_OpenFont("sd:/apps/TestGame/arial.ttf", 28);
	if (font == NULL)
	{
		fprintf(stderr, "Unable to load arial.ttf: %s\n", SDL_GetError());
		SDL_Delay(5000);
		exit(EXIT_FAILURE);
	}
	background = load_image("sd:/apps/TestGame/background.bmp");
	joe = load_image("sd:/apps/TestGame/joe.bmp");
	coin = load_image("sd:/apps/TestGame/coin.bmp");
	done = load_image("sd:/apps/TestGame/congrats.bmp");
}

void apply_surface (int x, int y, SDL_Surface* source, SDL_Surface* destination)
{
	SDL_Rect offset;
	offset.x = x;
	offset.y = y;
	SDL_BlitSurface(source, NULL, destination, &offset);
}

void cleanup()
{
	SDL_Quit();
	SDL_FreeSurface(background);
	SDL_FreeSurface(joe);
	SDL_FreeSurface(coin);
	SDL_FreeSurface(score);
	SDL_FreeSurface(done);
	TTF_CloseFont(font);
	TTF_Quit();
	exit(EXIT_SUCCESS);
}

void input()
{
	WPAD_ScanPads();
	u32 held = WPAD_ButtonsHeld(0);
	if (held & WPAD_BUTTON_HOME)
	{
		closeRequested = true;
	}
	if (held & WPAD_BUTTON_UP)
	{
		JOE_X -= 5;
		JOE_X2 -= 5;
		SCROLL_X += 5;
		apply_surface(0, 0, background, screen);	// sooooo hard on memory. but it'll have to do for now.
		apply_surface(JOE_X, JOE_Y, joe, screen);
	}
	if (held & WPAD_BUTTON_DOWN)
	{
		JOE_X += 5;
		JOE_X2 += 5;
		SCROLL_X -= 5;
		apply_surface(0, 0, background, screen);
		apply_surface(JOE_X, JOE_Y, joe, screen);
	}
	if (!coinCollected) apply_surface(COIN_X, COIN_Y, coin, screen);
}

void detectCollisions()
{
	if ((JOE_X2 > COIN_X && JOE_X2 < COIN_X2) || (JOE_X < COIN_X2 && JOE_X > COIN_X))
	{
		scoreNum += 15;
		apply_surface(0, 0, background, screen);
		apply_surface(JOE_X, JOE_Y, joe, screen);
		coinCollected = true;
	}
	if (coinCollected) apply_surface(DONE_X, DONE_Y, done, screen);
}

int main(int argc, char *argv[])
{
	init();
	score = TTF_RenderText_Solid(font, "Score: ", textColor);
	if (score == NULL)
	{
		fprintf(stderr, "Error rendering text. %s\n", SDL_GetError());
		SDL_Delay(5000);
		exit(EXIT_FAILURE);
	}
	apply_surface(0, 0, background, screen);
	apply_surface(JOE_X, JOE_Y, joe, screen);
	apply_surface(COIN_X, COIN_Y, coin, screen);
	while (!closeRequested)
	{
		input();
		detectCollisions();
		SDL_Flip(screen); // updates the screen
		VIDEO_WaitVSync(); // waits for vsync
	}
	cleanup();
	return 0;
}
