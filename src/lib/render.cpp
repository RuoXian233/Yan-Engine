#include "render.h"
#include <SDL2/SDL_image.h>
#include "resource.h"
#include "consts.h"

using namespace chem_war;

SDL_Renderer *Renderer::renderer;
SDL_Window *Renderer::window;
Uint64 Renderer::ticks;
float Renderer::prevFrameDeltatime;
TTF_Font *Renderer::font;


void Renderer::Initialize() {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    TTF_Init();
    Renderer::ticks = 0;
}

void Renderer::CreateWindow(int w, int h, const std::string &title) {
    Renderer::CreateWindow(w, h, title.c_str(), SDL_WINDOW_SHOWN);
}

void Renderer::CreateWindow(int w, int h, const char *title) {
    Renderer::CreateWindow(w, h, title, SDL_WINDOW_SHOWN);
}

void Renderer::CreateWindow(int w, int h, const char *title, Uint32 flags) {
    Renderer::window = SDL_CreateWindow(
        title, 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        w, h,
        flags
    );

    Renderer::renderer = SDL_CreateRenderer(Renderer::window, -1, 0);
}


void Renderer::Clear() {
    Renderer::ticks = SDL_GetPerformanceCounter();
    SDL_RenderClear(Renderer::renderer);
}

void Renderer::Update() {
    SDL_RenderPresent(Renderer::renderer);
    Renderer::prevFrameDeltatime = (SDL_GetPerformanceCounter() - Renderer::ticks) / (float) (SDL_GetPerformanceFrequency());
    if (prevFrameDeltatime < 1.0f / MAX_FRAMERATE) {
        SDL_Delay((Uint32) (1000 * (1.0f / MAX_FRAMERATE - prevFrameDeltatime)));
        Renderer::prevFrameDeltatime += (1.0f / MAX_FRAMERATE - prevFrameDeltatime);
    }
}

void Renderer::Finalize() {
    if (Renderer::HasFont()) {
        TTF_CloseFont(Renderer::font);
    }

    SDL_DestroyRenderer(Renderer::renderer);
    SDL_DestroyWindow(Renderer::window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


Renderer::Texture Renderer::CreateTexture(SDL_Surface *s) {
    Texture t;
    t.textureData = SDL_CreateTextureFromSurface(Renderer::renderer, s);
    t.size = Vec2(s->w, s->h);
    return t;
}

void Renderer::RenderTexture(const Renderer::Texture &t, const Vec2 &pos) {
    SDL_Rect r = { (int) pos.x, (int) pos.y, (int) t.size.x, (int) t.size.y };
    SDL_RenderCopy(Renderer::renderer, t.textureData, nullptr, &r);
}

float Renderer::GetDeltatime() {
    return Renderer::prevFrameDeltatime;
}

void Renderer::Draw(const std::string &drawCall) {

}

void Renderer::DrawRect(const Vec2 &pos, const Vec2 &size) {
    SDL_Rect r = Vec2::CreateRect(pos, size);
    SDL_RenderDrawRect(Renderer::renderer, &r);
}

void Renderer::FillRect(const Vec2 &pos, const Vec2 &size) {
    SDL_Rect r = Vec2::CreateRect(pos, size);
    SDL_RenderFillRect(Renderer::renderer, &r);
}

void Renderer::SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetRenderDrawColor(Renderer::renderer, r, g, b, a);
}

void Renderer::SetDrawColor(const SDL_Color &color) {
    SDL_SetRenderDrawColor(Renderer::renderer, color.r, color.g, color.b, color.a);
}

void Renderer::ClearDrawColor() {
    Renderer::SetDrawColor(0, 0, 0, 0);
}


bool Renderer::HasFont() {
    return Renderer::font != nullptr;
}

void Renderer::LoadFont(const std::string &font, int ptSize) {
    assert(!HasFont() && "Font resource already exists");
    auto f = TTF_OpenFont(font.c_str(), ptSize);
    if (!f) {
        Fatal("Font load failed");
    }
    Renderer::font = f;
}

TTF_Font *Renderer::GetFont() {
    return Renderer::font;
}

void Renderer::ChangeFontSize(int ptSize) {
    assert(HasFont() && "No font to be changed");
    TTF_SetFontSize(Renderer::font, ptSize);
}

void Renderer::ReloadFont(const std::string &font, int ptSize) {
    assert(HasFont() && "No font is loaded, use Renderer::LoadFont() instead");
    TTF_CloseFont(Renderer::font);
    Renderer::font = nullptr;
    Renderer::LoadFont(font, ptSize);
}

Renderer::Texture Renderer::Text(const std::string &text) {
    SDL_Color c;
    SDL_GetRenderDrawColor(Renderer::renderer, &c.r, &c.g, &c.b, &c.a);
    return Renderer::Text(text, c);
}

Renderer::Texture Renderer::Text(const std::string &text, const SDL_Color &color) {
    assert(HasFont() && "No font resource");
    SDL_Surface *surf = TTF_RenderUTF8_Blended(Renderer::font, text.c_str(), color);
    auto texture = SDL_CreateTextureFromSurface(Renderer::renderer, surf);
    ResourceManager::RegisterResource(std::format("text.Texture {}{}", (void *) texture, rand()), ResourceType::Texture, surf);
    
    Renderer::Texture t;
    t.size.x = surf->w;
    t.size.y = surf->h;
    t.textureData = texture;
    return t;
}

Vec2 Renderer::GetRenderSize() {
    int w, h;
    SDL_GetWindowSize(Renderer::window, &w, &h);
    return Vec2 { (float) w, (float) h };
}

void Renderer::DrawCircle(const Vec2 &pos, int radius, float delta) {
    float angle = 0;
    for (int i = 0; i < 360 / delta; i++){
        float prevradian = DEG_TO_RAD(angle), nextradian = DEG_TO_RAD(angle + delta);
        SDL_RenderDrawLine(Renderer::renderer, 
            pos.x + radius * cosf(prevradian),
            pos.y + radius * sinf(prevradian),
            pos.x + radius * cosf(nextradian),
            pos.y + radius * sinf(nextradian)
        );
        angle += delta;
    }
}

Renderer::Texture Renderer::CreateRenderContext(const Vec2 &size) {
    auto t = SDL_CreateTexture(Renderer::renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size.x, size.y);
    Renderer::Texture textureWrapper = { t, size };
    return textureWrapper;
}

void Renderer::SetRenderContext(const Renderer::Texture &texture) {
    SDL_SetRenderTarget(Renderer::renderer, texture.textureData);
}

void Renderer::ClearRenderContext() {
    SDL_SetRenderTarget(Renderer::renderer, nullptr);
}

void Renderer::DeleteRenderContext(Renderer::Texture &texture) {
    SDL_DestroyTexture(texture.textureData);
    texture.textureData = nullptr;
    texture.size = Vec2();
}
