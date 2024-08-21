#include "text_writer.hpp"

using std::string;

namespace sdl {

TextWriter::TextWriter(Texture::ptr&& text) : _tex(std::move(text)) {}

TextWriter::~TextWriter() {}

bee::OrError<> TextWriter::draw_text(
  Renderer& ren, const vec2i& pos, const string& text)
{
  int offset = 0;
  for (char c : text) {
    c = tolower(c);
    if (std::isalpha(c)) {
      int idx = c - 'a';
      Recti source{{idx * 32, 0}, {32, 32}};
      Recti dest{vec2i(pos.x + offset * 18 - 7, pos.y), {32, 32}};
      bail_unit(ren.fill_rect(*_tex, source, dest));
      offset++;
    } else if (isspace(c)) {
      offset++;
    }
  }

  return bee::ok();
}

bee::OrError<TextWriter::ptr> TextWriter::create(Renderer& renderer)
{
  bail(texture, renderer.create_texture(Images::Letters));
  return make_unique<TextWriter>(std::move(texture));
}

} // namespace sdl
