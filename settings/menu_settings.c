/*  xfce4
 *  
 *  Copyright (C) 2004 Brian Tarricone <bjt23@cornell.edu>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libxfcegui4/libxfcegui4.h>

#include "xfdesktop-common.h"
#include "settings_common.h"

enum {
    OPT_SHOWWL = 1,
    OPT_SHOWWLI,
#ifdef USE_DESKTOP_MENU
    OPT_SHOWDM,
    OPT_SHOWDMI,
#endif
};

/* globals */
static gboolean show_windowlist = TRUE;
static gboolean show_windowlist_icons = TRUE;
#ifdef USE_DESKTOP_MENU
static gboolean show_desktopmenu = TRUE;
static gboolean show_desktopmenu_icons = TRUE;
#endif
#ifdef ENABLE_DESKTOP_ICONS
/* !! keep in sync with src/xfce-desktop.c !! */
typedef enum
{
    XFCE_DESKTOP_ICON_STYLE_NONE = 0,
    XFCE_DESKTOP_ICON_STYLE_WINDOWS,
    XFCE_DESKTOP_ICON_STYLE_FILES,
} XfceDesktopIconStyle;
static XfceDesktopIconStyle desktop_icon_style = XFCE_DESKTOP_ICON_STYLE_WINDOWS;
#endif

static void
set_chk_option(GtkWidget *w, gpointer user_data)
{
    BackdropDialog *bd = (BackdropDialog *)user_data;
    guint opt;
    
    opt = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(w), "xfce-chknum"));
    switch(opt) {
        case OPT_SHOWWL:
            show_windowlist = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
            mcs_manager_set_int(bd->plugin->manager, "showwl", BACKDROP_CHANNEL,
                    show_windowlist ? 1 : 0);
            gtk_widget_set_sensitive(bd->frame_wl1, show_windowlist);
            break;
        case OPT_SHOWWLI:
            show_windowlist_icons = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
            mcs_manager_set_int(bd->plugin->manager, "showwli", BACKDROP_CHANNEL,
                    show_windowlist_icons ? 1 : 0);
            break;
#ifdef USE_DESKTOP_MENU
        case OPT_SHOWDM:
            show_desktopmenu = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
            mcs_manager_set_int(bd->plugin->manager, "showdm", BACKDROP_CHANNEL,
                    show_desktopmenu ? 1 : 0);
            gtk_widget_set_sensitive(bd->frame_dm1, show_desktopmenu);
            break;
        case OPT_SHOWDMI:
            show_desktopmenu_icons = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));
            mcs_manager_set_int(bd->plugin->manager, "showdmi", BACKDROP_CHANNEL,
                    show_desktopmenu_icons ? 1 : 0);
            break;
#endif
        default:
            g_warning("xfdesktop menu: got invalid checkbox ID");
            return;
    }
    
    mcs_manager_notify(bd->plugin->manager, BACKDROP_CHANNEL);
}

static void
set_di_option(GtkComboBox *combo,
              gpointer user_data)
{
    BackdropDialog *bd = (BackdropDialog *)user_data;
    
    desktop_icon_style = gtk_combo_box_get_active(combo);
    mcs_manager_set_int(bd->plugin->manager, "desktopiconstyle",
                        BACKDROP_CHANNEL, desktop_icon_style);
    mcs_manager_notify(bd->plugin->manager, BACKDROP_CHANNEL);
}

void
init_menu_settings(McsPlugin *plugin)
{
    McsSetting *setting;
    
    setting = mcs_manager_setting_lookup(plugin->manager, "showwl",
            BACKDROP_CHANNEL);
    if(setting)
        show_windowlist = setting->data.v_int == 0 ? FALSE : TRUE;
    else
        mcs_manager_set_int(plugin->manager, "showwl", BACKDROP_CHANNEL, 1);
    
    setting = mcs_manager_setting_lookup(plugin->manager, "showwli",
            BACKDROP_CHANNEL);
    if(setting)
        show_windowlist_icons = setting->data.v_int == 0 ? FALSE : TRUE;
    else
        mcs_manager_set_int(plugin->manager, "showwli", BACKDROP_CHANNEL, 1);

#ifdef USE_DESKTOP_MENU
    setting = mcs_manager_setting_lookup(plugin->manager, "showdm",
            BACKDROP_CHANNEL);
    if(setting)
        show_desktopmenu = setting->data.v_int == 0 ? FALSE : TRUE;
    else
        mcs_manager_set_int(plugin->manager, "showdm", BACKDROP_CHANNEL, 1);
    
    setting = mcs_manager_setting_lookup(plugin->manager, "showdmi",
            BACKDROP_CHANNEL);
    if(setting)
        show_desktopmenu_icons = setting->data.v_int == 0 ? FALSE : TRUE;
    else
        mcs_manager_set_int(plugin->manager, "showdmi", BACKDROP_CHANNEL, 1);
#endif
    
#ifdef ENABLE_DESKTOP_ICONS
    setting = mcs_manager_setting_lookup(plugin->manager, "desktopiconstyle",
            BACKDROP_CHANNEL);
    if(setting) {
        desktop_icon_style = setting->data.v_int;
# ifdef HAVE_THUNAR_VFS
        if(desktop_icon_style > XFCE_DESKTOP_ICON_STYLE_FILES)
# else
        if(desktop_icon_style > XFCE_DESKTOP_ICON_STYLE_WINDOWS)
# endif
            desktop_icon_style = XFCE_DESKTOP_ICON_STYLE_WINDOWS;
    } else
        mcs_manager_set_int(plugin->manager, "desktopiconstyle",
                            BACKDROP_CHANNEL, desktop_icon_style);
#endif
}

#if USE_DESKTOP_MENU
static void
_edit_menu_cb(GtkWidget *w, gpointer user_data)
{
    GError *err = NULL;
    
    if(!xfce_exec(BINDIR "/xfce4-menueditor", FALSE, FALSE, NULL)
            && !xfce_exec("xfce4-menueditor", FALSE, FALSE, &err))
    {
        xfce_warn(_("Unable to launch xfce4-menueditor: %s"), err->message);
        g_error_free(err);
    }
}
#endif

GtkWidget *
create_menu_page(BackdropDialog *bd)
{
    GtkWidget *page, *vbox, *frame, *frame_bin, *chk, *wl_vbox, *dm_vbox;
#ifdef USE_DESKTOP_MENU
    GtkWidget *hbox, *btn;
#endif
#ifdef ENABLE_DESKTOP_ICONS
    GtkWidget *combo;
#endif
    XfceKiosk *kiosk;
    
    kiosk = xfce_kiosk_new("xfdesktop");
    
    page = gtk_vbox_new(FALSE, 6);
    
    add_spacer(GTK_BOX(page));
    
    frame = xfce_create_framebox(_("Window List Menu"), &frame_bin);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(page), frame, FALSE, FALSE, 0);
    
    wl_vbox = vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(frame_bin), vbox);
    
    chk = gtk_check_button_new_with_mnemonic(_("Show window _list"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk), show_windowlist);
    g_object_set_data(G_OBJECT(chk), "xfce-chknum", GUINT_TO_POINTER(OPT_SHOWWL));
    gtk_widget_show(chk);
    gtk_box_pack_start(GTK_BOX(vbox), chk, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(chk), "toggled", G_CALLBACK(set_chk_option), bd);
    
    frame = xfce_create_framebox(NULL, &bd->frame_wl1);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
    
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(bd->frame_wl1), vbox);
    
    chk = gtk_check_button_new_with_mnemonic(_("Show _window icons"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk), show_windowlist_icons);
    g_object_set_data(G_OBJECT(chk), "xfce-chknum", GUINT_TO_POINTER(OPT_SHOWWLI));
    gtk_widget_show(chk);
    gtk_box_pack_start(GTK_BOX(vbox), chk, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(chk), "toggled", G_CALLBACK(set_chk_option), bd);
    
    gtk_widget_set_sensitive(bd->frame_wl1, show_windowlist);
    
    if(!xfce_kiosk_query(kiosk, "CustomizeWindowlist"))
        gtk_widget_set_sensitive(wl_vbox, FALSE);
    
#ifdef USE_DESKTOP_MENU
    frame = xfce_create_framebox(_("Desktop Menu"), &frame_bin);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(page), frame, FALSE, FALSE, 0);
    
    dm_vbox = vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(frame_bin), vbox);
    
    chk = gtk_check_button_new_with_mnemonic(_("Show desktop _menu"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk), show_desktopmenu);
    g_object_set_data(G_OBJECT(chk), "xfce-chknum", GUINT_TO_POINTER(OPT_SHOWDM));
    gtk_widget_show(chk);
    gtk_box_pack_start(GTK_BOX(vbox), chk, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(chk), "toggled", G_CALLBACK(set_chk_option), bd);
    
    frame = xfce_create_framebox(NULL, &bd->frame_dm1);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, 0);
    
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(bd->frame_dm1), vbox);
    
    chk = gtk_check_button_new_with_mnemonic(_("_Show application icons"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk), show_desktopmenu_icons);
    g_object_set_data(G_OBJECT(chk), "xfce-chknum", GUINT_TO_POINTER(OPT_SHOWDMI));
    gtk_widget_show(chk);
    gtk_box_pack_start(GTK_BOX(vbox), chk, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(chk), "toggled", G_CALLBACK(set_chk_option), bd);
    
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    
    btn = gtk_button_new_with_mnemonic(_("_Edit desktop menu"));
    gtk_widget_show(btn);
    gtk_box_pack_start(GTK_BOX(hbox), btn, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(btn), "clicked", G_CALLBACK(_edit_menu_cb), NULL);
    
    gtk_widget_set_sensitive(bd->frame_dm1, show_desktopmenu);
    
    if(!xfce_kiosk_query(kiosk, "CustomizeDesktopMenu"))
        gtk_widget_set_sensitive(dm_vbox, FALSE);
#endif
    
#ifdef ENABLE_DESKTOP_ICONS
    frame = xfce_create_framebox(_("Desktop Icons"), &frame_bin);
    gtk_widget_show(frame);
    gtk_box_pack_start(GTK_BOX(page), frame, FALSE, FALSE, 0);
    
    combo = gtk_combo_box_new_text();
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), _("None"));
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), _("Minimized application icons"));
# ifdef HAVE_THUNAR_VFS
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), _("File/launcher icons"));
# endif
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), desktop_icon_style);
    gtk_widget_show(combo);
    gtk_container_add(GTK_CONTAINER(frame_bin), combo);
    g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(set_di_option), bd);
    
    if(!xfce_kiosk_query(kiosk, "CustomizeDesktopIcons"))
        gtk_widget_set_sensitive(combo, FALSE);
#endif
    
    xfce_kiosk_free(kiosk);
    
    return page;
}
