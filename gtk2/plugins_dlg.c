/* plugins_dlg.c
 * Dialog boxes for plugins
 *
 * $Id: plugins_dlg.c,v 1.2 2002/09/05 18:48:51 jmayer Exp $
 *
 * Ethereal - Network traffic analyzer
 * By Gerald Combs <gerald@ethereal.com>
 * Copyright 1999 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

#include <epan/plugins.h>
#include "dlg_utils.h"

#ifdef HAVE_PLUGINS

static void plugins_close_cb(GtkWidget *, gpointer);
static void plugins_scan(GtkListStore *);

enum
{
    COLUMN_NAME,
    COLUMN_VERSION,
    NUM_COLUMNS
};

void
tools_plugins_cmd_cb(GtkWidget *widget _U_, gpointer data _U_)
{
    GtkWidget    *plugins_window;
    GtkWidget    *main_vbox;
    GtkWidget    *main_frame;
    GtkWidget    *frame_hbox;
    GtkWidget    *scrolledwindow;
    GtkWidget    *plugins_list;
    GtkWidget    *frame_vbnbox;
    GtkWidget    *main_hbnbox;
    GtkWidget    *close_bn;
    gchar        *titles[] = {"Name", "Version"};
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    plugins_window = dlg_window_new("Ethereal: Plugins");

    main_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(plugins_window), main_vbox);
    gtk_widget_show(main_vbox);

    main_frame = gtk_frame_new("Plugins List");
    gtk_box_pack_start(GTK_BOX(main_vbox), main_frame, TRUE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(main_frame), 10);
    gtk_widget_show(main_frame);

    frame_hbox = gtk_hbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(main_frame), frame_hbox);
    gtk_container_set_border_width(GTK_CONTAINER(frame_hbox), 5);
    gtk_widget_show(frame_hbox);

    scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(frame_hbox), scrolledwindow, TRUE, TRUE, 0);
    gtk_widget_set_size_request(scrolledwindow, 400, 150);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_show(scrolledwindow);

    store = gtk_list_store_new(NUM_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
    plugins_scan(store);
    plugins_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(plugins_list), TRUE);
    gtk_tree_view_set_search_column(GTK_TREE_VIEW(plugins_list), 0);
    g_object_unref(G_OBJECT(store));
    gtk_container_add(GTK_CONTAINER(scrolledwindow), plugins_list);
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text",
                                                      COLUMN_NAME, NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_NAME);
    gtk_tree_view_append_column(GTK_TREE_VIEW(plugins_list), column);
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Version", renderer,
                                                      "text", COLUMN_VERSION,
                                                      NULL);
    gtk_tree_view_column_set_sort_column_id(column, COLUMN_VERSION);
    gtk_tree_view_append_column(GTK_TREE_VIEW(plugins_list), column);
    gtk_widget_show(plugins_list);

    frame_vbnbox = gtk_vbutton_box_new();
    gtk_box_pack_start(GTK_BOX(frame_hbox), frame_vbnbox, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(frame_vbnbox), 20);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(frame_vbnbox),
                              GTK_BUTTONBOX_SPREAD);
    gtk_widget_show(frame_vbnbox);

    main_hbnbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(main_vbox), main_hbnbox, FALSE, TRUE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(main_hbnbox), 10);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(main_hbnbox),
                              GTK_BUTTONBOX_SPREAD);
    gtk_widget_show(main_hbnbox);

    close_bn = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    gtk_container_add(GTK_CONTAINER(main_hbnbox), close_bn);
    gtk_widget_show(close_bn);
    g_signal_connect(G_OBJECT(close_bn), "clicked",
                     G_CALLBACK(plugins_close_cb), GTK_OBJECT(plugins_window));

    gtk_widget_show(plugins_window);

}

/*
 * Fill the list widget with a list of the plugin modules.
 */
static void
plugins_scan(GtkListStore *store)
{
    plugin     *pt_plug;
    GtkTreeIter iter;

    pt_plug = plugin_list;
    while (pt_plug)
    {
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, COLUMN_NAME, pt_plug->name,
                           COLUMN_VERSION, pt_plug->version, -1);
	pt_plug = pt_plug->next;
    }
}

static void
plugins_close_cb(GtkWidget *close_bt _U_, gpointer parent_w)
{
    gtk_grab_remove(GTK_WIDGET(parent_w));
    gtk_widget_destroy(GTK_WIDGET(parent_w));
}
#endif
