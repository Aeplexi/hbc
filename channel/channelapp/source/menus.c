#include "menus.h"

#include "controls.h"
#include "dialogs.h"
#include "i18n.h"
#include "theme.h"
#include "widgets.h"
#include "wiiinfo.h"

void menu_home(view *m_view, int x, int y, int yadd) {
	yadd = theme_gfx[THEME_BUTTON]->h*2/3;

	widget_button (&m_view->widgets[0], x, y, 0, BTN_NORMAL,
				   _("Settings"));
	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[1], x, y, 0, BTN_NORMAL,
				   _("System Info"));
	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[2], x, y, 0, BTN_NORMAL,
				   _("About"));
	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[3], x, y, 0, BTN_NORMAL,
				   _("Exit"));
}

void menu_exit(view *m_view, int x, int y, int yadd, bool bootmii_ios, bool priiloader) {
	int button_count = 3 + bootmii_ios + priiloader;

	if (button_count == 3) {
		button_count = 5;
		yadd = theme_gfx[THEME_BUTTON]->h/4;
		y += theme_gfx[THEME_BUTTON]->h + yadd;
	} else if (button_count == 2) {
		button_count = 4;
		yadd = theme_gfx[THEME_BUTTON]->h*2/3;
		y += theme_gfx[THEME_BUTTON]->h + yadd;
	} else {
		yadd = ((theme_gfx[THEME_BUTTON]->h*(6-button_count))/(button_count-1));
	}
	// if you need to have 1 button in a menu, something's seriously wrong with either you or your way of thinking

	if (bootmii_ios) {
		widget_button (&m_view->widgets[0], x, y, 0, BTN_NORMAL,
					   _("Launch BootMii IOS"));
		y += theme_gfx[THEME_BUTTON]->h + yadd;
	}

	if (priiloader) {
		widget_button (&m_view->widgets[1], x, y, 0, BTN_NORMAL,
					   _("Launch Priiloader"));
		y += theme_gfx[THEME_BUTTON]->h + yadd;
	}

	widget_button (&m_view->widgets[2], x, y, 0, BTN_NORMAL,
				   _("Exit to System Menu"));
	y += theme_gfx[THEME_BUTTON]->h + yadd;

	if (IS_VWII) {
		widget_button (&m_view->widgets[3], x, y, 0, BTN_NORMAL,
					   _("Reboot to Wii U Menu"));
	} else {
		widget_button (&m_view->widgets[3], x, y, 0, BTN_NORMAL,
					   _("Reboot"));
	}
	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[4], x, y, 0, BTN_NORMAL,
				   _("Shutdown"));
}

void menu_settings(view *m_view, int x, int y, int yadd) {
	yadd = theme_gfx[THEME_BUTTON]->h/4;

	widget_button (&m_view->widgets[0], x, y, 0, BTN_NORMAL,
				   _("Theme"));

	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[1], x, y, 0, BTN_NORMAL,
				   _("Sound"));

	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[2], x, y, 0, BTN_NORMAL,
				   _("Language"));

	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[3], x, y, 0, BTN_NORMAL,
				   _("Parental Controls"));

	y += theme_gfx[THEME_BUTTON]->h + yadd;

	widget_button (&m_view->widgets[4], x, y, 0, BTN_NORMAL,
				   _("Miscellaneous"));
}

void submenu_menutest(view *v) {
	u32 bd;

	widget_label (&v->widgets[2], 52, 76, 1, "Option 1",
				  theme_gfx[THEME_DIALOG]->w - 64, FA_LEFT, FA_DESCENDER, FONT_LABEL);
	widget_label (&v->widgets[3], 268, 76, 1, "Option 2",
				  theme_gfx[THEME_DIALOG]->w - 64, FA_LEFT, FA_DESCENDER, FONT_LABEL);
	widget_label (&v->widgets[4], 52, 148, 1, "Option 3",
				  theme_gfx[THEME_DIALOG]->w - 64, FA_LEFT, FA_DESCENDER, FONT_LABEL);
	widget_label (&v->widgets[5], 268, 148, 1, "Option 4",
				  theme_gfx[THEME_DIALOG]->w - 64, FA_LEFT, FA_DESCENDER, FONT_LABEL);
	widget_label (&v->widgets[6], 52, 220, 1, "Option 5",
				  theme_gfx[THEME_DIALOG]->w - 64, FA_LEFT, FA_DESCENDER, FONT_LABEL);
	widget_label (&v->widgets[7], 268, 220, 1, "Option 6",
				  theme_gfx[THEME_DIALOG]->w - 64, FA_LEFT, FA_DESCENDER, FONT_LABEL);

	widget_button (&v->widgets[8], 52, 80, 1, BTN_SMALL, "Button 1");
	widget_button (&v->widgets[9], 268, 80, 1, BTN_SMALL, "Button 2");
	widget_button (&v->widgets[10], 52, 152, 1, BTN_SMALL, "Button 3");
	widget_button (&v->widgets[11], 268, 152, 1, BTN_SMALL, "Button 4");
	widget_button (&v->widgets[12], 52, 224, 1, BTN_SMALL, "Button 5");
	widget_button (&v->widgets[13], 268, 224, 1, BTN_SMALL, "Button 6");


	widget_button (&v->widgets[14], 32,
				   theme_gfx[THEME_DIALOG]->h -
				   theme_gfx[THEME_BUTTON_SMALL]->h - 16 , 1, BTN_SMALL,
				   caption_ok);
	widget_button (&v->widgets[15], theme_gfx[THEME_DIALOG]->w -
	theme_gfx[THEME_BUTTON_SMALL]->w - 32,
	theme_gfx[THEME_DIALOG]->h -
	theme_gfx[THEME_BUTTON_SMALL]->h - 16 , 1, BTN_SMALL,
	caption_back);

	view_set_focus (v, 15);

	dialog_fade (v, true);

	while (true) {
		view_plot (v, DIALOG_MASK_COLOR, &bd, NULL, NULL);

		if (bd & PADS_LEFT)
			view_set_focus_prev (v);

		if (bd & PADS_RIGHT)
			view_set_focus_next (v);

		if (bd & PADS_UP)
			if (v->focus == view_move_focus(v, -2))
				view_move_focus(v, -4);

		if (bd & PADS_DOWN)
			if (v->focus == view_move_focus(v, 2))
				view_move_focus(v, 4);

		if (bd & PADS_B)
			break;

		if ((bd & PADS_A) && (v->focus != -1)) {
			if (v->focus >= 14)
				break;
		}
	}

	if ((bd & PADS_A) && (v->focus == 14))
		gprintf("confirm handler here");
}
