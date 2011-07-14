/*
 * Copyright (c) 2009-2011, Albertas Vyšniauskas
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

#include "uiTransformations.h"

#include "Converter.h"
#include "uiUtilities.h"
#include "DynvHelpers.h"
#include "GlobalStateStruct.h"

#include "transformation/Chain.h"
#include "transformation/Factory.h"
#include "transformation/ColorVisionDeficiency.h"

#include <iostream>
using namespace std;

typedef enum{
	TRANSFORMATIONS_HUMAN_NAME = 0,
	TRANSFORMATIONS_TRANSFORMATION_PTR,
	TRANSFORMATIONS_N_COLUMNS
}TransformationsColumns;

typedef enum{
	AVAILABLE_TRANSFORMATIONS_HUMAN_NAME = 0,
	AVAILABLE_TRANSFORMATIONS_NAME,
	AVAILABLE_TRANSFORMATIONS_N_COLUMNS
}AvailableTransformationsColumns;

typedef struct TransformationsArgs{
	GtkWidget *list;
	GtkWidget *config_vbox;

	GtkWidget *enabled;

	transformation::Transformation *transformation;
	boost::shared_ptr<transformation::Configuration> configuration;

	struct dynvSystem *params;
	struct dynvSystem *transformations_params;
	GlobalState *gs;
}TransformationsArgs;

static void tranformations_update_row(GtkTreeModel *model, GtkTreeIter *iter1, transformation::Transformation *transformation, TransformationsArgs *args) {
	gtk_list_store_set(GTK_LIST_STORE(model), iter1,
			TRANSFORMATIONS_HUMAN_NAME, transformation->getReadableName().c_str(),
			TRANSFORMATIONS_TRANSFORMATION_PTR, transformation,
			-1);
}

static void available_tranformations_update_row(GtkTreeModel *model, GtkTreeIter *iter1, transformation::Factory::TypeInfo *type_info, TransformationsArgs *args) {
	gtk_list_store_set(GTK_LIST_STORE(model), iter1,
			AVAILABLE_TRANSFORMATIONS_HUMAN_NAME, type_info->human_name,
			AVAILABLE_TRANSFORMATIONS_NAME, type_info->name,
			-1);
}

static GtkWidget* available_transformations_list_new(TransformationsArgs *args)
{
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	GtkWidget *view;

	view = gtk_tree_view_new();
	args->list = view;

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), 1);

	store = gtk_list_store_new(AVAILABLE_TRANSFORMATIONS_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_sizing(col,GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_resizable(col, 1);
	gtk_tree_view_column_set_title(col, "Name");
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, true);
	gtk_tree_view_column_add_attribute(col, renderer, "text", AVAILABLE_TRANSFORMATIONS_HUMAN_NAME);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(store));
	g_object_unref(GTK_TREE_MODEL(store));

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);

	return view;
}

static GtkWidget* transformations_list_new(TransformationsArgs *args)
{
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	GtkWidget *view;

	view = gtk_tree_view_new();
	args->list = view;

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), 1);

	store = gtk_list_store_new(TRANSFORMATIONS_N_COLUMNS, G_TYPE_STRING, G_TYPE_POINTER);

	col = gtk_tree_view_column_new();
	gtk_tree_view_column_set_sizing(col,GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	gtk_tree_view_column_set_resizable(col, 1);
	gtk_tree_view_column_set_title(col, "Name");
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(col, renderer, true);
	gtk_tree_view_column_add_attribute(col, renderer, "text", TRANSFORMATIONS_HUMAN_NAME);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);

	gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(store));
	g_object_unref(GTK_TREE_MODEL(store));

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(view), true);

	return view;
}

static void apply_configuration(TransformationsArgs *args){
	if (args->configuration && args->transformation){
		struct dynvHandlerMap *handler_map = dynv_system_get_handler_map(args->gs->params);
		struct dynvSystem *dv = dynv_system_create(handler_map);
		args->configuration->applyConfig(dv);
		args->transformation->deserialize(dv);
		dynv_handler_map_release(handler_map);
		dynv_system_release(dv);
	}
}

static void transformation_chain_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, TransformationsArgs *args) {
	GtkTreeModel* model;
	GtkTreeIter iter;

	model = gtk_tree_view_get_model(tree_view);
	gtk_tree_model_get_iter(model, &iter, path);

	transformation::Transformation *transformation;
	gtk_tree_model_get(model, &iter, TRANSFORMATIONS_TRANSFORMATION_PTR, &transformation, -1);

	if (transformation){
		if (args->configuration){
			gtk_container_remove(GTK_CONTAINER(args->config_vbox), args->configuration->getWidget());
			apply_configuration(args);
		}
		args->configuration = transformation->getConfig();
		args->transformation = transformation;
		gtk_box_pack_start(GTK_BOX(args->config_vbox), args->configuration->getWidget(), true, true, 0);
	}
}


void dialog_transformations_show(GtkWindow* parent, GlobalState* gs)
{
	TransformationsArgs *args = new TransformationsArgs;
	args->gs = gs;
	args->params = dynv_get_dynv(args->gs->params, "gpick");
	args->transformations_params = dynv_get_dynv(args->gs->params, "gpick.transformations");
	args->transformation = 0;

	GtkWidget *dialog = gtk_dialog_new_with_buttons("Transformations", parent, GtkDialogFlags(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);

	gtk_window_set_default_size(GTK_WINDOW(dialog), dynv_get_int32_wd(args->params, "transformations.window.width", -1), dynv_get_int32_wd(args->params, "transformations.window.height", -1));

	gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);


	GtkWidget *widget;
	GtkWidget* vbox = gtk_vbox_new(false, 5);
	GtkWidget *vbox2 = gtk_vbox_new(false, 5);

	args->enabled = widget = gtk_check_button_new_with_mnemonic ("_Enabled");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), dynv_get_bool_wd(args->transformations_params, "enabled", false));
	gtk_box_pack_start(GTK_BOX(vbox), args->enabled, false, false, 0);

	GtkWidget* hbox = gtk_hbox_new(false, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, true, true, 0);

	GtkWidget *list, *scrolled;
	GtkTreeIter iter1;
	GtkTreeModel *model;

	list = available_transformations_list_new(args);
	scrolled = gtk_scrolled_window_new(0, 0);
	gtk_container_add(GTK_CONTAINER(scrolled), list);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox2), scrolled, true, true, 0);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
	vector<transformation::Factory::TypeInfo> types = transformation::Factory::getAllTypes();
	for (int i = 0; i != types.size(); i++){
		gtk_list_store_append(GTK_LIST_STORE(model), &iter1);
		available_tranformations_update_row(model, &iter1, &types[i], args);
	}

	list = transformations_list_new(args);
	g_signal_connect(G_OBJECT(list), "row-activated", G_CALLBACK(transformation_chain_row_activated), args);
	scrolled = gtk_scrolled_window_new(0, 0);
	gtk_container_add(GTK_CONTAINER(scrolled), list);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox2), scrolled, true, true, 0);

	gtk_box_pack_start(GTK_BOX(hbox), vbox2, true, true, 0);


	args->config_vbox = gtk_vbox_new(false, 5);
	gtk_box_pack_start(GTK_BOX(hbox), args->config_vbox, true, true, 0);

	transformation::Chain *chain = static_cast<transformation::Chain*>(dynv_get_pointer_wdc(args->gs->params, "TransformationChain", 0));

	/*boost::shared_ptr<transformation::ColorVisionDeficiency> color_vision_deficiency = boost::shared_ptr<transformation::ColorVisionDeficiency>(new transformation::ColorVisionDeficiency(transformation::ColorVisionDeficiency::DEUTERANOMALY, 0.8));
	chain->clear();
	chain->add(color_vision_deficiency);
  */

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
	for (transformation::Chain::TransformationList::iterator i = chain->getAll().begin(); i != chain->getAll().end(); i++){
		gtk_list_store_append(GTK_LIST_STORE(model), &iter1);
		tranformations_update_row(model, &iter1, (*i).get(), args);
	}

	gtk_widget_show_all(vbox);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), vbox, true, true, 5);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {

		apply_configuration(args);

		GtkTreeIter iter;
		GtkListStore *store;
		gboolean valid;

		store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
		valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);


    bool enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(args->enabled));
		dynv_set_bool(args->transformations_params, "enabled", enabled);
		chain->setEnabled(enabled);

		/*unsigned int count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL);
		if (count > 0){
			char** name_array = new char*[count];
			bool* copy_array = new bool[count];
			bool* paste_array = new bool[count];
			unsigned int i = 0;

			while (valid){
				Converter* converter;
				gboolean copy, paste;
				gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, CONVERTERLIST_CONVERTER_PTR, &converter, CONVERTERLIST_COPY, &copy, CONVERTERLIST_PASTE, &paste, -1);

				name_array[i] = converter->function_name;
				copy_array[i] = copy;
				paste_array[i] = paste;

				converter->copy = copy;
				converter->paste = paste;

				valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
				++i;
			}

			converters_reorder(converters, (const char**)name_array, count);

			dynv_set_string_array(args->params, "converters.names", (const char**)name_array, count);
			dynv_set_bool_array(args->params, "converters.copy", copy_array, count);
			dynv_set_bool_array(args->params, "converters.paste", paste_array, count);

			delete [] name_array;
			delete [] copy_array;
			delete [] paste_array;
		}else{
			converters_reorder(converters, 0, 0);

			dynv_set_string_array(args->params, "converters.names", 0, 0);
			dynv_set_bool_array(args->params, "converters.copy", 0, 0);
			dynv_set_bool_array(args->params, "converters.paste", 0, 0);

		} */
	}
	gint width, height;
	gtk_window_get_size(GTK_WINDOW(dialog), &width, &height);
	dynv_set_int32(args->params, "transformations.window.width", width);
	dynv_set_int32(args->params, "transformations.window.height", height);

	gtk_widget_destroy(dialog);

	dynv_system_release(args->transformations_params);
	dynv_system_release(args->params);
	delete args;
}




