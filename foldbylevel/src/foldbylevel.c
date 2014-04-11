#include "geanyplugin.h"
#include <config.h>

GeanyPlugin     *geany_plugin;
GeanyData       *geany_data;
GeanyFunctions  *geany_functions;

PLUGIN_VERSION_CHECK(211)

PLUGIN_SET_TRANSLATABLE_INFO(
	LOCALEDIR,
	GETTEXT_PACKAGE,
	_("Fold by Level"),
	_("Enables code folding by level, instead of all or none."),
	"1.0",
	"Peter O'Malley <ooomalley@gmail.com>")

enum {FOLD_LEVELS = 9};

static GtkWidget *fold_menu_item = NULL, *unfold_menu_item = NULL;
static GtkWidget *fold_items[FOLD_LEVELS], *unfold_items[FOLD_LEVELS];
static GeanyKeyGroup *G_key_group;

void fold_level(GeanyEditor *editor, gboolean want_fold, gint want_level)
{
    gint lines, first, i, level;
    
    if (editor == NULL)
        return;

    lines = sci_get_line_count(editor->sci);
    first = scintilla_send_message(editor->sci, SCI_GETFIRSTVISIBLELINE, 0, 0);
    
    for (i = 0; i < lines; i++)
    {
        level = scintilla_send_message(editor->sci, SCI_GETFOLDLEVEL, i, 0);
        
        if ((level & SC_FOLDLEVELHEADERFLAG) &&
            (level & SC_FOLDLEVELNUMBERMASK) - 1024 == want_level)
        {
            if (scintilla_send_message(editor->sci, SCI_GETFOLDEXPANDED, i, 0) == want_fold)
            {
                scintilla_send_message(editor->sci, SCI_TOGGLEFOLD, i, 0);
            }
        }
    /*editor_scroll_to_line(editor, first, 0.0F);*/
    }
}

static void item_activate_cb(GtkMenuItem *menuitem, gpointer user_data)
{
    gboolean want_fold;
    gint want_level; 
    char *action;
    
    /* this is pretty hokey but we'll pull the fold level out of the 
     * label of the menu item. */
    action = g_object_get_data(G_OBJECT(menuitem), "action");

    if (action[0] == 'F') {
        want_fold = TRUE;
        want_level = g_ascii_digit_value(action[1]) - 1;
    } else {
        want_fold = FALSE;
        want_level = g_ascii_digit_value(action[1]) - 1;
    }

    GeanyDocument *doc = document_get_current();
    g_return_if_fail(doc != NULL);
    fold_level(doc->editor, want_fold, want_level);
}

static gboolean kb_callback(guint key_id)
{
    GtkWidget *item;
    /* key IDs are:
     * 0 = Fold 1
     * 1 = Unfold 1
     * 2 = Fold 2
     * ... */
    if (key_id % 2 == 0)
        item = fold_items[key_id / 2];
    else
        item = unfold_items[key_id / 2];

    item_activate_cb(GTK_MENU_ITEM(item), NULL);
    return TRUE;
}

void plugin_init(GeanyData *data)
{
    GtkWidget *fold_menu, *unfold_menu, *menu_item;
    gint i;
    
    /* keybinding group */
    G_key_group = plugin_set_key_group(geany_plugin, "fold_by_level",
                                FOLD_LEVELS*2, &kb_callback);
    
    /* create fold and unfold submenus */
    fold_menu_item = gtk_menu_item_new_with_mnemonic(_("Fold by level"));
    fold_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fold_menu_item), fold_menu);
    unfold_menu_item = gtk_menu_item_new_with_mnemonic(_("Unfold by level"));
    unfold_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(unfold_menu_item), unfold_menu);
    
    /* create fold and unfold items, as well as key bindings */
    for (i = 1; i <= FOLD_LEVELS; i++)
    {
        char name[12], kf_name[12], action[4];
        
        /* we also set object data to remember what level and action
           the user wanted.  */
        g_snprintf(name, 12, _("Fold _%d"), i);
        g_snprintf(kf_name, 12, "fold_%d", i);
        g_snprintf(action, 4, "F%d", i);
        menu_item = gtk_menu_item_new_with_mnemonic(name);
        gtk_container_add(GTK_CONTAINER(fold_menu), menu_item);
        g_signal_connect(menu_item, "activate", 
                            G_CALLBACK(item_activate_cb), NULL);
        keybindings_set_item(G_key_group, i*2-2, NULL, 0, 0, 
            kf_name, name, menu_item);
        fold_items[i-1] = menu_item;
        g_object_set_data_full(G_OBJECT(menu_item), "action",
                               g_strdup(action),
                               (GDestroyNotify) g_free);
        
        g_snprintf(name, 12, _("Unfold _%d"), i);
        g_snprintf(kf_name, 12, "unfold_%d", i);
        g_snprintf(action, 4, "U%d", i);
        menu_item = gtk_menu_item_new_with_mnemonic(name);
        gtk_container_add(GTK_CONTAINER(unfold_menu), menu_item);
        g_signal_connect(menu_item, "activate",
                            G_CALLBACK(item_activate_cb), NULL);
        keybindings_set_item(G_key_group, i*2-1, NULL, 0, 0, 
            kf_name, name, menu_item);
        unfold_items[i-1] = menu_item;
        g_object_set_data_full(G_OBJECT(menu_item), "action",
                               g_strdup(action),
                               (GDestroyNotify) g_free);

    }
    
    /* add menus to toolbar and show */
    gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu),
        fold_menu_item);
    gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu),
        unfold_menu_item);
        
    gtk_widget_show(fold_menu_item);
    gtk_widget_show_all(fold_menu);
    gtk_widget_show(unfold_menu_item);
    gtk_widget_show_all(unfold_menu);
    
}

void plugin_cleanup(void)
{
    gtk_widget_destroy(fold_menu_item);
    gtk_widget_destroy(unfold_menu_item);
}
