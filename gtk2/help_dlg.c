/* help_dlg.c
 *
 * $Id: help_dlg.c,v 1.2 2002/09/05 18:48:51 jmayer Exp $
 *
 * Laurent Deniel <deniel@worldnet.fr>
 *
 * Ethereal - Network traffic analyzer
 * By Gerald Combs <gerald@ethereal.com>
 * Copyright 2000 Gerald Combs
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
# include "config.h"
#endif

#include <gtk/gtk.h>

#ifdef NEED_SNPRINTF_H
# include "snprintf.h"
#endif

#include "help_dlg.h"
#include "prefs.h"
#include "globals.h"
#include "gtkglobals.h"
#include "main.h"
#include "ui_util.h"
#include <epan/proto.h>

typedef enum {
  OVERVIEW_HELP,
  PROTOCOL_HELP,
  DFILTER_HELP,
  CFILTER_HELP
} help_type_t;

static void help_close_cb(GtkWidget *w, gpointer data);
static void help_destroy_cb(GtkWidget *w, gpointer data);
static void insert_text(GtkWidget *w, char *buffer, int nchars);
static void set_help_text(GtkWidget *w, help_type_t type);

/*
 * Keep a static pointer to the current "Help" window, if any, so that
 * if somebody tries to do "Help->Help" while there's already a
 * "Help" window up, we just pop up the existing one, rather than
 * creating a new one.
 */
static GtkWidget *help_w = NULL;

/*
 * Keep static pointers to the text widgets as well.
 */
GtkWidget *overview_text, *proto_text, *dfilter_text, *cfilter_text;

void help_cb(GtkWidget *w _U_, gpointer data _U_)
{

  GtkWidget *main_vb, *bbox, *help_nb, *close_bt, *label, *txt_scrollw,
    *overview_vb,
    *proto_vb,
    *dfilter_vb,
    *cfilter_vb;

  if (help_w != NULL) {
    /* There's already a "Help" dialog box; reactivate it. */
    reactivate_window(help_w);
    return;
  }

  help_w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(help_w, "Ethereal Help window" );
  gtk_window_set_title(GTK_WINDOW(help_w), "Ethereal: Help");
  g_signal_connect(G_OBJECT(help_w), "destroy",
                   G_CALLBACK(help_destroy_cb), NULL);
  g_signal_connect(G_OBJECT(help_w), "realize",
                   G_CALLBACK(window_icon_realize_cb), NULL);
  gtk_widget_set_size_request(GTK_WIDGET(help_w), DEF_WIDTH * 2/3,
                              DEF_HEIGHT * 2/3);
  gtk_container_border_width(GTK_CONTAINER(help_w), 2);

  /* Container for each row of widgets */

  main_vb = gtk_vbox_new(FALSE, 1);
  gtk_container_border_width(GTK_CONTAINER(main_vb), 1);
  gtk_container_add(GTK_CONTAINER(help_w), main_vb);
  gtk_widget_show(main_vb);

  /* help topics container */

  help_nb = gtk_notebook_new();
  gtk_container_add(GTK_CONTAINER(main_vb), help_nb);

  /* Overview panel */

  overview_vb = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(overview_vb), 1);
  txt_scrollw = scrolled_window_new(NULL, NULL);
  gtk_box_pack_start(GTK_BOX(overview_vb), txt_scrollw, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(txt_scrollw),
				 GTK_POLICY_AUTOMATIC,
				 GTK_POLICY_AUTOMATIC);
  overview_text = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(overview_text), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(overview_text), GTK_WRAP_WORD);
  set_help_text(overview_text, OVERVIEW_HELP);
  gtk_container_add(GTK_CONTAINER(txt_scrollw), overview_text);
  gtk_widget_show(txt_scrollw);
  gtk_widget_show(overview_text);
  gtk_widget_show(overview_vb);
  label = gtk_label_new("Overview");
  gtk_notebook_append_page(GTK_NOTEBOOK(help_nb), overview_vb, label);

  /* protocol list */

  proto_vb = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(proto_vb), 1);

  txt_scrollw = scrolled_window_new(NULL, NULL);
  gtk_box_pack_start(GTK_BOX(proto_vb), txt_scrollw, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(txt_scrollw),
				 GTK_POLICY_AUTOMATIC,
				 GTK_POLICY_AUTOMATIC);
  proto_text = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(proto_text), FALSE);
  set_help_text(proto_text, PROTOCOL_HELP);
  gtk_container_add(GTK_CONTAINER(txt_scrollw), proto_text);
  gtk_widget_show(txt_scrollw);
  gtk_widget_show(proto_text);
  gtk_widget_show(proto_vb);
  label = gtk_label_new("Protocols");
  gtk_notebook_append_page(GTK_NOTEBOOK(help_nb), proto_vb, label);

  /* display filter help */
  /* X windows have a maximum size of 32767.  Since the height can easily
     exceed this, we have to jump through some hoops to have a functional
     vertical scroll bar. */

  dfilter_vb = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(dfilter_vb), 1);

  txt_scrollw = scrolled_window_new(NULL, NULL);
  gtk_box_pack_start(GTK_BOX(dfilter_vb), txt_scrollw, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(txt_scrollw),
				 GTK_POLICY_AUTOMATIC,
				 GTK_POLICY_AUTOMATIC);
  dfilter_text = gtk_text_view_new();
  if (prefs.gui_scrollbar_on_right) {
    gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(txt_scrollw),
                                      GTK_CORNER_TOP_LEFT);
  }
  else {
    gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW(txt_scrollw),
                                      GTK_CORNER_TOP_RIGHT);
  }
  gtk_text_view_set_editable(GTK_TEXT_VIEW(dfilter_text), FALSE);
  set_help_text(dfilter_text, DFILTER_HELP);
  gtk_container_add(GTK_CONTAINER(txt_scrollw), dfilter_text);
  gtk_widget_show(txt_scrollw);
  gtk_widget_show(dfilter_text);
  gtk_widget_show(dfilter_vb);
  label = gtk_label_new("Display Filters");
  gtk_notebook_append_page(GTK_NOTEBOOK(help_nb), dfilter_vb, label);

  /* capture filter help (this one has no horizontal scrollbar) */

  cfilter_vb = gtk_vbox_new(FALSE, 0);
  gtk_container_border_width(GTK_CONTAINER(cfilter_vb), 1);
  txt_scrollw = scrolled_window_new(NULL, NULL);
  gtk_box_pack_start(GTK_BOX(cfilter_vb), txt_scrollw, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(txt_scrollw),
				 GTK_POLICY_NEVER,
				 GTK_POLICY_AUTOMATIC);
  cfilter_text = gtk_text_view_new();
  gtk_text_view_set_editable(GTK_TEXT_VIEW(cfilter_text), FALSE);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(cfilter_text), GTK_WRAP_WORD);
  set_help_text(cfilter_text, CFILTER_HELP);
  gtk_container_add(GTK_CONTAINER(txt_scrollw), cfilter_text);
  gtk_widget_show(txt_scrollw);
  gtk_widget_show(cfilter_text);
  gtk_widget_show(cfilter_vb);
  label = gtk_label_new("Capture Filters");
  gtk_notebook_append_page(GTK_NOTEBOOK(help_nb), cfilter_vb, label);

  /* XXX add other help panels here ... */

  gtk_widget_show(help_nb);

  /* Buttons (only one for now) */

  bbox = gtk_hbox_new(FALSE, 1);
  gtk_box_pack_end(GTK_BOX(main_vb), bbox, FALSE, FALSE, 0);
  gtk_widget_show(bbox);
  close_bt = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
  g_signal_connect(G_OBJECT(close_bt), "clicked",
                   G_CALLBACK(help_close_cb), GTK_OBJECT(help_w));
  GTK_WIDGET_SET_FLAGS(close_bt, GTK_CAN_DEFAULT);
  gtk_container_add(GTK_CONTAINER(bbox), close_bt);
  gtk_widget_grab_default(close_bt);
  gtk_widget_show(close_bt);

  gtk_quit_add_destroy(gtk_main_level(), GTK_OBJECT(help_w));
  gtk_widget_show(help_w);

} /* help_cb */

static void help_close_cb(GtkWidget *w _U_, gpointer data)
{
  gtk_widget_destroy(GTK_WIDGET(data));
}

static void help_destroy_cb(GtkWidget *w _U_, gpointer data _U_)
{
  /* Note that we no longer have a Help window. */
  help_w = NULL;
}

static void insert_text(GtkWidget *w, char *buffer, int nchars)
{
  GtkTextBuffer *buf= gtk_text_view_get_buffer(GTK_TEXT_VIEW(w));
  GtkTextIter    iter;

  gtk_text_buffer_get_end_iter(buf, &iter);
  gtk_widget_modify_font(w, m_r_font);
  if (!g_utf8_validate(buffer, -1, NULL))
      printf(buffer);
  gtk_text_buffer_insert(buf, &iter, buffer, nchars);
}

static char *proto_help =
"The protocols (and packet types) currently supported by\n"
"Ethereal are the following:\n\n";

static char *dfilter_help =
"The following list shows all per-protocol fields that\n"
"can be used in a display filter:\n";

static char *cfilter_help =
"Packet capturing is performed with the pcap library. The capture filter "
"syntax follows the rules of this library.\nSo this syntax is different "
"from the display filter syntax: see manual page of tcpdump.\n"
#ifndef HAVE_LIBPCAP
"\nNote: packet capturing is not enabled in this version.\n";
#else
;
#endif

static char *overview_help =
"Ethereal is a GUI network protocol analyzer. It lets you interactively "
"browse packet data from a live network or from a previously saved capture "
"file.\n\nEthereal knows how to read libpcap capture files, including those "
"of tcpdump. In addition, Ethereal can read capture files from snoop "
"(including Shomiti) and atmsnoop, LanAlyzer, Sniffer (compressed or "
"uncompressed), Microsoft Network Monitor, AIX's iptrace, NetXray, "
"Sniffer Pro, RADCOM's WAN/LAN analyzer, Lucent/Ascend router debug output, "
"HP-UX's nettl, the dump output from Toshiba's ISDN routers, the output from "
"i4btrace from the ISDN4BSD project, and output in IPLog format from the "
"Cisco Secure Intrusion Detection System.\n\n"
"There is no need to tell Ethereal what type of file you are reading; it will "
"determine the file type by itself. Ethereal is also capable of reading any "
"of these file formats if they are compressed using gzip. Ethereal recognizes "
"this directly from the file; the '.gz' extension is not required for this "
"purpose.";

static void set_help_text(GtkWidget *w, help_type_t type)
{

#define BUFF_LEN 4096
#define B_LEN    256
  char buffer[BUFF_LEN];
  header_field_info *hfinfo;
  int i, len, maxlen = 0, maxlen2 = 0;
  const char *type_name;
  char blanks[B_LEN];
  int blks;
  void *cookie;

  memset(blanks, ' ', B_LEN - 1);
  blanks[B_LEN-1] = '\0';

  switch(type) {

  case OVERVIEW_HELP :
    insert_text(w, overview_help, -1);
    break;

  case PROTOCOL_HELP :
    /* first pass to know the maximum length of first field */
    for (i = proto_get_first_protocol(&cookie); i != -1;
         i = proto_get_next_protocol(&cookie)) {
        hfinfo = proto_registrar_get_nth(i);
        if ((len = strlen(hfinfo->abbrev)) > maxlen)
        maxlen = len;
    }
    maxlen++;

    insert_text(w, proto_help, strlen(proto_help));

    /* ok, display the correctly aligned strings */
    for (i = proto_get_first_protocol(&cookie); i != -1;
         i = proto_get_next_protocol(&cookie)) {
      hfinfo = proto_registrar_get_nth(i);
      blks = maxlen - strlen(hfinfo->abbrev);
      snprintf(buffer, BUFF_LEN, "%s%s%s\n",
	       hfinfo->abbrev,
	       &blanks[B_LEN - blks - 2],
	       hfinfo->name);
      insert_text(w, buffer, strlen(buffer));
    }
    break;

  case DFILTER_HELP  :

    /* XXX we should display hinfo->blurb instead of name (if not empty) */

    /* first pass to know the maximum length of first and second fields */
    for (i = 0; i < proto_registrar_n() ; i++) {
      if (!proto_registrar_is_protocol(i)) {
	hfinfo = proto_registrar_get_nth(i);
	if ((len = strlen(hfinfo->abbrev)) > maxlen)
	  maxlen = len;
	if ((len = strlen(hfinfo->name)) > maxlen2)
	  maxlen2 = len;
      }
    }
    maxlen++;
    maxlen2++;

    insert_text(w, dfilter_help, strlen(dfilter_help));

    for (i = 0; i < proto_registrar_n() ; i++) {
      hfinfo = proto_registrar_get_nth(i);
      if (proto_registrar_is_protocol(i)) {
	snprintf(buffer, BUFF_LEN, "\n%s:\n", hfinfo->name);
	insert_text(w, buffer, strlen(buffer));
      } else {

	type_name = ftype_pretty_name(hfinfo->type);
	snprintf(buffer, BUFF_LEN, "%s%s%s%s(%s)\n",
		 hfinfo->abbrev,
		 &blanks[B_LEN - (maxlen - strlen(hfinfo->abbrev)) - 2],
		 hfinfo->name,
		 &blanks[B_LEN - (maxlen2 - strlen(hfinfo->name)) - 2],
		 type_name);
	insert_text(w, buffer, strlen(buffer));
      }
    }
    break;
  case CFILTER_HELP :
    insert_text(w, cfilter_help, -1);
    break;
  default :
    g_assert_not_reached();
    break;
  } /* switch(type) */
} /* set_help_text */

static void clear_help_text(GtkWidget *w)
{
  GtkTextBuffer *buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w));

  gtk_text_buffer_set_text(buf, "", 0);
}

/* Redraw all the text widgets, to use a new font. */
void help_redraw(void)
{
  if (help_w != NULL) {
    clear_help_text(overview_text);
    set_help_text(overview_text, OVERVIEW_HELP);

    clear_help_text(proto_text);
    set_help_text(proto_text, PROTOCOL_HELP);

    clear_help_text(dfilter_text);
    set_help_text(dfilter_text, DFILTER_HELP);

    clear_help_text(cfilter_text);
    set_help_text(cfilter_text, CFILTER_HELP);
  }
}
