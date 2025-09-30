# SWM 架构设计文档

## 概述

SWM (Simple Window Manager) 是一个遵循 Unix 哲学的轻量级窗口管理器：做好一件事，做到极致。本文档详细说明了 SWM 的架构设计和实现细节。

## 设计原则

1. **简单性**：代码清晰，易于理解和维护
2. **可定制性**：通过配置文件轻松定制行为
3. **可扩展性**：模块化设计，易于添加新功能
4. **高性能**：最小化资源占用，快速响应

## 架构层次

```
┌─────────────────────────────────────┐
│         Configuration Layer         │  config.h
│    (User customization interface)   │
└─────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────┐
│          Application Layer          │
├──────────┬──────────┬───────────────┤
│ Keybind  │  Layout  │     Tray      │  keybind.c, layout.c, tray.c
│ (Input)  │ (Tiling) │  (System UI)  │
└──────────┴──────────┴───────────────┘
                  ↓
┌─────────────────────────────────────┐
│          Management Layer           │
│       (Client lifecycle)            │  client.c
└─────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────┐
│            Core Layer               │
│    (X11 connection & events)        │  swm.c
└─────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────┐
│              X11 API                │
└─────────────────────────────────────┘
```

## 核心模块

### 1. Core Layer (swm.c)

**职责**：
- X11 连接管理
- 事件循环和分发
- 程序初始化和清理
- 原子（Atoms）管理

**关键函数**：
- `setup()`: 初始化 X11 连接，注册事件处理器
- `run()`: 主事件循环
- `scan()`: 扫描现有窗口
- `cleanup()`: 清理资源

**事件处理**：
```c
static void (*event_handlers[LASTEvent])(XEvent *) = {
    [ConfigureRequest] = on_configure_request,
    [MapRequest] = on_map_request,
    [UnmapNotify] = on_unmap_notify,
    // ...
};
```

使用函数指针数组实现事件分发，高效且易于扩展。

### 2. Client Management (client.c)

**职责**：
- 客户端（窗口）生命周期管理
- 窗口属性设置
- 焦点管理
- 窗口几何操作

**关键数据结构**：
```c
struct Client {
    Window win;              // X11 窗口 ID
    int x, y, w, h;         // 当前位置和大小
    int old_x, old_y, old_w, old_h;  // 保存的位置（用于切换模式）
    bool is_floating;       // 是否浮动
    bool is_fullscreen;     // 是否全屏
    Client *next, *prev;    // 双向链表
};
```

**关键函数**：
- `create_client()`: 创建并初始化客户端
- `attach_client()`: 添加到管理列表
- `detach_client()`: 从列表移除
- `focus_client()`: 设置焦点
- `resize_client()`: 调整窗口大小

### 3. Layout Engine (layout.c)

**职责**：
- 实现各种平铺算法
- 布局抽象和切换
- 窗口排列计算

**布局抽象**：
```c
typedef void (*LayoutFunc)(Monitor *m);

struct TilingLayout {
    const char *name;
    LayoutFunc apply;
};
```

使用函数指针实现策略模式，允许运行时切换布局算法。

**内置布局**：
1. **Tile**：主-栈布局
   - 主窗口占据左侧
   - 其他窗口平铺在右侧
   - 可调整主窗口比例

2. **Monocle**：单窗口全屏
   - 一次只显示一个窗口
   - 其他窗口隐藏

3. **Grid**：网格布局
   - 自动计算最佳行列数
   - 均匀分布所有窗口

4. **Floating**：浮动模式
   - 不强制平铺
   - 保持窗口原始位置

**添加自定义布局**：
```c
void my_custom_layout(Monitor *m) {
    // 1. 获取客户端列表
    // 2. 计算每个窗口的位置
    // 3. 调用 resize_client() 应用布局
}
```

### 4. Key Binding (keybind.c)

**职责**：
- 快捷键注册和处理
- 命令执行
- 窗口操作（关闭、浮动、全屏等）

**快捷键抽象**：
```c
struct KeyBinding {
    unsigned int mod;        // 修饰键（Mod、Shift 等）
    KeySym keysym;          // 键符号
    void (*func)(const char *arg);  // 回调函数
    const char *arg;        // 参数
};
```

**关键功能**：
- `grab_keys()`: 注册所有快捷键
- `spawn()`: 启动外部程序
- `kill_client()`: 关闭窗口
- `toggle_floating()`: 切换浮动模式
- `set_master_factor()`: 调整主窗口比例

**处理 NumLock 和 CapsLock**：
```c
XGrabKey(dpy, code, mod | Mod2Mask, root, ...);  // NumLock
XGrabKey(dpy, code, mod | LockMask, root, ...);  // CapsLock
```

### 5. System Tray (tray.c)

**职责**：
- 实现 FreeDesktop 系统托盘协议
- 管理托盘图标
- 处理 XEMBED 消息

**托盘数据结构**：
```c
struct SystemTray {
    Window win;             // 托盘窗口
    int x, y, w, h;        // 托盘位置
    TrayClient *clients;   // 托盘图标列表
};

struct TrayClient {
    Window win;            // 图标窗口
    int x, y, w, h;       // 图标位置
    TrayClient *next;
};
```

**协议实现**：
1. 声明选择所有权（_NET_SYSTEM_TRAY_S%d）
2. 发送 MANAGER 消息
3. 监听 SYSTEM_TRAY_REQUEST_DOCK 消息
4. 使用 XEMBED 协议嵌入图标

### 6. Configuration (config.h)

**职责**：
- 定义所有用户可配置的参数
- 编译时配置

**配置项**：
- 外观：边框宽度、颜色
- 布局：主窗口比例、数量
- 快捷键：所有键盘绑定
- 托盘：位置、大小

## 数据流

### 窗口创建流程

```
新窗口映射
    ↓
MapRequest 事件
    ↓
on_map_request()
    ↓
create_client() ────→ 初始化 Client 结构
    ↓                  设置边框和事件掩码
attach_client() ───→ 添加到客户端列表
    ↓
focus_client() ────→ 设置焦点
    ↓
apply_layout() ────→ 重新排列所有窗口
    ↓
完成
```

### 布局切换流程

```
用户按下布局切换键
    ↓
KeyPress 事件
    ↓
on_key_press()
    ↓
查找对应的 KeyBinding
    ↓
调用 set_layout()
    ↓
更新 mon->layout
    ↓
apply_layout()
    ↓
layout->apply(mon) ──→ 执行布局算法
    ↓                   计算每个窗口位置
循环所有 clients    ←─
    ↓
resize_client()
    ↓
完成
```

## 内存管理

### 资源分配
- 客户端：使用 `calloc()` 分配，确保初始化为零
- 列表管理：双向链表，便于插入和删除
- 托盘：独立的链表管理

### 清理策略
- 窗口关闭：自动清理关联的 Client 结构
- 程序退出：`cleanup()` 遍历所有资源
- 错误处理：及时释放已分配的资源

## 扩展性设计

### 添加新功能的接口

1. **新布局算法**：
   - 实现 `LayoutFunc` 类型的函数
   - 在 `config.h` 中注册

2. **新快捷键**：
   - 实现回调函数 `void func(const char *arg)`
   - 在 `config.h` 的 `keys[]` 中添加

3. **新窗口操作**：
   - 在 `client.c` 中添加函数
   - 在 `swm.h` 中声明
   - 通过快捷键绑定使用

4. **新事件处理**：
   - 实现事件处理函数 `void handler(XEvent *e)`
   - 在 `event_handlers[]` 数组中注册

## 性能考虑

### 优化策略
1. **事件处理**：使用函数指针数组，O(1) 查找
2. **客户端查找**：虽然使用链表，但通常窗口数量较少
3. **布局计算**：仅在必要时重新计算（窗口创建/销毁/调整）
4. **X11 调用**：批量操作后调用 `XSync()`

### 内存占用
- 核心结构体约 100-200 字节/窗口
- 最小运行时内存约 1-2 MB
- 无内存泄漏（正确释放所有资源）

## 错误处理

### 策略
1. **初始化阶段**：失败则退出，显示错误信息
2. **运行时**：记录错误但继续运行
3. **X11 错误**：使用默认错误处理器

### 调试支持
- 编译时添加 `-DDEBUG` 启用调试信息
- 使用 `printf()` 输出关键事件
- 支持在 Xephyr 中测试

## 未来改进方向

1. **多显示器支持**：为每个显示器创建独立的 Monitor
2. **工作区/标签**：添加虚拟桌面支持
3. **配置文件**：支持运行时配置（如使用 Lua）
4. **IPC**：支持外部命令控制窗口管理器
5. **EWMH 完整支持**：更好的应用兼容性

## 总结

SWM 的架构设计充分体现了模块化和可扩展性的原则。每个模块都有清晰的职责和接口，使得代码易于理解、维护和扩展。通过函数指针和配置文件，用户可以轻松定制窗口管理器的行为，而无需深入了解 X11 的复杂细节。
