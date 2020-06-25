/* TODO: organize declarations and put them near where they are used instead of at the start of a function */

#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xutil.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

#include "shotkey.hpp"

static auto mode_stack = std::vector<KeyModeData>();
static auto bindings = std::vector<Keybinding>();

#include "config.hpp"

int main(void) {
    XSetErrorHandler(handle_xerror);

    Display *disp = XOpenDisplay(0); /* TODO: change these pointers into references..? */
    Window root = DefaultRootWindow(disp);
    auto running = true;
    mode_stack.push_back(std::tuple<cfg::KeyMode, bool>(cfg::KeyMode::Normal, true));
    cfg::hooks::startup(bindings);

    for (auto& key : bindings) {
        bind_key(key, disp, root);
    }

    XSelectInput(disp, root, KeyPressMask); /* TODO: ??? */

    XSync(disp, false);
    while (running) {
        XEvent ev;
        XMaskEvent(disp, KeyPressMask, &ev); /* TODO: ??? */

        if (ev.type == KeyPress) { /* TODO: ??? */
            handle_keypress(mode_stack, bindings, disp, root, &ev.xkey);
        }
    }

    XCloseDisplay(disp); /* TODO: ??? */
}

ShellCommand::ShellCommand(const char *cmd): cmd((char*) cmd) {}
FunctionCommand::FunctionCommand(std::function<void (void)>): func(func) {}
KMChangeCommand::KMChangeCommand(cfg::KeyMode mode, bool persist): mode(mode), persist(persist) {}

void spawn_async(char **command) {
    auto code = fork();
    if (code == 0) {
        setsid();
        /* std::cerr << "shotkey: executing " << '"' << command[2] << '"' << '\n'; */
        execvp(command[0], command);
    } else if (code < 0) {
        perror("forking failed");
    }
}

int handle_xerror(Display *disp, XErrorEvent *ev) {
    if (ev->error_code == BadAccess) {
        std::cerr << "shotkey: couldn't grab keybinding - already grabbed." << '\n';
    } else {
        std::cerr << "shotkey: something went wrong." << '\n';
    }
    return 1;
}

void die(const char *s) {
    printf("error: %s\n", s);
    exit(1);
}

void handle_keypress(std::vector<KeyModeData>& mode_stack, const std::vector<Keybinding>& bindings, Display *disp, Window win, XKeyEvent *ev) {
    cfg::KeyMode mode;
    bool persist;
    std::tie(mode, persist) = *mode_stack.rbegin();

    auto keysym = XkbKeycodeToKeysym(disp, ev->keycode, 0, 0);
    for (auto& kb : bindings) {
        if (kb.mode == mode) {
            if (!persist) {
                mode_stack.pop_back();
                cfg::hooks::mode_change(std::get<0>(*mode_stack.rbegin()));
            }
            const std::unique_ptr<Command>& test = kb.cmd;
            process_command(*test);
        }
    }
}

void process_command(const Command& cmd) {
    cmd.execute();
}

void ShellCommand::execute(void) const {
    auto mode = *mode_stack.rbegin();
    setenv("SHOTKEY_MODE_ID", std::to_string((uint) std::get<0>(mode)).c_str(), 1);
    setenv("SHOTKEY_MODE_LABEL", cfg::to_string(std::get<0>(mode)).c_str(), 1);
    char *arr[] = { cfg::SHELL, (char*) "-c", cmd, NULL };
    spawn_async(arr);
}

void FunctionCommand::execute(void) const {
    func();
}

void KMChangeCommand::execute(void) const {
    mode_stack.push_back(std::tuple<cfg::KeyMode, bool>(mode, persist));
    cfg::hooks::mode_change(mode);
}

Keybinding::Keybinding(cfg::KeyMode mode, uint modmask, KeySym key, std::unique_ptr<Command> cmd_): mode(mode), modmask(modmask), key(key) {
    cmd = std::move(cmd_);
}

void bind_key(const Keybinding& kb, Display *disp, Window win) {
    XGrabKey(disp, XKeysymToKeycode(disp, kb.key), kb.modmask, win, false, GrabModeAsync, GrabModeAsync);
}

void unbind_key(const Keybinding& kb, Display *disp, Window win) {
    XUngrabKey(disp, XKeysymToKeycode(disp, kb.key), kb.modmask, win);
}
