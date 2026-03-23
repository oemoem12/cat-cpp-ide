# Cat C++ IDE

A lightweight, beginner-friendly C++ IDE designed for students and new programmers.

![License](https://img.shields.io/badge/license-GPL%203.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-orange.svg)
![Build Status](https://github.com/oemoem12/cat-cpp-ide/actions/workflows/build.yml/badge.svg)

## Overview

Cat C++ IDE provides a simple, intuitive interface for learning and practicing C++ programming. It eliminates the complexity of traditional IDEs while offering essential features for effective coding.

## Features

- **Syntax Highlighting** - Professional Solarized Light theme with clear C++ syntax coloring
- **One-click Compile & Run** - No configuration needed, just write code and execute
- **Integrated Output Panel** - Instantly view compile errors and program output
- **Line Numbers** - Easy code navigation and reference
- **Auto Indentation** - Smart formatting for clean, readable code
- **Bracket Matching** - Highlight matching braces and parentheses
- **Simple Interface** - Clean, distraction-free design focused on learning

## Installation

### Build from Source

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt install libgtk-3-dev libgtksourceview-3.0-dev g++

# Clone repository
git clone https://github.com/oemoem12/cat-cpp-ide.git
cd cat-cpp-ide

# Build and install
make
sudo make install
```

## Usage

1. **Launch the IDE** - From application menu or run `cat-cpp-ide` in terminal
2. **Write code** - Type your C++ program in the editor
3. **Compile & Run** - Click the **Run** button or use the menu (File → Run)
4. **View output** - Check results in the bottom output panel

## Requirements

- GTK3 (3.24+)
- GtkSourceView 3
- g++ compiler (GCC 9+ recommended)
- Linux operating system

## Screenshots

![Cat C++ IDE Screenshot](https://github.com/oemoem12/cat-cpp-ide/assets/12345678/your-screenshot-path)

*Main editor window with syntax highlighting and output panel*

## License

This project is licensed under the **GPL 3.0 License** - see the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome!

- **Bug reports** and **feature requests** - Open an issue
- **Code contributions** - Submit a pull request

## Getting Help

- Check the [GitHub Issues](https://github.com/oemoem12/cat-cpp-ide/issues) page for common questions
- Feel free to open a new issue for any problems or suggestions

---

Made with ❤️ for C++ beginners
