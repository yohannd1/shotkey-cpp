namespace cfg {
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
