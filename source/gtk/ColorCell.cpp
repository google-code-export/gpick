/*
 * Copyright (c) 2009-2010, Albertas Vyšniauskas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of the software author nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ColorCell.h"
#include "../ColorObject.h"

static void custom_cell_renderer_color_init(CustomCellRendererColor *cellcolor);
static void custom_cell_renderer_color_class_init(CustomCellRendererColorClass *klass);
static void custom_cell_renderer_color_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *pspec);
static void custom_cell_renderer_color_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec);
static void custom_cell_renderer_color_finalize(GObject *gobject);
static void custom_cell_renderer_get_preferred_width(GtkCellRenderer *cell, GtkWidget *widget, gint *minimum_size, gint *natural_size);
static void custom_cell_renderer_get_preferred_height(GtkCellRenderer *cell, GtkWidget *widget, gint *minimum_size, gint *natural_size);
static void custom_cell_renderer_color_render(GtkCellRenderer *cell, cairo_t *cr, GtkWidget *widget, const GdkRectangle *background_area, const GdkRectangle *cell_area, GtkCellRendererState flags);

enum
{
  PROP_COLOR = 1,
};

static   gpointer parent_class;

GType custom_cell_renderer_color_get_type(void) {
	static GType cell_color_type = 0;

	if (cell_color_type == 0){
		static const GTypeInfo cell_color_info = { sizeof(CustomCellRendererColorClass), NULL, /* base_init */
		NULL, /* base_finalize */
		(GClassInitFunc) custom_cell_renderer_color_class_init, NULL, /* class_finalize */
		NULL, /* class_data */
		sizeof(CustomCellRendererColor), 0, /* n_preallocs */
		(GInstanceInitFunc) custom_cell_renderer_color_init, };

		cell_color_type = g_type_register_static(GTK_TYPE_CELL_RENDERER, "CustomCellRendererColor", &cell_color_info, (GTypeFlags) 0);
	}

	return cell_color_type;
}

static void custom_cell_renderer_color_init(CustomCellRendererColor *cellrenderercolor) {
/*	GTK_CELL_RENDERER(cellrenderercolor)->mode = GTK_CELL_RENDERER_MODE_INERT;
	GTK_CELL_RENDERER(cellrenderercolor)->xpad = 2;
	GTK_CELL_RENDERER(cellrenderercolor)->ypad = 2;*/

	cellrenderercolor->width = 32;
	cellrenderercolor->height = 16;
	cellrenderercolor->color = 0;
}

static void custom_cell_renderer_color_class_init(CustomCellRendererColorClass *klass) {
	GtkCellRendererClass *cell_class = GTK_CELL_RENDERER_CLASS(klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	parent_class = g_type_class_peek_parent(klass);
	object_class->finalize = custom_cell_renderer_color_finalize;

	object_class->get_property = custom_cell_renderer_color_get_property;
	object_class->set_property = custom_cell_renderer_color_set_property;

	cell_class->get_preferred_width = custom_cell_renderer_get_preferred_width;
	cell_class->get_preferred_height = custom_cell_renderer_get_preferred_height;
	cell_class->render = custom_cell_renderer_color_render;

	g_object_class_install_property(object_class, PROP_COLOR, g_param_spec_pointer("color", "Color", "ColorObject pointer", (GParamFlags) G_PARAM_READWRITE));
}


static void custom_cell_renderer_color_finalize(GObject *object) {
	(*G_OBJECT_CLASS(parent_class)->finalize)(object);
}

static void custom_cell_renderer_color_get_property(GObject *object, guint param_id, GValue *value, GParamSpec *psec) {
	CustomCellRendererColor *cellcolor = CUSTOM_CELL_RENDERER_COLOR(object);

	switch (param_id) {
	case PROP_COLOR:
		g_value_set_pointer(value, cellcolor->color);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, psec);
		break;
	}
}

static void custom_cell_renderer_color_set_property(GObject *object, guint param_id, const GValue *value, GParamSpec *pspec) {
	CustomCellRendererColor *cellcolor = CUSTOM_CELL_RENDERER_COLOR (object);

	switch (param_id) {
	case PROP_COLOR:
		cellcolor->color = (struct ColorObject*) g_value_get_pointer(value);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, param_id, pspec);
		break;
	}
}

GtkCellRenderer *
custom_cell_renderer_color_new() {
	return (GtkCellRenderer *) g_object_new(CUSTOM_TYPE_CELL_RENDERER_COLOR, NULL);
}

void custom_cell_renderer_color_set_size(GtkCellRenderer *cell,  gint width, gint height){
	CustomCellRendererColor *cellcolor=CUSTOM_CELL_RENDERER_COLOR(cell);
	cellcolor->width=width;
	cellcolor->height=height;
}

static void custom_cell_renderer_get_preferred_width(GtkCellRenderer *cell, GtkWidget *widget, gint *minimum_size, gint *natural_size)
{
	CustomCellRendererColor *cellcolor = CUSTOM_CELL_RENDERER_COLOR (cell);
	*minimum_size = 1;
	*natural_size = cellcolor->width;
}

static void custom_cell_renderer_get_preferred_height(GtkCellRenderer *cell, GtkWidget *widget, gint *minimum_size, gint *natural_size)
{
	CustomCellRendererColor *cellcolor = CUSTOM_CELL_RENDERER_COLOR (cell);
	*minimum_size = 1;
	*natural_size = cellcolor->height;
}

static void custom_cell_renderer_color_render(GtkCellRenderer *cell, cairo_t *cr, GtkWidget *widget, const GdkRectangle *background_area, const GdkRectangle *cell_area, GtkCellRendererState flags){
	CustomCellRendererColor *cellcolor = CUSTOM_CELL_RENDERER_COLOR (cell);
	cairo_rectangle(cr, cell_area->x, cell_area->y, cell_area->width, cell_area->height);
	Color c;
	color_object_get_color(cellcolor->color, &c);
	cairo_set_source_rgb(cr, c.rgb.red, c.rgb.green, c.rgb.blue);
	cairo_fill(cr);
}

