#include "draw_text.h"

extern input_c in;

ftchar_t ch[128];
shader_c textshader;

glid text_vao, text_vbo;

void SetupText()
{
	FT_Library ft;
	FT_Face face;
	char filename[FILENAME_MAX] = {};
	shader_c tmp("shaders/txt_v.glsl", "shaders/txt_f.glsl");
	
	textshader = tmp;

	strcat(filename, GAMEDIR);
	strcat(filename, "system/lucon.ttf");

	if (FT_Init_FreeType(&ft))
		SYS_Exit("Could not initialize FreeType!");

	if (FT_New_Face(ft, filename, 0, &face))
		SYS_Exit("Could not load font %s!", filename);

	FT_Set_Pixel_Sizes(face, 0, 48);

	//if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
	//	SYS_Exit("Unable to load char 'X'!");

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
	glActiveTexture(TEXT_TEXTURE_UNIT);

	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			printf("Freetype failed to load %c!\n", c);
			continue;
		}
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_R8,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// now store character for later use
		
		ch[c].id = texture;
		ch[c].x = face->glyph->bitmap.width;
		ch[c].y = face->glyph->bitmap.rows;
		ch[c].bx = face->glyph->bitmap_left;
		ch[c].by = face->glyph->bitmap_top;
		ch[c].adv = face->glyph->advance.x;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	
	glGenVertexArrays(1, &text_vao);
	glGenBuffers(1, &text_vbo);
	glBindVertexArray(text_vao);
	glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	textshader.Use();
	textshader.SetI("text", TUtoI(TEXT_TEXTURE_UNIT)); //GL_TEXTURE4

}

void DrawText(winfo_t* winfo, menuflags_t menu)
{
	glDisable(GL_CULL_FACE);

	glm::mat4 projection = glm::ortho(0.0f, (float)winfo->w, 0.0f, (float)winfo->h);

	textshader.Use();
	textshader.SetM4F("textProjection", glm::value_ptr(projection));

	vec3_c color(0.2f, 0.0f, 1.0f);
	DrawString("100", 25, 25, 0.5, &color);
	DrawString("100", 115, 25, 0.5, &color);

	vec3_c color2(0.2f, 1.0f, 0.0f);
	if(menu == MENU_NONE)
		DrawString("+", winfo->w / 2.0f, winfo->h / 2.0f, 0.5, &color2);

	vec3_c scolor(0.0f, 1.0f, 0.0f);
	char speedo[16];
	vec3_c projvel = in.vel;
	itoa((int)projvel[1], speedo, 10);
	DrawString(speedo, winfo->w / 2.0f, 50, 0.5f, &scolor); //y vel speedo

	projvel[1] = 0;
	itoa((int)projvel.len(), speedo, 10);

	if (projvel.len() > 320.0f)
	{
		scolor[0] = ( projvel.len() - 320.0f) / (600.0f - 320.0f);
		scolor[1] = 1.0f - scolor[0];
	}

	DrawString(speedo, winfo->w / 2.0f, 25, 0.5f, &scolor);

	glEnable(GL_CULL_FACE);
}

void DrawString(const char* str, float x, float y, float scale, vec3_c* color)
{
	//glUniform3f(glGetUniformLocation(s.Program, "textColor"), color.x, color.y, color.z);
	if (color)
	{
		glUniform3f(glGetUniformLocation(textshader.id, "textColor"), color->v[0], color->v[1], color->v[2]);
	}
	else
	{
		glUniform3f(glGetUniformLocation(textshader.id, "textColor"), 1.0f, 1.0f, 1.0f);
	}

	glActiveTexture(TEXT_TEXTURE_UNIT);
	glBindVertexArray(text_vao);

	for (int i = 0; str[i]; i++)
	{
		float xpos;
		float ypos;
		float w;
		float h;
		ftchar_t* c;

		c = &ch[str[i]];

		xpos = x + c->bx * scale;
		ypos = y - (c->y - c->by) * scale;

		w = c->x * scale;
		h = c->y * scale;

		// update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, c->id);
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (c->adv >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
};
