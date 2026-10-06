# Colorful 0.1.0 — 完整发行包（Windows x64）

自包含的 Colorful 语言发行版：**完整源代码 + 已构建的工具链 + runtime 与标准库**，
另附 VSCode 插件。解压即用，无需安装任何依赖。

- 构建环境：Windows 10/11 x64，MSVC 2022
- 语言版本：0.1.0
- 许可证：MIT（见 `source/LICENSE`）

---

## 目录结构

```text
Colorful-0.1.0/
  bin/                  工具链（7 个可执行文件，静态链接，无需额外 DLL）
    colorc.exe            编译器
    colorlsp.exe          语言服务器
    colorfmt.exe          格式化器
    colortest.exe         测试运行器
    colorpkg.exe          包管理器
    colorlib.exe          runtime 打包工具
    colordoc.exe          文档生成
  lib/                  runtime 与链接库
    libcolorful.lib       静态运行库（编译用户程序时链接）
    libcolorful.cll       加密模块归档（19 个标准库模块 + gui + widgets）
    colorful.dll/.lib     动态运行库（可选，配合 --shared-runtime）
    include/              runtime.h、gui_native.h
    fonts/                13 个字体（GUI 程序运行时读取）
    colorful.json         清单
  stdlib/               标准库源码（19 个模块）
  source/               完整源代码
    compiler/             编译器（词法、语法、类型检查、LLVM 与 C 后端）
    runtime/              C 运行库
    gui/                  gui 与 widgets 模块（声明式 Material 3 组件）
    tools/                七个工具的源码
    tests/                前端测试与端到端测试
    examples/             示例程序
    lib/                  预构建 runtime 副本（供源码目录内跑测试）
    build/                开发脚本（_dev.bat、_stage1.bat、_gui.bat 等）
    CMakeLists.txt  LICENSE  .gitignore
  vscode/
    colorful-lang-0.1.0.vsix    VSCode 插件（内置同一套工具链与 runtime）
  README.md
```

---

## 快速开始：命令行

`bin/` 与 `lib/`、`stdlib/` 同级，工具会自动找到它们，**不需要任何路径参数**：

```bat
cd Colorful-0.1.0
bin\colorc.exe source\examples\hello.cf -o hello.exe
hello.exe
:: Hello, Colorful
```

编译并直接运行：

```bat
bin\colorc.exe source\examples\fizzbuzz.cf --run
```

写自己的程序：

```bat
:: myapp\app.cf
::     def main():
::         print("hi from my app")

bin\colorc.exe myapp\app.cf -o myapp\app.exe
myapp\app.exe
```

查看中间产物（理解编译过程很有用）：

```bat
bin\colorc.exe app.cf --emit c    -o app.c
bin\colorc.exe app.cf --emit llvm -o app.ll
```

---

## 快速开始：VSCode

1. 安装 [Visual Studio Code](https://code.visualstudio.com/)
2. 安装插件并重载窗口：

```bat
code --install-extension vscode\colorful-lang-0.1.0.vsix --force
:: 然后在 VSCode 中执行一次 Developer: Reload Window
```

打开任意 `.cf` 文件，即可获得语法高亮、**打字即时的诊断**、补全、悬停、跳转定义、
查找引用、重命名、格式化、测试资源管理器与任务（构建 / 运行 / 测试）。

插件内已打包工具链，因此**编辑器与命令行互不依赖**：可以只用编辑器，也可以只用命令行。

---

## 工具链速查

| 工具 | 用途 | 示例 |
| --- | --- | --- |
| `colorc` | 编译、运行、输出中间产物 | `bin\colorc.exe app.cf -o app.exe` |
| `colorlsp` | 语言服务器（stdio） | `bin\colorlsp.exe --stdio` |
| `colorfmt` | 格式化 | `bin\colorfmt.exe app.cf`；`--check`；`- --stdout` |
| `colortest` | 测试发现与运行 | `bin\colortest.exe tests\`；`--list`；`--json` |
| `colorpkg` | 包管理 | `bin\colorpkg.exe init` / `build` / `run` / `test` |
| `colorlib` | 重新打包 runtime | `bin\colorlib.exe build --out lib --root source ...` |
| `colordoc` | 生成文档 | `bin\colordoc.exe source\stdlib --out doc` |

查看完整参数：`bin\colorc.exe --help`（其它工具同理）。

`colorc` 常用参数：

```text
-o, --output <path>      输出文件
--emit <kind>            exe | object | llvm | c | wasm | tokens | ast | hir | mir
--backend <kind>         auto | llvm | c
--opt <0-3>              优化级别
--run                    构建后立即运行
-I, --search <dir>       追加模块搜索路径（可重复）
--stdlib <dir>           标准库目录
-L, --lib-dir <dir>      runtime 库目录
```

---

## 验证这套发行包

```bat
:: 1) 编译并运行示例
bin\colorc.exe source\examples\hello.cf -o hello.exe
hello.exe

:: 2) 端到端测试套件（7 个测试文件，应全部通过）
cd source
..\bin\colortest.exe --stdlib ..\stdlib tests\e2e

:: 3) 语言服务器自检
..\bin\colorlsp.exe --version
```

`source/lib/` 是 runtime 的副本：GUI 相关测试按「测试文件上溯两级查找 `lib/fonts`」
的方式定位字体，所以从 `source/` 目录内部运行测试时需要它存在。

---

## runtime 与链接方式

编译用户程序时，`colorc` 默认在 `<exe目录>/../lib` 查找 `libcolorful.lib` 与
`libcolorful.cll`，在 `<exe目录>/../stdlib` 查找标准库。若把 `bin/` 单独搬走，
显式指定即可：

```bat
bin\colorc.exe app.cf -o app.exe -L lib --stdlib stdlib
```

- **静态链接（默认）**：产物是单文件，不依赖 `colorful.dll`。
- **动态链接**：加 `--shared-runtime`，需要把 `lib/colorful.dll` 放在产物旁边。

---

## 从源码构建

Windows（一键脚本，使用 MSVC）：

```bat
cd source
build\_dev.bat
```

CMake（Windows / macOS / Linux）：

```bat
cmake -S source -B build
cmake --build build --config Release
```

构建完成后重新打包 runtime（与随包提供的 `lib/` 同源）：

```bat
bin\colorlib.exe build --out lib --root source ^
    --roboto source\Roboto --symbols "source\Material Symbols-package" --clean
```

源码构建不会改动本包内的 `bin/` 与 `lib/`，可以放心对照。

---

## 说明与已知限制

- 本包只含 **Windows x64** 二进制；macOS 与 Linux 需从 `source/` 用 CMake 构建。
- `bin/` 里的程序是静态链接的，可以单独复制到别处运行；但编译用户程序时仍需要
  `lib/libcolorful.lib` 与 `lib/libcolorful.cll`。
- GUI 程序运行时从 `lib/fonts` 读取 Roboto 与 Material Symbols，缺失时回退到系统字体。
- 语言层面的已知边界：**默认参数值、命名参数、闭包捕获局部变量**尚未实现，
  编译器对前两者会给出相应诊断（CF1128 / CF1130）。这些不影响现有标准库与示例。
- `.cll` 是加密的模块归档；标准库同时以明文形式提供在 `stdlib/`，
  因此可以直接阅读、修改并从源码构建。
