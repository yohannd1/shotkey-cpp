#include <memory>
#include <string>

#define Mod Mod4Mask
#define bind(mode, modmask, key, type, constructor) bindings.push_back(Keybinding(mode, modmask, key, std::unique_ptr<Command>(new type constructor)))

namespace cfg {
    char *SHELL = (char*) "/bin/sh";

    enum class KeyMode: uint {
        Normal,
        Alt,
    };

    std::string to_string(KeyMode mode) {
        switch (mode) {
            case KeyMode::Normal:
                return std::string("Normal");
                break;
            case KeyMode::Alt:
                return std::string("Alt");
                break;
            default:
                return std::string("???");
                break;
        }
    }

    namespace hooks {
        void startup(std::vector<Keybinding> &bindings) {
            bind(KeyMode::Normal, Mod, XK_n, KMChangeCommand, (KeyMode::Alt, false));
            bind(KeyMode::Alt, Mod, XK_n, ShellCommand, ("notify-send asd"));
        }

        void mode_change(KeyMode mode) {
            std::cerr << "Changed mode to " << '"' << to_string(mode) << '"' << '\n';
        }
    }
}
