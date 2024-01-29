#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

using namespace std;

//screen dimension const
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

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

class LTimer{
private:
    //the clock time when timer starts
    Uint32 mStartTicks;

    //the ticks stored when timer is paused
    Uint32 mPauseTicks;

    //clock status
    bool mStart;
    bool mPause;

public:
    //the various clock actions
    void start();
    void stop();
    void pause();
    void unpause();

    //constructor
    LTimer();

    //get the timer ticks
    Uint32 getTicks();

    //check the status of timer
    bool isPaused();
    bool isStarted();
};

class Dot{
private:
    //position
    int mPosX;
    int mPosY;

    //velocity
    int mVelX;
    int mVelY;

    //dot's collision boxes
    vector<SDL_Rect> mCollider;

    //move the collision boxes relative to dot's offset
    void shift_colliders();


public:
    //the dimension of dot
    static const int DOT_WIDTH = 20;
    static const int DOT_HEIGHT = 20;

    //maximum axis velocity of the dot
    static const int DOT_VEL = 2;

    //constructor
    Dot(int _x, int _y);

    //adjust velocity through handle event
    void handle_event(SDL_Event &e);

    //moves the dot and checks collision
    void dot_move(vector <SDL_Rect> otherColliders);

    //show the dot on the screen
    void render();

    //get collision boxes
    vector <SDL_Rect> getColliders();
};


//the window we'll be rendering to
SDL_Window* gWindow = NULL;

//the window renderer
SDL_Renderer* gRenderer = NULL;

//font used in this program
TTF_Font *gFont = NULL;

//fps texture
LTexture gDotTexture;

bool init();

//loads media
bool loadMedia();

//box set collision detector
bool checkCollision(vector <SDL_Rect> &a, vector <SDL_Rect> &b);

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

            //the fixed dot
            Dot another_dot (SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4);

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
                dot.dot_move(another_dot.getColliders());

                //clear screen
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);

                //render texture
                dot.render();
                another_dot.render();

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
        SDL_SetColorKey(LoadedSurface, SDL_TRUE, SDL_MapRGB(LoadedSurface->format, 0, 0xFF, 0xFF));

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

#if defined (SDL_MAJOR_VERSION)
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

LTimer :: LTimer(){
    mStartTicks = 0;
    mPauseTicks = 0;
    mStart = false;
    mPause = false;
}

Uint32 LTimer :: getTicks()
{
    //the actual timer time
    Uint32 time = 0;

    //if timer is not started or stopped
    if (!isStarted())
    {
        time = 0;
    }
    else if (isPaused())
    {
        time = mPauseTicks;
    }
    else time = SDL_GetTicks() - mStartTicks;
    return time;
}

bool LTimer :: isStarted()
{
    return mStart;
}

bool LTimer :: isPaused()
{
    return mPause;
}

void LTimer :: start(){
    //start the timer
    mStart = true;

    //unpause the timer
    mPause = false;

    //get the current clock time
    mStartTicks = SDL_GetTicks();
    mPauseTicks = 0;
}

void LTimer :: stop()
{
    //stop the timer
    mStart = false;

    //unpause the timer
    mPause = false;

    //clear tick variables
    mStartTicks = 0;
    mPauseTicks = 0;
}

void LTimer :: pause()
{
    //if the timer is running and unpaused
    if (isStarted() && !isPaused())
    {
        mPause = true;

        //stores ticks
        mPauseTicks = SDL_GetTicks() - mStartTicks;

        //reset the timer
        mStartTicks = 0;
    }
}

void LTimer :: unpause()
{
    if (isStarted() && isPaused())
    {
        mPause = false;

        mStartTicks = SDL_GetTicks() - mPauseTicks;

        mPauseTicks = 0;
    }
}

Dot :: Dot(int _x, int _y)
{
    //initialize dot's offset
    mPosX = _x;
    mPosY = _y;

    //initialize dot's velocity
    mVelX = 0;
    mVelY = 0;

    //create the necessary SDL_Rect
    mCollider.resize(11);

    //initialize the collision boxes's width and height
    mCollider[0].w = 6;
    mCollider[0].h = 1;

    mCollider[1].w = 10;
    mCollider[1].h = 1;

    mCollider[2].w = 14;
    mCollider[2].h = 1;

    mCollider[3].w = 16;
    mCollider[3].h = 2;

    mCollider[4].w = 18;
    mCollider[4].h = 2;

    mCollider[5].w = 20;
    mCollider[5].h = 6;

    mCollider[6].w = 18;
    mCollider[6].h = 2;

    mCollider[7].w = 16;
    mCollider[7].h = 2;

    mCollider[8].w = 14;
    mCollider[8].h = 1;

    mCollider[9].w = 10;
    mCollider[9].h = 1;

    mCollider[10].w = 6;
    mCollider[10].h = 1;

    //initialize colliders relative to the position
    shift_colliders();
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
void Dot :: dot_move(vector <SDL_Rect> otherColliders){
    //move the dot left or right
    mPosX += mVelX;
    shift_colliders();

    //if the dot collided or move too far to the left or right
    if (mPosX < 0 || mPosX + DOT_WIDTH > SCREEN_WIDTH || checkCollision(mCollider, otherColliders)){
        mPosX -= mVelX;
        shift_colliders();
    }

    //move the dot up or down
    mPosY += mVelY;
    shift_colliders();

    //if the dot collided or move too far to up or down
    if (mPosY < 0 || mPosY + DOT_HEIGHT > SCREEN_HEIGHT || checkCollision(mCollider, otherColliders)){
        mPosY -= mVelY;
        shift_colliders();
    }
}
void Dot :: render()
{
    gDotTexture.render(mPosX, mPosY);
}

void Dot :: shift_colliders(){
    //the row offset
    int r = 0;

    //go through collision boxes
    for (int i = 0; i < (int)mCollider.size(); i++){
        //center the collision box
        mCollider[i].x = mPosX + (DOT_WIDTH - mCollider[i].w) / 2;

        //set the collision box at it row offset
        mCollider[i].y = mPosY + r;

        //adjust rot offset
        r += mCollider[i].h;
    }
}

vector <SDL_Rect> Dot :: getColliders()
{
    return mCollider;
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

                //initialize sdl ttf
                if (TTF_Init() < 0)
                {
                    cout << "failed to initialize SDL ttf. ERROR: " << TTF_GetError();
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
    return success;
}

bool checkCollision(vector <SDL_Rect> &a, vector<SDL_Rect> &b){
    //box dimensions
    int leftA, leftB;
    int rightA, rightB;
    int topA, topB;
    int bottomA, bottomB;

    //go through the a box
    for (int Abox= 0; Abox < (int)a.size(); Abox++){
        //calculate the side of rect a
        leftA = a[Abox].x;
        rightA = a[Abox].x + a[Abox].w;
        topA = a[Abox].y;
        bottomA = a[Abox].y + a[Abox].h;

        //go through the b box
        for (int Bbox= 0; Bbox < (int)b.size(); Bbox++){
            //set box b dimensions
            leftB = b[Bbox].x;
            rightB = b[Bbox].x + b[Bbox].w;
            topB = b[Bbox].y;
            bottomB= b[Bbox].y + b[Bbox].h;

            //check if there is collision
            if (!(rightA <= leftB || leftA >= rightB || topA >= bottomB || bottomA <= topB))   return true;
        }
    }
    //if no collision detection
    return false;
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
    TTF_Quit();
}
