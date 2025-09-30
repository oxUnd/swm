# SWM 安装指南

## 依赖

SWM 需要以下依赖：

- X11 开发库 (libX11-dev / libX11-devel)
- C 编译器 (gcc 或 clang)
- make

### Debian/Ubuntu

```bash
sudo apt-get install libx11-dev build-essential
```

### Fedora/RHEL/CentOS

```bash
sudo dnf install libX11-devel gcc make
```

### Arch Linux

```bash
sudo pacman -S libx11 base-devel
```

## 编译

1. 克隆或下载源代码

2. 配置（可选）：
```bash
cp config.example.h config.h
# 编辑 config.h 来自定义你的配置
```

3. 编译：
```bash
make
```

4. 安装（可选）：
```bash
sudo make install
```

或者直接运行：
```bash
./swm
```

## 配置 X11 启动

### 使用 .xinitrc

编辑 `~/.xinitrc` 文件：

```bash
#!/bin/sh

# 启动应用
xterm &
dmenu &

# 启动窗口管理器
exec swm
```

然后使用 `startx` 启动。

### 使用显示管理器 (Display Manager)

创建 `/usr/share/xsessions/swm.desktop`：

```ini
[Desktop Entry]
Name=SWM
Comment=Simple Window Manager
Exec=/usr/local/bin/swm
Type=Application
DesktopNames=SWM
```

然后在你的显示管理器（如 GDM、LightDM）中选择 SWM。

## 测试

在 Xephyr 中测试（无需替换当前窗口管理器）：

```bash
# 安装 Xephyr
sudo apt-get install xserver-xephyr  # Debian/Ubuntu
sudo dnf install xorg-x11-server-Xephyr  # Fedora

# 启动嵌套 X 服务器
Xephyr -br -ac -noreset -screen 1024x768 :1 &

# 在嵌套 X 中运行 SWM
DISPLAY=:1 ./swm
```

## 卸载

```bash
sudo make uninstall
```
