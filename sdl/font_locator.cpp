#include "font_locator.hpp"

#include <vector>

#include "bee/filesystem.hpp"
#include "bee/string_util.hpp"
#include "bee/sub_process.hpp"

namespace sdl {
namespace {

bool is_regular_style(const FontInfo& info)
{
  return info.style == "style=Regular";
}

bool is_ubuntu_family(const FontInfo& info)
{
  return bee::contains_string(info.name, "Ubuntu");
}

bool is_preferred_over(const FontInfo& font1, const FontInfo& font2)
{
  bool is_reg1 = is_regular_style(font1);
  bool is_reg2 = is_regular_style(font2);
  if (is_reg1 != is_reg2) { return is_reg1; }
  bool is_ubuntu1 = is_ubuntu_family(font1);
  bool is_ubuntu2 = is_ubuntu_family(font2);
  if (is_ubuntu1 != is_ubuntu2) { return is_ubuntu1; }
  return false;
}

bee::OrError<FontInfo> find_from_fc()
{
  auto output_stdout = bee::SubProcess::OutputToString::create();
  auto output_stderr = bee::SubProcess::OutputToString::create();
  auto res = bee::SubProcess::run(
    {.cmd = bee::FilePath("fc-list"),
     .stdout_spec = output_stdout,
     .stderr_spec = output_stderr});
  if (res.is_error()) {
    return EF(
      "Failed to run fc-list: $\nstderr:$",
      res.error(),
      output_stderr->get_output());
  }
  bail(output, output_stdout->get_output());
  auto lines = bee::split_lines(output);

  int num_fonts = 0;
  std::optional<FontInfo> chosen_font;
  for (auto&& line : lines) {
    num_fonts++;
    auto parts = bee::split(line, ":");
    if (parts.size() != 3) { continue; }
    FontInfo cand_font{
      .file_path = bee::FilePath(parts[0]),
      .name = bee::trim_spaces(parts[1]),
      .style = parts[2]};
    if (
      !chosen_font.has_value() || is_preferred_over(cand_font, *chosen_font)) {
      chosen_font = std::move(cand_font);
    }
  }
  if (!chosen_font.has_value()) {
    return EF("No valid fonts returned by fc-list out of $ fonts", num_fonts);
  }
  return std::move(*chosen_font);
}

} // namespace

bee::OrError<FontInfo> FontLocator::find_font() { return find_from_fc(); }

} // namespace sdl
