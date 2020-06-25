# shotkey

shotkey is a suckless hot key daemon for X, inspired by the suckless
philosophy and with configurable custom modes.

For the original version, made by @phenax, see
https://github.com/phenax/shotkey.

### Why did you remake this in C++?

I wanted to add some features (specially), but I also felt like tweaking
the API. Idk if I needed to rewrite all of this.

## Installation

To install, clone this repository and execute

```bash
sudo make install
```

to compile the program.

## Configuration

To configure, edit the `config.hpp` file and recompile/reinstall the
program.

<!-- TODO: talk about hooks -->

Read the man page for more details on how to use this.
