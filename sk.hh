#ifndef _SK_HH
#define _SK_HH

#include <functional>
#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <string>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>

#include <unistd.h>

static int handle_x_error(Display *disp, XErrorEvent *ev);
static void spawn_async(const char **command);

namespace sk {
    typedef unsigned int uint;

    template <typename M>
    using ModeToStringFn = std::function<std::string (M mode)>;

    template <typename M>
    struct KeyModeData {
        M mode;
        bool is_persistent;
    };

    template <typename M>
    struct Config {
        KeyModeData<M> default_mode;
        const char *shell;
        ModeToStringFn<M> mode_to_string;
    };

    template <typename M>
    using Command = std::function<void (std::vector<sk::KeyModeData<M>> &mode_stack,
                                        const sk::Config<M> &config)>;

    template <typename M>
    struct Keybinding {
        M mode;
        uint modmask;
        KeySym key;
        Command<M> cmd;
    };

    template <typename M>
    class Manager {
        Display *display;
        Window root;
        Config<M> config;
        std::vector<KeyModeData<M>> mode_stack;
        std::vector<Keybinding<M>> bindings;
        std::function<std::string (M mode)> mode_to_string;

        void bind_key(const Keybinding<M> &kb);
        void unbind_key(const Keybinding<M> &kb);
    public:
        void add_binding(M mode, uint modmask, KeySym key, Command<M> cmd);
        void run();
        explicit Manager(Config<M> config);
        ~Manager();
    };

    template <typename M>
    static Command<M> shell_command(std::string cmd);

    template <typename M>
    static Command<M> change_mode_command(M mode, bool persist);
}

template <typename M>
sk::Manager<M>::Manager(sk::Config<M> config)
    : config(std::move(config))
{
    XSetErrorHandler(handle_x_error);

    this->display = XOpenDisplay(nullptr);
    if (this->display == nullptr) {
        std::cerr << "fatal: could not open display" << '\n';
        std::exit(0);
    }

    this->root = DefaultRootWindow(this->display);

    this->mode_stack.push_back(this->config.default_mode);
}

template <typename M>
sk::Manager<M>::~Manager() {
    XCloseDisplay(this->display);
}

template <typename M>
void sk::Manager<M>::run() {
    for (auto &key: this->bindings) {
        this->bind_key(key);
    }

    XSelectInput(this->display, this->root, KeyPressMask);

    XSync(this->display, false);

    for (;;) {
        XEvent ev;
        XMaskEvent(this->display, KeyPressMask, &ev);
        
        switch (ev.type) {
        case KeyPress:
            const auto data = *(this->mode_stack.rbegin());
            const auto keysym = XkbKeycodeToKeysym(this->display, ev.xkey.keycode,
                                                   0, 0);

            for (const auto &binding: this->bindings) {
                if (binding.mode == data.mode) {
                    if (!data.is_persistent) {
                        this->mode_stack.pop_back();
                        // TODO: mode change hook
                    }

                    binding.cmd(this->mode_stack, this->config);
                }
            }
        }
    }
}

template <typename M>
void sk::Manager<M>::bind_key(const Keybinding<M> &kb) {
    XGrabKey(this->display, XKeysymToKeycode(this->display, kb.key),
             kb.modmask, this->root, false,
             GrabModeAsync, GrabModeAsync);
}

template <typename M>
void sk::Manager<M>::unbind_key(const Keybinding<M> &kb) {
    XUngrabKey(this->display, XKeysymToKeycode(this->display, kb.key),
               kb.modmask, this->root);
}

template <typename M>
sk::Command<M> sk::shell_command(std::string cmd, ModeToStringFn<M> to_string) {
    return [cmd, to_string](const auto &mstack, const auto &config) {
        const auto mode = (*mstack.rbegin()).mode;

        auto label = to_string(mode);
        setenv("SK_MODE_LABEL", label.c_str(), 1);

        const char *arr[] = { config.shell, "-c", cmd.c_str(), NULL };
        spawn_async(arr);
    };
}

template <typename M>
sk::Command<M> sk::change_mode_command(M mode, bool is_persistent) {
    return [mode, is_persistent](auto &mstack, const auto &_config) {
        mstack.push_back((KeyModeData<M>) { mode, is_persistent });
    };
}

template <typename M>
void sk::Manager<M>::add_binding(M mode, uint modmask, KeySym key, Command<M> cmd) {
    const sk::Keybinding<M> kb = {
        .mode = mode,
        .modmask = modmask,
        .key = key,
        .cmd = cmd,
    };

    this->bindings.push_back(kb);
}

static int handle_x_error(Display *disp, XErrorEvent *ev) {
    switch (ev->error_code) {
    case BadAccess:
        std::cerr << "warning: couldn't grab keybinding - already grabbed." << '\n';
        break;
    default:
        std::cerr << "warning: unhandled error." << '\n';
        break;
    }

    return 1;
}

static void spawn_async(const char **command) {
    auto code = fork();
    if (code == 0) {
        setsid();
        /* std::cerr << "shotkey: executing " << '"' << command[2] << '"' << '\n'; */
        execvp((char *) command[0], (char **) command);
    } else if (code < 0) {
        perror("forking failed");
    }
}

#endif /* _SK_HH */
