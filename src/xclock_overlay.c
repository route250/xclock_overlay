// Copyright (c) 2024 Maeda
// This software is released under the MIT License.

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrender.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Display *display;
Window window;
GC gc;
XFontStruct *font;
Visual *visual;
Atom wm_delete_window;
Atom _NET_WM_STATE;
Atom _NET_WM_STATE_ABOVE;
Atom _NET_WM_STATE_STAYS_ON_TOP;
int window_width = 140;    // デフォルト値
int window_height = 40;    // デフォルト値
int window_x = -1;        // -1は自動配置を意味する
int window_y = -1;
unsigned long bg_color = 0x000000;   // デフォルト背景色 (黒)
unsigned long fg_color = 0xFFFFFF;   // デフォルトフォント色 (白)
int alpha = 0x8000;                  // デフォルト透明度 (50%)

// 16進数の文字列を数値に変換
unsigned long parse_color(const char *hex) {
    // #を無視
    if (hex[0] == '#') hex++;
    
    unsigned long color;
    sscanf(hex, "%lx", &color);
    return color;
}

void parse_geometry(const char *geometry) {
    int mask;
    XSizeHints hints;
    
    mask = XParseGeometry(geometry, &hints.x, &hints.y,
                         (unsigned int*)&hints.width, (unsigned int*)&hints.height);
    
    if (mask & WidthValue)
        window_width = hints.width;
    if (mask & HeightValue)
        window_height = hints.height;
    if (mask & XValue)
        window_x = hints.x;
    if (mask & YValue)
        window_y = hints.y;
    
    // 負の値は画面右端/下端からの位置指定
    if (mask & XNegative)
        window_x = -1;  // 自動計算用のフラグ
    if (mask & YNegative)
        window_y = -1;
}

void create_window(void) {
    XSetWindowAttributes attributes;
    Colormap colormap;
    int screen_num;

    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Cannot open display\n");
        exit(1);
    }

    screen_num = DefaultScreen(display);
    // ARGBA形式のビジュアルを探す
    XVisualInfo vinfo_template;
    int num_visuals;
    vinfo_template.screen = screen_num;
    XVisualInfo *visual_list = XGetVisualInfo(display, VisualScreenMask, &vinfo_template, &num_visuals);
    
    visual = NULL;
    for (int i = 0; i < num_visuals; i++) {
        XRenderPictFormat *format = XRenderFindVisualFormat(display, visual_list[i].visual);
        if (format && format->type == PictTypeDirect && format->direct.alpha) {
            visual = visual_list[i].visual;
            break;
        }
    }
    
    if (!visual) {
        fprintf(stderr, "No RGBA visual found\n");
        exit(1);
    }
    
    // カラーマップを作成
    colormap = XCreateColormap(display, DefaultRootWindow(display), visual, AllocNone);
    XFree(visual_list);

    // ウィンドウ属性の設定
    attributes.colormap = colormap;
    attributes.border_pixel = 0;
    attributes.background_pixel = 0;
    attributes.override_redirect = True;  // ウィンドウマネージャを無視
    attributes.event_mask = ExposureMask;

    // ウィンドウの位置を設定
    int x = window_x;
    int y = window_y;
    if (x == -1)
        x = DisplayWidth(display, screen_num) - window_width - 20;
    if (y == -1)
        y = 20;
    window = XCreateWindow(display, DefaultRootWindow(display),
                          x, y, window_width, window_height, 0,
                          32, InputOutput, visual,  // 32ビット深度（RGBA用）
                          CWColormap | CWBorderPixel | CWBackPixel | CWOverrideRedirect | CWEventMask,
                          &attributes);

    // 背景を透明に設定
    attributes.background_pixel = 0x00000000;  // ARGB: 完全透明
    attributes.border_pixel = 0x00000000;

    // マウスイベントを透過させる
    XRectangle rect = {0, 0, 0, 0};
    XShapeCombineRectangles(display, window, ShapeInput, 0, 0, &rect, 1, ShapeSet, 0);

    // フォントの設定
    font = XLoadQueryFont(display, "fixed");
    if (!font) {
        fprintf(stderr, "Cannot load font\n");
        exit(1);
    }

    // グラフィックスコンテキストの設定
    XGCValues values;
    values.font = font->fid;
    values.foreground = fg_color;  // 指定された文字色を使用
    gc = XCreateGC(display, window, GCFont | GCForeground, &values);

    // WM_DELETE_WINDOWプロトコルの設定
    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    // 常に最前面に表示するためのプロパティを設定
    _NET_WM_STATE = XInternAtom(display, "_NET_WM_STATE", False);
    _NET_WM_STATE_ABOVE = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
    _NET_WM_STATE_STAYS_ON_TOP = XInternAtom(display, "_NET_WM_STATE_STAYS_ON_TOP", False);

    XChangeProperty(display, window, _NET_WM_STATE, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&_NET_WM_STATE_ABOVE, 1);
    XChangeProperty(display, window, _NET_WM_STATE, XA_ATOM, 32,
                    PropModeAppend, (unsigned char *)&_NET_WM_STATE_STAYS_ON_TOP, 1);

    // ウィンドウを表示
    XMapWindow(display, window);
}

void draw_time(void) {
    time_t current_time;
    struct tm *time_info;
    char time_str[9];
    int direction, ascent, descent;
    XCharStruct overall;

    // 現在時刻を取得して文字列に変換
    time(&current_time);
    time_info = localtime(&current_time);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", time_info);

    // テキストの寸法を取得
    XTextExtents(font, time_str, strlen(time_str),
                 &direction, &ascent, &descent, &overall);

    // 背景を半透明で描画
    XRenderColor render_color;
    render_color.red = ((bg_color >> 16) & 0xFF) * 257;    // 8bit to 16bit
    render_color.green = ((bg_color >> 8) & 0xFF) * 257;
    render_color.blue = (bg_color & 0xFF) * 257;
    render_color.alpha = alpha;

    Picture picture = XRenderCreatePicture(display, window,
                                         XRenderFindVisualFormat(display, visual),
                                         0, NULL);
    XRenderFillRectangle(display, PictOpSrc, picture, &render_color,
                         0, 0, window_width, window_height);
    XRenderFreePicture(display, picture);

    // テキストを中央に描画
    XFlush(display);  // 背景の描画を確実に反映
    int x = (window_width - overall.width) / 2;
    int y = (window_height + (ascent - descent)) / 2;
    XDrawString(display, window, gc, x, y, time_str, strlen(time_str));

    XFlush(display);
}

void print_help(const char *program_name) {
    printf("Usage: %s [options]\n\n", program_name);
    printf("Options:\n");
    printf("  -geometry WxH+X+Y    Set window size and position\n");
    printf("                       e.g., -geometry 140x40+100+50\n");
    printf("                            -geometry 140x40-0+0 (top-right)\n");
    printf("  -bg #RRGGBB         Set background color (default: #000000)\n");
    printf("  -fg #RRGGBB         Set font color (default: #FFFFFF)\n");
    printf("  -alpha N            Set transparency level (0-100, default: 50)\n");
    printf("  -h, --help          Show this help message\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    // オプションの解析
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
        } else if (strncmp(argv[i], "-geometry", 9) == 0 && i + 1 < argc) {
            parse_geometry(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-bg") == 0 && i + 1 < argc) {
            bg_color = parse_color(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-fg") == 0 && i + 1 < argc) {
            fg_color = parse_color(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-alpha") == 0 && i + 1 < argc) {
            int percent = atoi(argv[i + 1]);
            if (percent < 0) percent = 0;
            if (percent > 100) percent = 100;
            alpha = (percent * 65535) / 100;  // 0-100を0-65535に変換
            i++;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_help(argv[0]);
        }
    }
    create_window();

    while (1) {
        XEvent event;
        while (XPending(display)) {
            XNextEvent(display, &event);
            switch (event.type) {
                case Expose:
                    draw_time();
                    break;
                case ClientMessage:
                    if ((unsigned long)event.xclient.data.l[0] == wm_delete_window) {
                        XCloseDisplay(display);
                        exit(0);
                    }
                    break;
            }
        }
        draw_time();
        XRaiseWindow(display, window);  // 常に最前面に表示
        sleep(1);
    }

    return 0;
}
