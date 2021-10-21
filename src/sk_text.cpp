#include "sk_text.h"

#include <array>
#include <stdexcept>
#include <string>
#include <harfbuzz/hb-ft.h>
#include <CImg.h>
#include <iostream>
#include <vector>
#define FONT_SIZE 64
cimg_library::CImg<unsigned char> img{ 500,500,1,4 };
namespace csl {
	struct vector
	{
		int32_t x;
		int32_t y;
		vector operator+(vector& other) const {
			return vector{ this->x + other.x, this->y + other.y };
		}
		void operator+=(vector other) {
			this->x += other.x;
			this->y += other.y;
		}
	};
}
#define FONT_DIR "./resources/fonts/CascadiaCode.ttf"
void write_to_pixel(FT_GlyphSlot glyph, csl::vector pen_pos) {
	std::cout << glyph->bitmap.width << std::endl;
	pen_pos += {0, 250};

	std::cout << (uint8_t)glyph->bitmap.pixel_mode << std::endl;
	size_t y = 0;
	for(size_t h=pen_pos.y;h>=pen_pos.y-glyph->bitmap.rows;h--)
	{
		for(size_t w=pen_pos.x;w<glyph->bitmap.width+pen_pos.x;w++)
		{
			auto alpha = *(glyph->bitmap.buffer + glyph->bitmap.width * (glyph->bitmap.rows - y - 1) + (w - pen_pos.x));
			std::array<unsigned char, 4> val{ 0,0,0,alpha };
			img.atXY(w, h) = *val.data();
		}
		y++;
	}
	std::cout << "But it's clearly working..." << std::endl;
}

void sk::text::test() {
	FT_Library ftl;
	FT_Face face;
	std::string path;
	

	if(FT_Init_FreeType(&ftl))
	{
		throw std::runtime_error("Could not create freetype context");
	}
	if(auto error = FT_New_Face(ftl, FONT_DIR, 0, &face))
	{
		throw std::runtime_error("Could not find font");
	}
	FT_Set_Char_Size(face, 0, FONT_SIZE*64, 0, 0);
	hb_font_t* font = hb_ft_font_create_referenced(face);
	hb_buffer_t* buf = hb_buffer_create();
	std::string text = "Hellgo world!";
	hb_buffer_add_utf8(buf, text.c_str(), text.size(), 0, text.length());
	hb_ft_font_set_funcs(font);
	hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
	hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
	hb_buffer_set_language(buf, hb_language_from_string("en",-1));
	hb_buffer_guess_segment_properties(buf);
	hb_shape(font, buf, nullptr,0);

	uint32_t glyph_count = 0;
	hb_glyph_info_t* glyphs = hb_buffer_get_glyph_infos(buf, &glyph_count);
	hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buf, &glyph_count);
	std::vector<hb_glyph_position_t> glyph_pos( positions,positions+glyph_count);
	csl::vector cursor_pos{ 0,0 };
	for(int i=0;i<glyph_count;i++)
	{
		if(FT_Load_Glyph(face,(glyphs+i)->codepoint,FT_LOAD_NO_BITMAP))
		{
			throw std::runtime_error("Error in loading glyph");

		}
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		write_to_pixel(face->glyph,cursor_pos);
		cursor_pos.x += (glyph_pos[i].x_advance + glyph_pos[i].x_offset) / 64;
		cursor_pos.y += (glyph_pos[i].y_advance + glyph_pos[i].y_offset) / 64;

	}
	img.save("./resources/outtext.tiff");



}
