实现一个linux x11的desktop manager

# SWM - Simple Window Manager

一个轻量级、可定制的 X11 平铺窗口管理器，使用 C 语言实现。

## 特性

✅ **平铺窗口管理**
- 支持多种平铺算法：Tile、Monocle、Grid、Floating
- 算法可定制，易于扩展新布局
- 自动管理窗口布局和大小

✅ **全局快捷键系统**
- 完全可配置的快捷键绑定
- 支持修饰键组合（Mod、Shift、Control 等）
- 易于添加自定义命令和动作

✅ **系统托盘支持**
- 完整的系统托盘实现
- 支持标准的 FreeDesktop 系统托盘协议
- 自动布局和管理托盘图标

✅ **高度可定制**
- 所有配置通过 config.h 管理
- 支持自定义布局算法
- 灵活的外观配置

## 快速开始

### 编译

```bash
# 复制配置文件
cp config.example.h config.h

# 编译
make

# 运行
./swm
```

详细安装说明请查看 [INSTALL.md](INSTALL.md)

### 基本使用

- `Mod + Enter` : 打开终端
- `Mod + d` : 应用启动器
- `Mod + q` : 关闭窗口
- `Mod + j/k` : 切换窗口焦点
- `Mod + t/m/g` : 切换布局
- `Mod + Shift + q` : 退出

完整使用指南请查看 [USAGE.md](USAGE.md)

## 项目结构

```
swm/
├── swm.h           # 主头文件，类型定义和函数声明
├── swm.c           # 主程序，X11 事件循环
├── client.c        # 客户端（窗口）管理
├── layout.c        # 平铺布局算法实现
├── keybind.c       # 快捷键绑定和处理
├── tray.c          # 系统托盘实现
├── config.h        # 配置文件（由 config.example.h 复制）
├── config.example.h # 配置示例
├── Makefile        # 构建脚本
├── README.md       # 本文件
├── INSTALL.md      # 安装指南
└── USAGE.md        # 使用手册
```

## 架构设计

SWM 采用模块化设计，具有清晰的抽象层次：

1. **核心层** (`swm.c`)：X11 连接、事件循环
2. **客户端层** (`client.c`)：窗口生命周期管理
3. **布局层** (`layout.c`)：平铺算法抽象
4. **输入层** (`keybind.c`)：键盘事件处理
5. **托盘层** (`tray.c`)：系统托盘协议实现
6. **配置层** (`config.h`)：用户配置接口

每个模块都有清晰的接口和职责分离，便于维护和扩展。

## 技术实现

### 约束条件
- ✅ 使用 C 语言实现
- ✅ 只支持 X11 环境
- ✅ 代码具有足够的抽象

### 核心技术
- X11 Xlib API
- FreeDesktop 系统托盘协议
- EWMH (Extended Window Manager Hints)
- 函数指针实现的插件式布局系统

## 自定义布局示例

```c
// 在 layout.c 中添加你的布局
void my_layout(Monitor *m) {
    // 自定义布局逻辑
}

// 在 config.h 中注册
static TilingLayout layouts[] = {
    { "my_layout", my_layout },
    // ...
};
```

## 依赖

- X11 开发库 (libX11)
- C 编译器 (gcc/clang)
- make

## 许可证

本项目为示例实现，供学习和参考使用。

## 贡献

欢迎提交 Issue 和 Pull Request！ 
