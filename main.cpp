#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_image.h>
//#include <SDL_ttf.h>

using namespace std;

//screen dimension constant
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//button dimension const
const int BUTTON_WIDTH = 300;
const int BUTTON_HEIGHT = 200;
const int TOTAL_BUTTONS = 4;

enum LButtonSprite{
    BUTTON_SPRITE_MOUSE_OUT ,
    BUTTON_SPRITE_MOUSE_OVER_MOTION,
    BUTTON_SPRITE_MOUSE_DOWN = 2,
    BUTTON_SPRITE_MOUSE_UP = 3,
    BUTTON_SPRITE_TOTAL = 4
};

class LTexture{
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
    ~LTexture(){};

    //load image from file
    bool loadFromFile(string);

    #if defined(SDL_TTF_MAJOR_VERSION)
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

class LButton{
private:
    SDL_Point mPosition;
    LButtonSprite mCurrenSprite;
public:
    //initialize internal variables
    LButton();

    //set top left position
    void setPosition(int x, int y);

    //handle mouse event
    void handle_mouse_event(SDL_Event &e);

    //shows button sprite
    void render();
};


//the window we'll be rendering to
SDL_Window* gWindow = NULL;

//the window renderer
SDL_Renderer* gRenderer = NULL;

//4 source rectangle
SDL_Rect gSpriteClip[BUTTON_SPRITE_TOTAL];

//4 button for 4 corner
LButton gButton[TOTAL_BUTTONS];

//texture load img from file
LTexture gSpriteSheetTexture;

//starts up SDL and creates window
bool init();

//loads media
bool loadMedia();

//free media and shuts down sdl
void close_program();

int main(int argc, char *argv[]){
    if (!init())
    {
        cout << "unable to initialize sdl\n";
    }
    else{
        //SET HINT FOR SCALE QUALITY
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear")){
            cout << "warning to set hint for scale quality\n";
        }
        if (!loadMedia()){
            cout << "unable to load media\n";
        }
        else{
            //event handler
            SDL_Event e;
            bool quit = false;

            while (!quit)
            {
                while (SDL_PollEvent(&e)){// handle events on queue
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }
                    else{
                        for (int i = 0; i < TOTAL_BUTTONS; i++){
                            gButton[i].handle_mouse_event(e);
                        }
                    }
                }
                // clear screen
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 1);
                SDL_RenderClear(gRenderer);

                for (int i = 0; i < TOTAL_BUTTONS; i++){
                    gButton[i].render();
                }
                //update screen
                SDL_RenderPresent(gRenderer);
            }
        }
    }
    close_program();
    return 0;
}

LTexture :: LTexture(){
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

bool LTexture :: loadFromFile(string file){
    free();

    //the final texture
    SDL_Texture *newTexture = NULL;

    SDL_Surface *LoadedSurface = IMG_Load(file.c_str());
    if (LoadedSurface == NULL){
        cout << "unable to load img from " << file << ". SDL ERROR: " << SDL_GetError() << endl;
    }
    else{
        //set color key image (background)
        SDL_SetColorKey(LoadedSurface, SDL_TRUE, SDL_MapRGB(LoadedSurface->format, 0, 0xFF, 0xFF));

        //create texture from surface
        newTexture = SDL_CreateTextureFromSurface(gRenderer, LoadedSurface);
        if (newTexture == NULL){
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
bool LTexture :: loadFromRenderedText(string textTexture, SDL_Color color){
    free();
    // final texture
    SDL_Texture *newTexture = NULL;

    //surface to create texture
    SDL_Surface *newSurface = NULL;
    newSurface = TTF_RenderText_Solid(gFont, textTexture.c_str(), color);
    if (newSurface == NULL){
        cout << "unable to load new surface. ERROR: " << TTF_GetError() << endl;
    }
    else{
        newTexture = SDL_CreateTextureFromSurface(gRenderer, newSurface);
        if (newTexture == NULL)
        {
            cout << "failed to create texture. ERROR: " << SDL_GetError();
        }
        else {
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
void LTexture :: render (int x, int y, SDL_Rect *clip, double angle, SDL_Point *center, SDL_RendererFlip flip){
    SDL_Rect rectQuad = {x, y, mWidth, mHeight}; //destination space
    if (clip != NULL){
        rectQuad.w = clip->w;
        rectQuad.h = clip->h;
    }
    SDL_RenderCopyEx(gRenderer, mTexture, clip, &rectQuad, angle, center, flip);
}

void LTexture :: free(){
    //free texture if it exists
    if (mTexture != NULL){
        SDL_DestroyTexture(mTexture);
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }

}

void LTexture ::setColor(Uint8 red, Uint8 green, Uint8 blue){
    SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture :: setBlendMode(){
    SDL_SetTextureBlendMode(mTexture, SDL_BLENDMODE_BLEND);
}

void LTexture :: setAlphaMode(Uint8 alpha){
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

//LButton
LButton :: LButton(){
    mPosition.x = 0;
    mPosition.y = 0;
    mCurrenSprite = BUTTON_SPRITE_MOUSE_OUT;
}

void LButton :: setPosition(int x, int y){
    mPosition.x = x;
    mPosition.y = y;
}

void LButton :: handle_mouse_event (SDL_Event &e)
{
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEMOTION){
        //the coordinate of mouse
        int x, y;
        SDL_GetMouseState(&x, &y);

        //check inside
        bool inside = true;
        if (x < mPosition.x || y < mPosition.y || x > mPosition.x + BUTTON_WIDTH || y > mPosition.y + BUTTON_HEIGHT){
            inside = false;
        }
        if (!inside){
            mCurrenSprite = BUTTON_SPRITE_MOUSE_OUT;
        }
        else{
            mCurrenSprite = BUTTON_SPRITE_MOUSE_OVER_MOTION;
            if  (e.type == SDL_MOUSEBUTTONDOWN){
                mCurrenSprite = BUTTON_SPRITE_MOUSE_DOWN;
            }
            else if (e.type == SDL_MOUSEBUTTONUP){
                mCurrenSprite = BUTTON_SPRITE_MOUSE_UP;
            }
        }
    }
}

void LButton :: render(){
    gSpriteSheetTexture.render(mPosition.x, mPosition.y, &gSpriteClip[mCurrenSprite]);
}

bool init()
{
    bool success = true;
    // initialize sdl subsystem
    if (SDL_Init(SDL_INIT_EVENTS) < 0)
    {
        cout << "failed to initialize sdl. SDL ERROR: " << SDL_GetError() << endl;
        success = false;
    }
    else {
        //create window
        gWindow = SDL_CreateWindow("huy dep zai v", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL){
            cout << "failed to create window. SDL ERROR: " << SDL_GetError() << endl;
            success = false;
        }
        else
        {
            //create renderer
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);//synchronize with screen
            //allow the rendering update with the same time the moniter update
            // allow screen not tear
            if (gRenderer == NULL){
                cout << "failed to create renderer. SDL ERROR: " << SDL_GetError() << endl;
                success = false;
            }
            else{
                //set color for renderer
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

                //initialize img png loading
                int img_flag = IMG_INIT_PNG;
                if (!(IMG_Init(img_flag) & img_flag)){
                    cout << "failed to initialize img. SDL ERROR: " << SDL_GetError() << endl;
                    success = false;
                }

                #if defined (SDL_TTF_MAJOR_VERSION)
                //initialize sdl ttf
                if (TTF_Init() < 0){
                    cout << "failed to initialize SDL ttf. ERROR: " << TTF_GetError();
                    success = false;
                }
                #endif
            }
        }
    }
    return success;
}

bool loadMedia(){
    bool success = true;
    if (!gSpriteSheetTexture.loadFromFile("C:/learnSDL2/SDL2_test/button.png"))
    {
        cout << "failed to load img\n";
        success = false;
    }
    else{

        //4 sprite in source img
        for (int i = 0; i < BUTTON_SPRITE_TOTAL; i++){
            gSpriteClip[i].x = 0;
            gSpriteClip[i].y = i * 200;
            gSpriteClip[i].w = BUTTON_WIDTH;
            gSpriteClip[i].h = BUTTON_HEIGHT;
        }

        //set position for 4 corner button
        gButton[0].setPosition(0, 0);
        gButton[1].setPosition(SCREEN_WIDTH - BUTTON_WIDTH, 0);
        gButton[2].setPosition(0, SCREEN_HEIGHT - BUTTON_HEIGHT);
        gButton[3].setPosition(SCREEN_WIDTH - BUTTON_WIDTH, SCREEN_HEIGHT - BUTTON_HEIGHT);
    }

    return success;
}

void close_program()
{
    gSpriteSheetTexture.free();

    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;

    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
    IMG_Quit();
//    TTF_Quit();
}
