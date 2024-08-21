#include "font.hpp"

#include "font_locator.hpp"
#include "sdl_error.hpp"
#include "sdl_header.hpp"
#include "sdl_ttf_header.hpp"

namespace sdl {
namespace {

////////////////////////////////////////////////////////////////////////////////
// FontImpl
//

struct FontImpl final : public Font {
  FontImpl(const FontInfo& info, TTF_Font* font) : _info(info), _font(font) {}

  ~FontImpl() { TTF_CloseFont(_font); }

  virtual bee::OrError<Texture::ptr> render_text(
    Renderer& ren, const std::string& text) override
  {
    auto surface =
      TTF_RenderUTF8_Blended(_font, text.data(), Color::white().to_sdl_color());
    bail(
      texture,
      Texture::create_from_sdl_surface(ren.sdl_renderer(), surface, true));
    return std::move(texture);
  }

  virtual const FontInfo& info() const override { return _info; }

 private:
  FontInfo _info;
  TTF_Font* _font;
};

} // namespace

////////////////////////////////////////////////////////////////////////////////
// Font
//

Font::~Font() {}

bee::OrError<Font::ptr> Font::create(int ptsize)
{
  bail(font_info, FontLocator::find_font());
  auto font = TTF_OpenFont(font_info.file_path.data(), ptsize);
  return std::make_shared<FontImpl>(font_info, font);
}

////////////////////////////////////////////////////////////////////////////////
// TTF
//

bee::OrError<> TTF::init()
{
  bail_unit_sdl(TTF_Init());
  return bee::ok();
}

} // namespace sdl
