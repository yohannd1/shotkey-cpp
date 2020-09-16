#include <stdlib.h>
#include <X11/Xutil.h>
#include <functional>
#include <tuple>
#include <vector>

typedef unsigned int uint;

/* config.hpp */
namespace cfg {
    enum class KeyMode: uint;

    std::string to_string(KeyMode mode);
    namespace hooks {
        void startup();
        void mode_change(KeyMode mode);
    }
}

typedef std::tuple<cfg::KeyMode, bool> KeyModeData;

struct Command {
    virtual void execute(void) const = 0;
};

struct ShellCommand : Command {
    char *cmd;

    ShellCommand(const char *cmd);
    virtual void execute(void) const;
};

struct FunctionCommand : Command {
    std::function<void (void)> func;

    FunctionCommand(std::function<void (void)>);
    virtual void execute(void) const;
};

struct KMChangeCommand : Command {
    cfg::KeyMode mode;
    bool persist;

    KMChangeCommand(cfg::KeyMode mode, bool persist);
    virtual void execute(void) const;
};

struct Keybinding {
    cfg::KeyMode mode;
    uint modmask;
    KeySym key;
    std::unique_ptr<Command> cmd;
    Keybinding(cfg::KeyMode mode, uint modmask, KeySym key, std::unique_ptr<Command> cmd);
};

/* shotkey.cpp */
void spawn_async(char **command);
void die(const char *str);
int handle_xerror(Display *disp, XErrorEvent *ev);
void handle_keypress(std::vector<KeyModeData>& mode_stack, const std::vector<Keybinding>& bindings, Display *disp, Window win, XKeyEvent *ev);
void process_command(const Command& cmd);
void bind_key(const Keybinding& kb, Display *disp, Window win);
void unbind_key(const Keybinding& kb, Display *disp, Window win);
