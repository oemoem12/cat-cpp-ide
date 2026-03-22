# Cat C++ IDE

A simple, lightweight C++ IDE for beginners and students.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-orange.svg)

## Features

- **Syntax Highlighting** - Beautiful Solarized Light theme for C++ code
- **One-click Compile & Run** - No configuration needed, just write and run
- **Line Numbers** - Easy code navigation
- **Output Panel** - View compile errors and program output instantly
- **Auto Indentation** - Smart code formatting
- **Bracket Matching** - Highlight matching brackets

## Installation

### Ubuntu (Snap)

```bash
snap install cat-cpp-ide
```

### Ubuntu/Debian (DEB Package)

Download from [GitHub Releases](https://github.com/yourusername/cat-cpp-ide/releases) and install:

```bash
sudo dpkg -i cat-cpp-ide_1.0.0-1_amd64.deb
sudo apt-get install -f
```

### Build from Source

```bash
# Install dependencies
sudo apt install libgtk-3-dev libgtksourceview-3.0-dev g++

# Clone and build
git clone https://github.com/yourusername/cat-cpp-ide.git
cd cat-cpp-ide
make

# Install
sudo make install
```

## Usage

1. Open Cat C++ IDE from application menu or run `cat-cpp-ide`
2. Write your C++ code
3. Click **Run** button (or use menu) to compile and execute
4. View output in the bottom panel

## Requirements

- GTK3
- GtkSourceView 3
- g++ compiler

## Screenshots

Main editor window with syntax highlighting and output panel.

## License

MIT License - feel free to use, modify, and distribute.

## Contributing

Pull requests are welcome! Feel free to submit issues and feature requests.

---

Made with ❤️ for C++ beginners
