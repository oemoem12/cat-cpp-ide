#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define APP_NAME "Cat C++ IDE"
#define APP_VERSION "1.0.0"

typedef struct {
    GtkWidget *window;
    GtkWidget *source_view;
    GtkSourceBuffer *source_buffer;
    GtkWidget *output_view;
    GtkTextBuffer *output_buffer;
    GtkWidget *statusbar;
    gchar *current_file;
    guint status_context_id;
} AppData;

static const gchar *default_code = 
    "#include <iostream>\n"
    "using namespace std;\n"
    "\n"
    "int main() {\n"
    "    cout << \"Hello, Cat C++ IDE!\" << endl;\n"
    "    return 0;\n"
    "}\n";

static void on_save_as(GtkWidget *widget, AppData *app);

static void update_title(AppData *app) {
    if (app->current_file) {
        gchar *basename = g_path_get_basename(app->current_file);
        gchar *title = g_strdup_printf("%s - %s", basename, APP_NAME);
        gtk_window_set_title(GTK_WINDOW(app->window), title);
        g_free(basename);
        g_free(title);
    } else {
        gtk_window_set_title(GTK_WINDOW(app->window), APP_NAME " - 未命名");
    }
}

static void show_status(AppData *app, const gchar *message) {
    gtk_statusbar_pop(GTK_STATUSBAR(app->statusbar), app->status_context_id);
    gtk_statusbar_push(GTK_STATUSBAR(app->statusbar), app->status_context_id, message);
}

static void append_output(AppData *app, const gchar *text) {
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(app->output_buffer, &end);
    gtk_text_buffer_insert(app->output_buffer, &end, text, -1);
}

static void clear_output(AppData *app) {
    gtk_text_buffer_set_text(app->output_buffer, "", -1);
}

static void on_new(GtkWidget *widget, AppData *app) {
    gtk_source_buffer_begin_not_undoable_action(app->source_buffer);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(app->source_buffer), default_code, -1);
    gtk_source_buffer_end_not_undoable_action(app->source_buffer);
    
    g_free(app->current_file);
    app->current_file = NULL;
    update_title(app);
    clear_output(app);
    show_status(app, "新建文件");
}

static void on_open(GtkWidget *widget, AppData *app) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "打开文件",
        GTK_WINDOW(app->window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "取消", GTK_RESPONSE_CANCEL,
        "打开", GTK_RESPONSE_ACCEPT,
        NULL
    );
    
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "C++ 源文件");
    gtk_file_filter_add_pattern(filter, "*.cpp");
    gtk_file_filter_add_pattern(filter, "*.cxx");
    gtk_file_filter_add_pattern(filter, "*.cc");
    gtk_file_filter_add_pattern(filter, "*.C");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    GtkFileFilter *filter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_all, "所有文件");
    gtk_file_filter_add_pattern(filter_all, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter_all);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        gchar *content = NULL;
        gsize length = 0;
        GError *error = NULL;
        
        if (g_file_get_contents(filename, &content, &length, &error)) {
            gtk_source_buffer_begin_not_undoable_action(app->source_buffer);
            gtk_text_buffer_set_text(GTK_TEXT_BUFFER(app->source_buffer), content, length);
            gtk_source_buffer_end_not_undoable_action(app->source_buffer);
            
            g_free(app->current_file);
            app->current_file = g_strdup(filename);
            update_title(app);
            clear_output(app);
            
            gchar *status = g_strdup_printf("已打开: %s", filename);
            show_status(app, status);
            g_free(status);
        } else {
            GtkWidget *err_dialog = gtk_message_dialog_new(
                GTK_WINDOW(app->window),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "无法打开文件: %s",
                error->message
            );
            gtk_dialog_run(GTK_DIALOG(err_dialog));
            gtk_widget_destroy(err_dialog);
            g_error_free(error);
        }
        
        g_free(content);
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

static void on_save(GtkWidget *widget, AppData *app) {
    if (app->current_file) {
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(app->source_buffer), &start, &end);
        gchar *content = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(app->source_buffer), &start, &end, FALSE);
        
        GError *error = NULL;
        if (g_file_set_contents(app->current_file, content, -1, &error)) {
            gchar *status = g_strdup_printf("已保存: %s", app->current_file);
            show_status(app, status);
            g_free(status);
        } else {
            GtkWidget *err_dialog = gtk_message_dialog_new(
                GTK_WINDOW(app->window),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "保存失败: %s",
                error->message
            );
            gtk_dialog_run(GTK_DIALOG(err_dialog));
            gtk_widget_destroy(err_dialog);
            g_error_free(error);
        }
        
        g_free(content);
    } else {
        on_save_as(widget, app);
    }
}

static void on_save_as(GtkWidget *widget, AppData *app) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "保存文件",
        GTK_WINDOW(app->window),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "取消", GTK_RESPONSE_CANCEL,
        "保存", GTK_RESPONSE_ACCEPT,
        NULL
    );
    
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "untitled.cpp");
    
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "C++ 源文件");
    gtk_file_filter_add_pattern(filter, "*.cpp");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(app->source_buffer), &start, &end);
        gchar *content = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(app->source_buffer), &start, &end, FALSE);
        
        GError *error = NULL;
        if (g_file_set_contents(filename, content, -1, &error)) {
            g_free(app->current_file);
            app->current_file = g_strdup(filename);
            update_title(app);
            
            gchar *status = g_strdup_printf("已保存: %s", filename);
            show_status(app, status);
            g_free(status);
        } else {
            GtkWidget *err_dialog = gtk_message_dialog_new(
                GTK_WINDOW(app->window),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "保存失败: %s",
                error->message
            );
            gtk_dialog_run(GTK_DIALOG(err_dialog));
            gtk_widget_destroy(err_dialog);
            g_error_free(error);
        }
        
        g_free(content);
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

static gchar *get_executable_path(const gchar *source_file) {
    gchar *dir = g_path_get_dirname(source_file);
    gchar *basename = g_path_get_basename(source_file);
    
    gchar *name_without_ext = g_strdup(basename);
    gchar *dot = strrchr(name_without_ext, '.');
    if (dot) *dot = '\0';
    
    gchar *exec_path = g_build_filename(dir, name_without_ext, NULL);
    
    g_free(dir);
    g_free(basename);
    g_free(name_without_ext);
    
    return exec_path;
}

static gboolean compile_and_run(AppData *app, gboolean run_after_compile) {
    if (!app->current_file) {
        show_status(app, "请先保存文件");
        on_save_as(NULL, app);
        if (!app->current_file) return FALSE;
    }
    
    on_save(NULL, app);
    
    gchar *exec_path = get_executable_path(app->current_file);
    gchar *compile_cmd = g_strdup_printf("g++ -std=c++17 -Wall -o \"%s\" \"%s\"", 
                                          exec_path, app->current_file);
    
    clear_output(app);
    gchar *cmd_info = g_strdup_printf("命令: %s\n", compile_cmd);
    append_output(app, "=== 编译中 ===\n");
    append_output(app, cmd_info);
    g_free(cmd_info);
    
    gchar *stdout_output = NULL;
    gchar *stderr_output = NULL;
    gint exit_status;
    GError *error = NULL;
    
    if (g_spawn_command_line_sync(compile_cmd, &stdout_output, &stderr_output, &exit_status, &error)) {
        if (stdout_output && strlen(stdout_output) > 0) {
            append_output(app, stdout_output);
        }
        if (stderr_output && strlen(stderr_output) > 0) {
            append_output(app, stderr_output);
        }
        
        if (exit_status == 0) {
            append_output(app, "\n编译成功!\n");
            show_status(app, "编译成功");
            
            if (run_after_compile) {
                append_output(app, "\n=== 运行中 ===\n");
                
                gchar *run_cmd = g_strdup_printf("x-terminal-emulator -e bash -c '\"%s\"; echo; echo \"按回车键继续...\"; read'", exec_path);
                
                if (!g_spawn_command_line_async(run_cmd, &error)) {
                    append_output(app, "运行失败: ");
                    append_output(app, error->message);
                    append_output(app, "\n");
                    g_error_free(error);
                }
                
                g_free(run_cmd);
            }
        } else {
            append_output(app, "\n编译失败!\n");
            show_status(app, "编译失败");
        }
        g_free(stdout_output);
        g_free(stderr_output);
    } else {
        append_output(app, "编译命令执行失败: ");
        append_output(app, error->message);
        append_output(app, "\n");
        g_error_free(error);
    }
    
    g_free(compile_cmd);
    g_free(exec_path);
    
    return TRUE;
}

static void on_compile(GtkWidget *widget, AppData *app) {
    compile_and_run(app, FALSE);
}

static void on_run(GtkWidget *widget, AppData *app) {
    compile_and_run(app, TRUE);
}

static void on_about(GtkWidget *widget, AppData *app) {
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(app->window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s v%s\n\n"
        "一个简单易用的 C++ IDE\n"
        "无需配置，开箱即用\n\n"
        "功能:\n"
        "• 语法高亮\n"
        "• 一键编译运行\n"
        "• 代码自动补全提示",
        APP_NAME, APP_VERSION
    );
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static GtkWidget *create_toolbar(AppData *app) {
    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);
    
    GtkToolItem *item;
    
    item = gtk_tool_button_new(gtk_image_new_from_icon_name("document-new", GTK_ICON_SIZE_LARGE_TOOLBAR), "新建");
    g_signal_connect(item, "clicked", G_CALLBACK(on_new), app);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    
    item = gtk_tool_button_new(gtk_image_new_from_icon_name("document-open", GTK_ICON_SIZE_LARGE_TOOLBAR), "打开");
    g_signal_connect(item, "clicked", G_CALLBACK(on_open), app);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    
    item = gtk_tool_button_new(gtk_image_new_from_icon_name("document-save", GTK_ICON_SIZE_LARGE_TOOLBAR), "保存");
    g_signal_connect(item, "clicked", G_CALLBACK(on_save), app);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new(), -1);
    
    item = gtk_tool_button_new(gtk_image_new_from_icon_name("applications-development", GTK_ICON_SIZE_LARGE_TOOLBAR), "编译");
    g_signal_connect(item, "clicked", G_CALLBACK(on_compile), app);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    
    item = gtk_tool_button_new(gtk_image_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_LARGE_TOOLBAR), "运行");
    g_signal_connect(item, "clicked", G_CALLBACK(on_run), app);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, -1);
    
    return toolbar;
}

static GtkWidget *create_menubar(AppData *app) {
    GtkWidget *menubar = gtk_menu_bar_new();
    
    GtkWidget *file_menu = gtk_menu_new();
    GtkWidget *file_item = gtk_menu_item_new_with_label("文件");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    
    GtkWidget *new_item = gtk_menu_item_new_with_label("新建");
    g_signal_connect(new_item, "activate", G_CALLBACK(on_new), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), new_item);
    
    GtkWidget *open_item = gtk_menu_item_new_with_label("打开");
    g_signal_connect(open_item, "activate", G_CALLBACK(on_open), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_item);
    
    GtkWidget *save_item = gtk_menu_item_new_with_label("保存");
    g_signal_connect(save_item, "activate", G_CALLBACK(on_save), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_item);
    
    GtkWidget *save_as_item = gtk_menu_item_new_with_label("另存为");
    g_signal_connect(save_as_item, "activate", G_CALLBACK(on_save_as), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), save_as_item);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), gtk_separator_menu_item_new());
    
    GtkWidget *quit_item = gtk_menu_item_new_with_label("退出");
    g_signal_connect(quit_item, "activate", G_CALLBACK(gtk_main_quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_item);
    
    GtkWidget *build_menu = gtk_menu_new();
    GtkWidget *build_item = gtk_menu_item_new_with_label("构建");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(build_item), build_menu);
    
    GtkWidget *compile_item = gtk_menu_item_new_with_label("编译");
    g_signal_connect(compile_item, "activate", G_CALLBACK(on_compile), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(build_menu), compile_item);
    
    GtkWidget *run_item = gtk_menu_item_new_with_label("运行");
    g_signal_connect(run_item, "activate", G_CALLBACK(on_run), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(build_menu), run_item);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), build_item);
    
    GtkWidget *help_menu = gtk_menu_new();
    GtkWidget *help_item = gtk_menu_item_new_with_label("帮助");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
    
    GtkWidget *about_item = gtk_menu_item_new_with_label("关于");
    g_signal_connect(about_item, "activate", G_CALLBACK(on_about), app);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_item);
    
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_item);
    
    return menubar;
}

static void setup_source_view(AppData *app) {
    GtkSourceLanguageManager *lm = gtk_source_language_manager_get_default();
    GtkSourceLanguage *lang = gtk_source_language_manager_get_language(lm, "cpp");
    
    app->source_buffer = gtk_source_buffer_new_with_language(lang);
    gtk_source_buffer_set_highlight_syntax(app->source_buffer, TRUE);
    gtk_source_buffer_set_highlight_matching_brackets(app->source_buffer, TRUE);
    
    GtkSourceStyleSchemeManager *sm = gtk_source_style_scheme_manager_get_default();
    GtkSourceStyleScheme *scheme = gtk_source_style_scheme_manager_get_scheme(sm, "solarized-light");
    if (!scheme) scheme = gtk_source_style_scheme_manager_get_scheme(sm, "tango");
    if (!scheme) scheme = gtk_source_style_scheme_manager_get_scheme(sm, "classic");
    gtk_source_buffer_set_style_scheme(app->source_buffer, scheme);
    
    app->source_view = gtk_source_view_new_with_buffer(app->source_buffer);
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(app->source_view), TRUE);
    gtk_source_view_set_show_line_marks(GTK_SOURCE_VIEW(app->source_view), FALSE);
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(app->source_view), TRUE);
    gtk_source_view_set_indent_on_tab(GTK_SOURCE_VIEW(app->source_view), TRUE);
    gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(app->source_view), TRUE);
    gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(app->source_view), 4);
    gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW(app->source_view), TRUE);
    
    PangoFontDescription *font = pango_font_description_from_string("Monospace 12");
    gtk_widget_override_font(app->source_view, font);
    pango_font_description_free(font);
    
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(app->source_buffer), default_code, -1);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    AppData *app = g_new0(AppData, 1);
    
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), APP_NAME " - 未命名");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 900, 700);
    gtk_window_set_position(GTK_WINDOW(app->window), GTK_WIN_POS_CENTER);
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app->window), vbox);
    
    GtkWidget *menubar = create_menubar(app);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
    
    GtkWidget *toolbar = create_toolbar(app);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
    
    setup_source_view(app);
    
    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(vbox), paned, TRUE, TRUE, 0);
    
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), 
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), app->source_view);
    gtk_paned_pack1(GTK_PANED(paned), scroll, TRUE, TRUE);
    
    GtkWidget *output_frame = gtk_frame_new("输出");
    GtkWidget *output_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(output_frame), output_vbox);
    
    GtkWidget *output_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(output_scroll),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(output_vbox), output_scroll, TRUE, TRUE, 0);
    
    app->output_buffer = gtk_text_buffer_new(NULL);
    app->output_view = gtk_text_view_new_with_buffer(app->output_buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->output_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->output_view), GTK_WRAP_WORD_CHAR);
    
    PangoFontDescription *output_font = pango_font_description_from_string("Monospace 11");
    gtk_widget_override_font(app->output_view, output_font);
    pango_font_description_free(output_font);
    
    gtk_container_add(GTK_CONTAINER(output_scroll), app->output_view);
    gtk_paned_pack2(GTK_PANED(paned), output_frame, FALSE, TRUE);
    
    gtk_paned_set_position(GTK_PANED(paned), 500);
    
    app->statusbar = gtk_statusbar_new();
    app->status_context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(app->statusbar), "main");
    gtk_box_pack_start(GTK_BOX(vbox), app->statusbar, FALSE, FALSE, 0);
    
    show_status(app, "就绪");
    
    gtk_widget_show_all(app->window);
    
    gtk_main();
    
    g_free(app->current_file);
    g_free(app);
    
    return 0;
}
