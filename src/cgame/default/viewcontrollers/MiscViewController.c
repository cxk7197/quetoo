/*
 * Copyright(c) 1997-2001 id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quetoo.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "cg_local.h"

#include "MiscViewController.h"

#include "CvarSelect.h"
#include "VideoModeSelect.h"

#define _Class _MiscViewController

#pragma mark - Actions & Delegates

/**
 * @brief ActionFunction for Apply Button.
 */
static void applyAction(Control *control, const SDL_Event *event, ident sender, ident data) {
	cgi.Cbuf("r_restart\n");
}

#pragma mark - ViewController

/**
 * @see ViewController::loadView(ViewController *)
 */
static void loadView(ViewController *self) {

	super(ViewController, self, loadView);

	MenuViewController *this = (MenuViewController *) self;

	StackView *columns = $(alloc(StackView), initWithFrame, NULL);

	columns->axis = StackViewAxisHorizontal;
	columns->spacing = DEFAULT_PANEL_SPACING;

	{
		StackView *column = $(alloc(StackView), initWithFrame, NULL);
		column->spacing = DEFAULT_PANEL_SPACING;

		{
			Box *box = $(alloc(Box), initWithFrame, NULL);
			$(box->label, setText, "MISC");

			StackView *stackView = $(alloc(StackView), initWithFrame, NULL);

			Cg_CvarSliderInput((View *) stackView, "Blegh", "s_music_volume", 0.0, 1.0, 0.0);

			$((View *) box, addSubview, (View *) stackView);
			release(stackView);

			$((View *) column, addSubview, (View *) box);
			release(box);
		}

		$((View *) columns, addSubview, (View *) column);
		release(column);
	}

	$((View *) this->panel->contentView, addSubview, (View *) columns);
	release(columns);

	this->panel->accessoryView->view.hidden = false;
	Cg_Button((View *) this->panel->accessoryView, "Apply", applyAction, self, NULL);
}

#pragma mark - Class lifecycle

/**
 * @see Class::initialize(Class *)
 */
static void initialize(Class *clazz) {

	((ViewControllerInterface *) clazz->def->interface)->loadView = loadView;
}

/**
 * @fn Class *MiscViewController::_MiscViewController(void)
 * @memberof MiscViewController
 */
Class *_MiscViewController(void) {
	static Class clazz;
	static Once once;

	do_once(&once, {
		clazz.name = "MiscViewController";
		clazz.superclass = _MenuViewController();
		clazz.instanceSize = sizeof(MiscViewController);
		clazz.interfaceOffset = offsetof(MiscViewController, interface);
		clazz.interfaceSize = sizeof(MiscViewControllerInterface);
		clazz.initialize = initialize;
	});

	return &clazz;
}

#undef _Class
