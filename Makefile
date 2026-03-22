CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0 gtksourceview-3.0` -Wall -g
LDFLAGS = `pkg-config --libs gtk+-3.0 gtksourceview-3.0`

TARGET = cat-cpp-ide
SRC = src/main.c

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
DESKTOPDIR = $(PREFIX)/share/applications
ICONDIR = $(PREFIX)/share/icons/hicolor/48x48/apps

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/
	install -d $(DESTDIR)$(DESKTOPDIR)
	install -m 644 cat-cpp-ide.desktop $(DESTDIR)$(DESKTOPDIR)/
	install -d $(DESTDIR)$(ICONDIR)
	install -m 644 cat-cpp-ide.png $(DESTDIR)$(ICONDIR)/

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(DESKTOPDIR)/cat-cpp-ide.desktop
	rm -f $(DESTDIR)$(ICONDIR)/cat-cpp-ide.png

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean
