# Installation and first run

Agenda Qt is currently distributed as source code. A prebuilt application bundle is not included, so the first build requires Qt 6, a C++17 compiler, qmake, make, and Git.

## Supported and validated environments

- **macOS:** built and manually tested with Qt 6.10.1 and Apple Clang on Apple Silicon;
- **Linux:** built and tested through GitHub Actions on Ubuntu/Xvfb;
- **course Docker image:** clean build completed with `unipd-oop/qt-env:2025`;
- **Windows:** the code uses portable Qt APIs, but no Windows binary or complete manual validation is currently provided.

## Required components

The `.pro` file requires only these Qt modules:

```text
Qt Core
Qt GUI
Qt Widgets
```

No database server, web service, account, package manager for C++, or external runtime is required. Agenda files are stored locally as JSON.

## macOS

### 1. Install the compiler and build tools

Open Terminal and install the Xcode Command Line Tools:

```bash
xcode-select --install
```

Verify the installation:

```bash
xcode-select -p
clang++ --version
make --version
git --version
```

### 2. Install Qt 6

Install Qt through the Qt Online Installer and select a macOS **Desktop/Clang** kit. The project was tested with Qt 6.10.1, but the commands below work with another Qt 6 version after changing the version directory.

A typical Qt Online Installer path is:

```text
$HOME/Qt/6.10.1/macos
```

List the installed versions and kits when uncertain:

```bash
find "$HOME/Qt" -maxdepth 3 -type f -name qmake 2>/dev/null
```

### 3. Configure the Qt PATH permanently

macOS uses zsh by default. Replace `6.10.1` with the version actually installed:

```bash
cat >> "$HOME/.zshrc" <<'EOF'

# Qt installed through the Qt Online Installer.
export QT_ROOT="$HOME/Qt/6.10.1/macos"
export PATH="$QT_ROOT/bin:$PATH"
EOF

source "$HOME/.zshrc"
```

Verify the resolved executable and Qt version:

```bash
echo "$QT_ROOT"
command -v qmake
qmake -v
```

The path should resemble:

```text
/Users/your-name/Qt/6.10.1/macos/bin/qmake
```

If `qmake` is still not found, run it once using the complete path discovered by `find`, then correct `QT_ROOT` in `~/.zshrc`.

### 4. Clone and build

```bash
cd "$HOME/Desktop"
git clone https://github.com/hipietro/qt_agenda.git
cd qt_agenda

rm -rf build
mkdir build
cd build

qmake ../agenda_qt.pro
make -j"$(sysctl -n hw.ncpu)"
```

### 5. Run

```bash
open agenda_qt.app
```

A successful command opens the application bundle generated inside `build/`.

### Homebrew Qt alternative

On Apple Silicon, Homebrew normally installs Qt below `/opt/homebrew/opt/qt`. After `brew install qt`, configure:

```bash
cat >> "$HOME/.zshrc" <<'EOF'

# Qt installed through Homebrew on Apple Silicon.
export PATH="/opt/homebrew/opt/qt/bin:$PATH"
EOF

source "$HOME/.zshrc"
qmake -v
```

On Intel Macs the prefix is commonly `/usr/local/opt/qt/bin` instead. Use `brew --prefix qt` to obtain the exact path:

```bash
echo "export PATH=\"$(brew --prefix qt)/bin:\$PATH\"" >> "$HOME/.zshrc"
source "$HOME/.zshrc"
```

## Ubuntu and Debian

### 1. Install dependencies

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  git \
  qt6-base-dev \
  qt6-base-dev-tools
```

Verify the tools:

```bash
g++ --version
make --version
git --version
qmake6 -v
```

The distro package installs `qmake6` in the normal system `PATH`, so no persistent path configuration is normally required.

### 2. Clone, build, and run

```bash
git clone https://github.com/hipietro/qt_agenda.git
cd qt_agenda

rm -rf build
mkdir build
cd build

qmake6 ../agenda_qt.pro
make -j"$(nproc)"
./agenda_qt
```

The application requires access to a graphical desktop session. A terminal-only server without X11 or Wayland cannot display the window.

## Portable qmake selection

Some installations expose `qmake`, while Ubuntu packages commonly expose `qmake6`. This command chooses the available Qt 6 executable:

```bash
QMAKE_BIN="$(command -v qmake6 || command -v qmake)"

if [ -z "$QMAKE_BIN" ]; then
  echo "Qt qmake was not found in PATH" >&2
  exit 1
fi

"$QMAKE_BIN" -v
```

Use it for a portable build:

```bash
rm -rf build
mkdir build
cd build

QMAKE_BIN="$(command -v qmake6 || command -v qmake)"
"$QMAKE_BIN" ../agenda_qt.pro
make -j2
```

## Course Docker environment

The supplied `Dockerfile` builds the course image:

```bash
docker build -t unipd-oop/qt-env:2025 .
```

Open the interactive shell exactly as specified by the course:

```bash
docker run -it --rm \
  -v "$(pwd)":/app -w /app \
  -u $(id -u):$(id -g) \
  unipd-oop/qt-env:2025 bash
```

Inside the container, compile in a clean temporary directory:

```bash
rm -rf /tmp/agenda_qt_build
mkdir /tmp/agenda_qt_build
cd /tmp/agenda_qt_build

qmake6 /app/agenda_qt.pro
make -j"$(nproc)"

test -x agenda_qt && echo "DOCKER BUILD PASSED"
```

On an Apple Silicon Mac, Docker may report that the AMD64 image differs from the ARM64 host. The image can still run through emulation; adding `--platform linux/amd64` makes that choice explicit.

The professor's graphical Docker command is designed for a GNU/Linux host with X11. The macOS validation should run the native `.app` built with the macOS Qt kit.

## First use

1. Start Agenda Qt.
2. Create an activity with **Add activity**, or load [`examples/sample_agenda.json`](examples/sample_agenda.json) through **File → Open**.
3. Use **File → Save As** to choose a writable JSON file.
4. The application stores no information outside the JSON file selected by the user.

## Clean rebuild

When generated files appear stale, delete the build directory rather than compiling over it:

```bash
cd /path/to/qt_agenda
rm -rf build
mkdir build
cd build
QMAKE_BIN="$(command -v qmake6 || command -v qmake)"
"$QMAKE_BIN" ../agenda_qt.pro
make -j2
```

## Common problems

### `qmake` or `qmake6`: command not found

Qt is not installed or its `bin` directory is not in `PATH`. Follow the platform-specific installation and verification steps above.

### `Project ERROR: Unknown module(s) in QT: widgets`

The Qt base development package or Desktop kit is missing. Install `qt6-base-dev` on Ubuntu/Debian or add a Qt Desktop/Clang kit through the Qt installer.

### macOS builds with the wrong Qt installation

Inspect all qmake executables:

```bash
which -a qmake qmake6 2>/dev/null
qmake -v
```

Place the intended Qt `bin` directory before other entries in `PATH`.

### The Linux executable cannot connect to a display

Run it from a graphical Linux desktop session. For automated GUI tests, the repository workflow uses Xvfb.

### Existing build files reference another machine

Delete the complete `build/` directory and run qmake again. Generated Makefiles are not portable between Qt installations or operating systems.
