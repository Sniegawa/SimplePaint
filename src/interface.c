#include "interface.h"

#include "colors.h"
#include "draw.h"

#include <nuklear.h>
#include <demo/glfw_opengl4/nuklear_glfw_gl4.h>

void DrawToolbox(APP_STATE* state)
{
	struct nk_context* ctx = state->ctx;

	if (nk_group_begin(ctx, "Toolbox", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		// Tools

		nk_layout_row_dynamic(ctx, 200, 1);
		if (nk_group_begin(ctx, "Tools", 0))
		{
			// grid of 50x50 boxes 2 cols wide
			nk_layout_row_static(ctx, 50, 50, 2);

			if (nk_button_label(ctx, "Pencil"))
				state->SelectedTool = Pencil;

			if (nk_button_label(ctx, "Brush"))
				state->SelectedTool = Brush;

			if (nk_button_label(ctx, "Eraser"))
				state->SelectedTool = Eraser;

			if (nk_button_label(ctx, "Picker"))
				state->SelectedTool = ColorPicker;

			nk_layout_row_dynamic(ctx, 25, 1);
			nk_property_int(ctx, "Size:", 1, &state->BrushSize, 20, 1, 0.5f);
			nk_group_end(ctx);
		}

		// Color
		nk_layout_row_dynamic(ctx, 300, 1);
		if (nk_group_begin(ctx, "Colors", NK_WINDOW_NO_SCROLLBAR))
		{
			struct nk_rect r = nk_window_get_content_region(ctx);

			// Make color picker square
			float picker_size = r.w < r.h ? r.w : r.h;
			if (picker_size > 120)
				picker_size = 120;

			nk_layout_row_static(ctx, picker_size, picker_size, 1);

			Color fg = state->Palette.foreground;
			struct nk_colorf color = nk_color_picker(ctx, ColorToNKf(fg), NK_RGB);

			state->Palette.foreground = NKftoColor(color);

			//Grid for palette colors
			nk_layout_row_dynamic(ctx, 32, 4);

			Color* Palette = state->Palette.colorsArray;

			for (int i = 0; i < PALETTE_SIZE; i++)
			{
				struct nk_color nkc = ColorToNK(Palette[i]);

				nk_style_push_style_item(ctx, &ctx->style.button.normal, nk_style_item_color(nkc));
				nk_style_push_style_item(ctx, &ctx->style.button.hover, nk_style_item_color(nkc));
				nk_style_push_style_item(ctx, &ctx->style.button.active, nk_style_item_color(nkc));

				struct nk_rect bounds;

				bounds = nk_widget_bounds(ctx);
				nk_button_label(ctx, "");

				if (nk_input_mouse_clicked(&ctx->input, NK_BUTTON_LEFT, bounds))
				{
					state->Palette.foreground = Palette[i];
				}

				if (nk_input_mouse_clicked(&ctx->input, NK_BUTTON_RIGHT, bounds))
				{
					Palette[i] = state->Palette.foreground;
				}

				if (nk_input_mouse_clicked(&ctx->input, NK_BUTTON_MIDDLE, bounds))
				{
					Palette[i] = (Color){ 255,255,255 };
				}

				nk_style_pop_style_item(ctx);
				nk_style_pop_style_item(ctx);
				nk_style_pop_style_item(ctx);

			}

			nk_group_end(ctx);
		}


		nk_group_end(ctx);
	}

}

void DrawViewport(APP_STATE* state)
{
	struct nk_context* ctx = state->ctx;
	Image* image = state->CurrentImage;

	if (nk_group_begin(ctx, "Viewport", NK_WINDOW_BORDER | NK_WINDOW_TITLE))
	{
		struct nk_rect content = nk_window_get_content_region(ctx);

		float pad_x = (content.w - image->Width) * 0.5f;
		float pad_y = (content.h - image->Height) * 0.5f;

		if (pad_x < 0) pad_x = 0;
		if (pad_y < 0) pad_y = 0;

		nk_layout_row_dynamic(ctx, 0.5 * pad_y, 1);
		nk_spacing(ctx, 1);


		nk_layout_row_begin(ctx, NK_STATIC, image->Height, 3);

		nk_layout_row_push(ctx, pad_x);
		nk_spacing(ctx, 1);
		nk_layout_row_push(ctx, image->Width);
		struct nk_rect bound = nk_widget_bounds(ctx);
		nk_image(ctx, image->handle);
		nk_layout_row_push(ctx, pad_x);

		nk_layout_row_end(ctx);

		if (nk_input_is_mouse_down(&ctx->input, NK_BUTTON_LEFT))
		{

			struct nk_vec2 mouse = ctx->input.mouse.pos;

			if (mouse.x >= bound.x && mouse.x < bound.x + bound.w && mouse.y >= bound.y && mouse.y < bound.y + bound.h)
			{
				int localX = (int)mouse.x - bound.x;
				int localY = (int)mouse.y - bound.y;

				if (localX >= 0 && localY >= 0 && localX < image->Width && localY < image->Height)
				{
					int index = (localY * image->Width + localX) * 3;
					int R = image->Data[index];
					int G = image->Data[index + 1];
					int B = image->Data[index + 2];

					switch (state->SelectedTool)
					{
					case ColorPicker:
						state->Palette.foreground = (Color){ R,G,B };
						break;
					case Pencil:
						DrawPencil(state, localX, localY, state->Palette.foreground, 1);
						break;
					case Eraser:
						DrawPencil(state, localX, localY, state->Palette.background, state->BrushSize);
						break;
					case Brush:
						DrawPencil(state, localX, localY, state->Palette.foreground, state->BrushSize);
					}
				}
				state->LastMouseX = localX;
				state->LastMouseY = localY;
			}
		}
		else
		{
			state->LastMouseX = -1;
			state->LastMouseY = -1;
		}
		nk_group_end(ctx);
	}

}

void DrawMenu(APP_STATE* state)
{
	struct nk_context* ctx = state->ctx;

	static bool AboutFlag = false;
	nk_menubar_begin(ctx);

	nk_layout_row_static(ctx, 10, 50, 2);

	if (nk_menu_begin_label(ctx, "File", NK_TEXT_LEFT, nk_vec2(150, 200)))
	{
		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_menu_item_label(ctx, "New", NK_TEXT_LEFT))
			state->NewFileFlag = true;

		if (nk_menu_item_label(ctx, "Open", NK_TEXT_LEFT))
			APP_OPEN(state);
		if (nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT))
		{
			if (!state->ShouldCreateFile)
				APP_SAVE(state);
			else
				APP_SAVEAS(state);
		}
		if (nk_menu_item_label(ctx, "Save as", NK_TEXT_LEFT))
			APP_SAVEAS(state);
		nk_menu_end(ctx);
	}

	if (nk_menu_begin_label(ctx, "Help", NK_TEXT_LEFT, nk_vec2(150, 200)))
	{
		nk_layout_row_dynamic(ctx, 25, 1);
		if (nk_menu_item_label(ctx, "About", NK_TEXT_LEFT))
			AboutFlag = true;
		nk_menu_end(ctx);
	}

	nk_menubar_end(ctx);

	if (state->NewFileFlag)
	{
		if (nk_popup_begin(ctx, NK_POPUP_STATIC, "New image", NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR, nk_rect(100, 100, 220, 180)))
		{
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, "Create new image", NK_TEXT_CENTERED);

			static int width = 200, height = 200;
			nk_property_int(ctx, "Width", 1, &width, 4096, 1, 0.5f);
			nk_property_int(ctx, "Height", 1, &height, 4096, 1, 0.5f);


			if (nk_button_label(ctx, "Create"))
			{
				FreeImage(state->CurrentImage);

				state->CurrentImage = CreateBlankImage(width, height);
				state->CurrentPath = "";
				state->ShouldCreateFile = true;
				nk_popup_close(ctx);
				state->NewFileFlag = false;
			}
			if (nk_button_label(ctx, "Close"))
			{
				nk_popup_close(ctx);
				state->NewFileFlag = false;
			}
			nk_popup_end(ctx);
		}

	}

	if (AboutFlag)
	{
		if (nk_popup_begin(ctx, NK_POPUP_STATIC, "About", NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_NO_SCROLLBAR, nk_rect(0, 0, 200, 150)))
		{
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_label(ctx, "About", NK_TEXT_CENTERED);

			nk_label(ctx, "Author : Mikolaj Lisowski", NK_TEXT_CENTERED);

			nk_label(ctx, "Version : 1.0.0", NK_TEXT_CENTERED);

			if (nk_button_label(ctx, "Close"))
			{
				nk_popup_close(ctx);
				AboutFlag = false;
			}
			nk_popup_end(ctx);
		}
	}

}