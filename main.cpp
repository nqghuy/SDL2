#include <bits/stdc++.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

using namespace std;

//screen dimension const
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//the number of data
const int TOTAL_DATA = 10;

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


//the window we'll be rendering to
SDL_Window* gWindow = NULL;

//the window renderer
SDL_Renderer* gRenderer = NULL;

//the font used in this program
TTF_Font *gFont = NULL;

//texture
LTexture gPromptTexture;

LTexture gDataTextures [TOTAL_DATA];

int gData[ TOTAL_DATA ];

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

            //quit flag
            bool quit = false;

            //set text color as black
            SDL_Color textColor = {0, 0, 0, 255};

            //set current data as red
            SDL_Color hightLight = {255, 0, 0, 255};

            //enable text input
            SDL_StartTextInput();

            //current input point
            int currentData = 0;

            while (!quit)
            {
                //the rerender text flag
                bool textRender = false;

                while (SDL_PollEvent(&e)) // handle events on queue
                {   //user request quit
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }
                    else if (e.type == SDL_KEYDOWN){
                        switch (e.key.keysym.sym)
                        {
                        case SDLK_DOWN:
                            //rerender previous entry input point
                            gDataTextures[currentData].loadFromRenderedText(to_string(gData[currentData]), textColor);
                            currentData++;
                            break;
                        case SDLK_UP:
                            //rerender previous entry input point
                            gDataTextures[currentData].loadFromRenderedText(to_string(gData[currentData]), textColor);
                            currentData--;
                            break;
                        case SDLK_RIGHT:
                            gData[currentData]++;
                            break;
                        case SDLK_LEFT:
                            gData[currentData]--;
                            break;
                        }
                        if (currentData < 0){
                            currentData = 9;
                        }
                        else if (currentData == 10){
                            currentData = 0;
                        }
                    }
                }
                gDataTextures[currentData].loadFromRenderedText(to_string(gData[currentData]), hightLight);

                //clear screen
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                SDL_RenderClear(gRenderer);

                //prompt texture
                gPromptTexture.render((SCREEN_WIDTH - gPromptTexture.getWidth()) / 2, 0);

                for (int i = 0; i < 10; i++){
                    gDataTextures[i].render((SCREEN_WIDTH - gDataTextures[i].getHeight()) /2, gPromptTexture.getHeight() + gDataTextures[i].getHeight() * i);
                }
                SDL_RenderPresent(gRenderer);


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
                if (TTF_Init() < 0){
                    cout << "failed to init true type font\n. ERROR: " << TTF_GetError() << endl;
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

    //open font
    gFont = TTF_OpenFont("media and etc/lazy.ttf", 28);
    if (gFont == NULL){
        cout << "failed to load ttf\n";
        success = false;
    }

    //create prompt texture
    if (!gPromptTexture.loadFromRenderedText("Enter text:", {0, 0, 0, 255})){
        cout << "failed to load prompt text texture\n";
        success = false;
    }

    //open file
    SDL_RWops *file = SDL_RWFromFile("nums.bin", "r+b");

    //if file does not exist
    if (file == NULL)
    {
        cout << "Warning: unable to open file\n";

        //create a new file
        file = SDL_RWFromFile("nums.bin", "w+b");
        if (file == NULL){
            cout << "failed to create file\n";
            success = false;
        }
        //create successful
        else {
            cout << "new file created\n";
            for (int i = 0; i < 10; i++){
                gData[i] = 0;
                SDL_RWwrite(file, &gData[i], sizeof(int), 1);
                SDL_RWclose(file);
            }
        }
    }
    //if file exists
    else{
        //copy data
        for (int i = 0; i < 10; i++){
            SDL_RWread(file, &gData[i], sizeof(int), 1);
        }
        SDL_RWclose(file);
    }

    //initialize data texture
    for (int i = 0; i < 10; i++){
        gDataTextures[i].loadFromRenderedText(to_string(gData[i]), {0, 0, 0, 255});
    }
    return success;
}

void close_program()
{
    //write data in file
    SDL_RWops *file = SDL_RWFromFile("nums.bin", "w");
    for (int i = 0; i < 10; i++){
        SDL_RWwrite(file, &gData[i], sizeof(int), 1);
    }
    SDL_RWclose(file);

    gFont = NULL;

    //free texture
    for (int i = 0; i < 10; i++){
        gDataTextures[i].free();
    }
    gPromptTexture.free();

    //destroy render
    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;

    //destroy window
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_StopTextInput();

    SDL_Quit();
    IMG_Quit();
    TTF_Quit();
}
