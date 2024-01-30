#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

using namespace std;

//screen dimension const
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const int LEVEL_WIDTH = 1280;
const int LEVEL_HEIGHT = 960;

class LTexture
{
private:
    //the actual hardware texture
    SDL_Texture *mTexture;

    //image dimensions
    int mWidth;
    int mHeight;
public:
    // initialize variables
    LTexture();

    //deallocate memory
    ~LTexture() {};

    //load image from file
    bool loadFromFile(string);

    #if defined (SDL_TTF_MAJOR_VERSION)
    //create image from font string
    bool loadFromRenderedText(string, SDL_Color);
    #endif

    //render texture at given point
    void render(int, int, SDL_Rect* = NULL, double angle = 0, SDL_Point* center = NULL, SDL_RendererFlip = SDL_FLIP_NONE);

    //free memory
    void free();

    // set blend mode for texture
    void setBlendMode();

    //set the amount of alpha
    void setAlphaMode(Uint8 alpha);

    //increase or decrease color (r,g,b);
    void setColor(Uint8 red, Uint8 green, Uint8 blue);

    //gets image dimensions
    int getWidth();
    int getHeight();
};

class Dot{
private:
    //position
    int mPosX;
    int mPosY;

    //velocity
    int mVelX;
    int mVelY;

public:
    //the dimension of dot
    static const int DOT_WIDTH = 20;
    static const int DOT_HEIGHT = 20;

    //maximum axis velocity of the dot
    static const int DOT_VEL = 10;

    //constructor
    Dot(int _x, int _y);

    //adjust velocity through handle event
    void handle_event(SDL_Event &e);

    //moves the dot and checks collision
    void dot_move();

    //show the dot on the screen, the position relative to the camera
    void render(int camX, int camY);

    //get position
    int getPosX();
    int getPosY();
};


//the window we'll be rendering to
SDL_Window* gWindow = NULL;

//the window renderer
SDL_Renderer* gRenderer = NULL;

//fps texture
LTexture gDotTexture;

LTexture gBGTexture;

bool init();

//loads media
bool loadMedia();

//free media and shuts down sdl
void close_program();

int main(int argc, char *argv[])
{
    if (!init())
    {
        cout << "unable to initialize sdl\n";
    }
    else
    {
        //SET HINT FOR SCALE QUALITY
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear"))
        {
            cout << "warning to set hint for scale quality\n";
        }
        if (!loadMedia())
        {
            cout << "unable to load media\n";
        }
        else
        {
            //event handler
            SDL_Event e;
            bool quit = false;

            //the dot move around the window
            Dot dot(0, 0);

            //source space
            SDL_Rect camera = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

            while (!quit)
            {

                while (SDL_PollEvent(&e)) // handle events on queue
                {   //user request quit
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }
                    dot.handle_event(e);
                }
                //update the position of the dot
                dot.dot_move();

                //center camera over the dot
                camera.x = dot.getPosX() + Dot :: DOT_WIDTH / 2 - SCREEN_WIDTH / 2;
                camera.y = dot.getPosY() + Dot :: DOT_HEIGHT / 2 - SCREEN_HEIGHT / 2;

                //keep the camera in bounds
                if (camera.x < 0)   camera.x = 0;
                if (camera.x + camera.w > LEVEL_WIDTH ) camera.x = LEVEL_WIDTH - camera.w;
                if (camera.y < 0)   camera.y = 0;
                if (camera.y + camera.h > LEVEL_HEIGHT)    camera.y = LEVEL_HEIGHT - camera.h;

                //clear screen
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);

                //render texture
                //background first
                gBGTexture.render(0, 0, &camera);

                //dot later
                dot.render(camera.x, camera.y);

                //update screen
                SDL_RenderPresent(gRenderer);
            }
        }
    }
    close_program();
    return 0;
}

LTexture :: LTexture()
{
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

bool LTexture :: loadFromFile(string file)
{
    free();

    //the final texture
    SDL_Texture *newTexture = NULL;

    SDL_Surface *LoadedSurface = IMG_Load(file.c_str());
    if (LoadedSurface == NULL)
    {
        cout << "unable to load img from " << file << ". SDL ERROR: " << SDL_GetError() << endl;
    }
    else
    {
        //set color key image (background)
        SDL_SetColorKey(LoadedSurface, SDL_TRUE, SDL_MapRGB(LoadedSurface->format, 255, 255, 255));

        //create texture from surface
        newTexture = SDL_CreateTextureFromSurface(gRenderer, LoadedSurface);
        if (newTexture == NULL)
        {
            cout << "failed to load new texture. SDL ERROR: " << SDL_GetError() << endl;
        }
        else
        {
            //get image dimensions
            mWidth = LoadedSurface->w;
            mHeight = LoadedSurface->h;
        }

        //free surface
        SDL_FreeSurface(LoadedSurface);
    }
    mTexture = newTexture;
    return mTexture != NULL;
}

#if defined (SDL_TTF_MAJOR_VERSION)
bool LTexture :: loadFromRenderedText(string textTexture, SDL_Color color)
{
    free();
    // final texture
    SDL_Texture *newTexture = NULL;

    //surface to create texture
    SDL_Surface *newSurface = NULL;
    newSurface = TTF_RenderText_Solid(gFont, textTexture.c_str(), color);
    if (newSurface == NULL)
    {
        cout << "unable to load new surface. ERROR: " << TTF_GetError() << endl;
    }
    else
    {
        newTexture = SDL_CreateTextureFromSurface(gRenderer, newSurface);
        if (newTexture == NULL)
        {
            cout << "failed to create texture. ERROR: " << SDL_GetError();
        }
        else
        {
            mTexture = newTexture;
            mWidth = newSurface->w;
            mHeight = newSurface->h;
        }
        SDL_FreeSurface(newSurface);
    }
    return mTexture != NULL;

}
#endif

//LTexture
void LTexture :: render (int x, int y, SDL_Rect *clip, double angle, SDL_Point *center, SDL_RendererFlip flip)
{
    SDL_Rect rectQuad = {x, y, mWidth, mHeight}; //destination space
    if (clip != NULL)
    {
        rectQuad.w = clip->w;
        rectQuad.h = clip->h;
    }
    SDL_RenderCopyEx(gRenderer, mTexture, clip, &rectQuad, angle, center, flip);
}

void LTexture :: free()
{
    //free texture if it exists
    if (mTexture != NULL)
    {
        SDL_DestroyTexture(mTexture);
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }

}

void LTexture ::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
    SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture :: setBlendMode()
{
    SDL_SetTextureBlendMode(mTexture, SDL_BLENDMODE_BLEND);
}

void LTexture :: setAlphaMode(Uint8 alpha)
{
    SDL_SetTextureAlphaMod(mTexture, alpha);
}

int LTexture :: getWidth()
{
    return this->mWidth;
}

int LTexture :: getHeight()
{
    return this->mHeight;
}

Dot :: Dot(int _x, int _y)
{
    //initialize dot's offset
    mPosX = _x;
    mPosY = _y;

    //initialize dot's velocity
    mVelX = 0;
    mVelY = 0;

}

void Dot :: handle_event(SDL_Event &e){
    //ignore repeated key down
    //if key is pressed
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0){
        switch (e.key.keysym.sym){
            case SDLK_UP: mVelY -= DOT_VEL; break;
            case SDLK_DOWN: mVelY += DOT_VEL; break;
            case SDLK_RIGHT: mVelX += DOT_VEL; break;
            case SDLK_LEFT: mVelX -= DOT_VEL; break;
        }
    }
    //if key is released
    else if (e.type == SDL_KEYUP && e.key.repeat == 0){
        switch (e.key.keysym.sym){
            case SDLK_UP: mVelY += DOT_VEL; break;
            case SDLK_DOWN: mVelY -= DOT_VEL; break;
            case SDLK_RIGHT: mVelX -= DOT_VEL; break;
            case SDLK_LEFT: mVelX += DOT_VEL; break;
        }
    }
}

//update position
void Dot :: dot_move(){
    //move the dot left or right
    mPosX += mVelX;

    //if the dot collided or move too far to the left or right
    if (mPosX < 0 || mPosX + DOT_WIDTH > LEVEL_WIDTH){
        mPosX -= mVelX;
    }

    //move the dot up or down
    mPosY += mVelY;

    //if the dot move too far to up or down
    if (mPosY < 0 || mPosY + DOT_HEIGHT > LEVEL_HEIGHT ){
        mPosY -= mVelY;
    }
}
void Dot :: render(int camX, int camY)
{
    //show the dot relative to the camera
    gDotTexture.render(mPosX - camX, mPosY - camY);
}

int Dot :: getPosX()
{
    return mPosX;
}

int Dot :: getPosY()
{
    return mPosY;
}

bool init()
{
    bool success = true;
    // initialize sdl subsystem
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_AUDIO) < 0)
    {
        cout << "failed to initialize sdl. SDL ERROR: " << SDL_GetError() << endl;
        success = false;
    }
    else
    {
        //create window
        gWindow = SDL_CreateWindow("huy dep zai v", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL)
        {
            cout << "failed to create window. SDL ERROR: " << SDL_GetError() << endl;
            success = false;
        }
        else
        {
            //create renderer
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED |  SDL_RENDERER_PRESENTVSYNC);//synchronize with screen
            //allow the rendering update with the same time the moniter update
            // allow screen not tear
            if (gRenderer == NULL)
            {
                cout << "failed to create renderer. SDL ERROR: " << SDL_GetError() << endl;
                success = false;
            }
            else
            {
                //set color for renderer
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

                //initialize img png loading
                int img_flag = IMG_INIT_PNG;
                if (!(IMG_Init(img_flag) & img_flag))
                {
                    cout << "failed to initialize img. SDL ERROR: " << SDL_GetError() << endl;
                    success = false;
                }
                if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
                    cout << "failed to initialize sdl mixed. ERROR: " << Mix_GetError() << endl;
                    success = false;
                }
            }
        }
    }
    return success;
}

bool loadMedia()
{
    bool success = true;

    //upload dot media
    if (!gDotTexture.loadFromFile("C:/learnSDL2/SDL2_test/media and etc/dot.bmp")){
        cout << "failed to load dot media\n";
        success = false;
    }
    if (!gBGTexture.loadFromFile("C:/learnSDL2/SDL2_test/media and etc/bg.png"))
    {
        cout << "failed to load bg texture\n";
        success = false;
    }
    return success;
}

void close_program()
{
    //free texture
    gDotTexture.free();

    //destroy render
    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;

    //destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
    IMG_Quit();
//    TTF_Quit();
}
