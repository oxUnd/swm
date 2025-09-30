# SWM 使用指南

## 概述

SWM (Simple Window Manager) 是一个轻量级的 X11 平铺窗口管理器，使用 C 语言实现。

## 核心特性

### 1. 平铺布局

SWM 提供多种平铺算法，可以通过快捷键快速切换：

#### Tile 布局 (默认)
- 主窗口区域在左侧，占据屏幕的一定比例
- 其他窗口平铺在右侧
- 按 `Mod+t` 切换到此布局

```
┌────────┬────┐
│        │  2 │
│   1    ├────┤
│        │  3 │
└────────┴────┘
```

#### Monocle 布局
- 每次只显示一个窗口，占据全屏
- 其他窗口隐藏在后台
- 按 `Mod+m` 切换到此布局

```
┌───────────┐
│           │
│     1     │
│           │
└───────────┘
```

#### Grid 布局
- 将所有窗口排列成网格
- 自动计算最佳行列数
- 按 `Mod+g` 切换到此布局

```
┌─────┬─────┐
│  1  │  2  │
├─────┼─────┤
│  3  │  4  │
└─────┴─────┘
```

#### Floating 布局
- 窗口可以自由移动和调整大小
- 不进行自动平铺
- 按 `Mod+s` 切换到此布局

### 2. 快捷键系统

默认使用 `Mod` (Super/Windows 键) 作为主修饰键。

#### 应用启动
- `Mod + Enter` : 启动终端 (xterm)
- `Mod + d` : 启动应用启动器 (dmenu)
- `Mod + w` : 启动浏览器 (firefox)
- `Mod + e` : 启动文件管理器 (thunar)

#### 窗口管理
- `Mod + q` : 关闭当前窗口
- `Mod + j` : 聚焦下一个窗口
- `Mod + k` : 聚焦上一个窗口
- `Mod + f` : 切换全屏模式
- `Mod + Space` : 切换浮动模式

#### 布局控制
- `Mod + h` : 减小主窗口区域
- `Mod + l` : 增大主窗口区域
- `Mod + i` : 增加主窗口数量
- `Mod + o` : 减少主窗口数量

#### 布局切换
- `Mod + t` : Tile 布局
- `Mod + m` : Monocle 布局
- `Mod + g` : Grid 布局
- `Mod + s` : Floating 布局

#### 系统
- `Mod + Shift + q` : 退出窗口管理器
- `Mod + Shift + r` : 重启窗口管理器

### 3. 系统托盘

SWM 支持系统托盘功能，可以显示系统托盘图标（如网络、音量等）。

托盘默认位于屏幕右下角。

### 4. 可定制性

#### 修改配置

编辑 `config.h` 文件来自定义：

1. **外观**：
```c
#define BORDER_WIDTH        3
#define BORDER_NORMAL       "#333333"
#define BORDER_FOCUS        "#0088cc"
```

2. **布局参数**：
```c
#define MASTER_FACTOR       0.55
#define NUM_MASTER          1
```

3. **修饰键**：
```c
#define MODKEY Mod4Mask  // 或 Mod1Mask 使用 Alt
```

4. **快捷键**：
```c
static KeyBinding keys[] = {
    { MODKEY, XK_Return, spawn, "xterm" },
    // 添加更多快捷键...
};
```

5. **布局顺序**：
```c
static TilingLayout layouts[] = {
    { "tile",     tile_layout },
    { "monocle",  monocle_layout },
    // 第一个为默认布局
};
```

修改后重新编译：
```bash
make clean && make
```

#### 添加自定义布局

要添加自定义布局算法：

1. 在 `layout.c` 中实现布局函数：
```c
void my_custom_layout(Monitor *m) {
    // 你的布局算法
}
```

2. 在 `swm.h` 中声明：
```c
void my_custom_layout(Monitor *m);
```

3. 在 `config.h` 中注册：
```c
static TilingLayout layouts[] = {
    { "custom", my_custom_layout },
    // ...
};
```

4. 添加快捷键：
```c
{ MODKEY, XK_c, set_layout, "custom" },
```

## 工作流程示例

### 典型使用场景

1. **启动 SWM**：
```bash
startx  # 或通过显示管理器登录
```

2. **打开应用**：
- `Mod + Enter` 打开终端
- `Mod + d` 打开应用菜单
- `Mod + w` 打开浏览器

3. **管理窗口**：
- 窗口自动平铺
- 使用 `Mod + j/k` 在窗口间切换
- 使用 `Mod + h/l` 调整窗口大小比例

4. **切换布局**：
- 需要专注单个窗口时：`Mod + m` (Monocle)
- 需要查看多个窗口时：`Mod + t` (Tile)
- 需要均匀分布窗口时：`Mod + g` (Grid)

## 故障排除

### 窗口管理器无法启动

检查是否有其他窗口管理器正在运行：
```bash
ps aux | grep -E "metacity|openbox|xfwm|kwin"
```

### 快捷键不工作

确保没有其他应用占用相同的快捷键组合。

### 系统托盘不显示

检查是否有其他系统托盘正在运行：
```bash
ps aux | grep tray
```

### 应用无法启动

确保快捷键中配置的应用已安装：
```bash
which xterm dmenu firefox
```

## 高级技巧

### 1. 自动启动应用

在 `~/.xinitrc` 或启动脚本中：
```bash
#!/bin/sh

# 后台启动应用
xterm &
firefox &
pcmanfm &

# 启动窗口管理器（必须在最后）
exec swm
```

### 2. 多显示器支持

当前版本将所有显示器作为单一工作区。未来版本将支持独立的显示器管理。

### 3. 性能优化

SWM 非常轻量，但如果需要进一步优化：
- 减少边框宽度
- 使用简单的布局（tile 而不是 grid）
- 关闭不需要的系统托盘

## 贡献

欢迎贡献代码和建议！SWM 的设计遵循以下原则：
- 保持简单
- 高度可定制
- 代码清晰易读
- 良好的抽象层次
