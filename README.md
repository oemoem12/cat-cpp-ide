# Cat C++ IDE

A simple, lightweight C++ IDE for beginners and students.

## Features

- **Syntax Highlighting** - C++ code with Solarized Light theme
- **One-click Compile & Run** - No configuration needed
- **Line Numbers** - Easy code navigation
- **Output Panel** - View compile errors and program output

## Installation

### Ubuntu (Snap)

```bash
snap install cat-cpp-ide
```

### Ubuntu/Debian (DEB Package)

```bash
# Download from GitHub Releases
sudo dpkg -i cat-cpp-ide_1.0.0-1_amd64.deb
sudo apt-get install -f
```

### From Source

```bash
sudo apt install libgtk-3-dev libgtksourceview-3.0-dev g++
make
sudo make install
```

## Usage

1. Open Cat C++ IDE from application menu or run `cat-cpp-ide`
2. Write your C++ code
3. Click **Run** button to compile and execute

## Requirements

- GTK3
- GtkSourceView 3
- g++ compiler

## License

MIT License
