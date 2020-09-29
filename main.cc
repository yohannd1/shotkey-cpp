#include <iostream>
#include <memory>

#include "sk.hh"

using sk::uint;

enum class Mode: uint {
    Normal,
    Alt,
};

std::string mode_to_string(Mode mode) {
    std::string str;
    str.append("Mode::");

    switch (mode) {
    case Mode::Normal:
        str.append("Normal");
        break;
    case Mode::Alt:
        str.append("Alt");
        break;
    default:
        str.append("<Unknown>");
        break;
    }

    return str;
}

int main(const int argc, const char *argv[]) {
    const auto Mod = Mod4Mask;

    sk::Config<Mode> config = (sk::Config<Mode>) {
        .default_mode = {
            .mode = Mode::Normal,
            .is_persistent = true,
        },
        .shell = "/bin/sh",
        .mode_to_string = mode_to_string,
    };

    sk::Manager<Mode> manager(config);
    manager.add_binding(Mode::Normal, Mod, XK_n,
                        sk::change_mode_command<Mode>(Mode::Alt, false));
    manager.add_binding(Mode::Alt, Mod, XK_n,
                        sk::shell_command<Mode>("notify-send helloworld", mode_to_string));

    manager.run();
}
