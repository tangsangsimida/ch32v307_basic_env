@echo off
REM ============================================================================
REM VS Code环境配置脚本 (Windows)
REM
REM 此脚本从env.cmake读取工具链路径，自动生成VS Code配置文件。
REM 用法: setup_vscode.bat
REM ============================================================================

setlocal enabledelayedexpansion

REM 从env.cmake读取TOOLCHAIN_PATH
set "TOOLCHAIN_PATH="
for /f "tokens=2 delims=()" %%a in ('findstr /C:"set(TOOLCHAIN_PATH" env.cmake') do (
    set "line=%%a"
    REM 提取引号内的路径
    for /f "tokens=2 delims===" %%b in ("!line!") do (
        set "path=%%b"
        REM 移除引号和空格
        set "path=!path:"=!"
        set "path=!path: =!"
        set "TOOLCHAIN_PATH=!path!"
    )
)

if "!TOOLCHAIN_PATH!"=="" (
    echo 错误: 无法从env.cmake读取TOOLCHAIN_PATH
    pause
    exit /b 1
)

echo 工具链路径: !TOOLCHAIN_PATH!

REM 创建.vscode目录
if not exist ".vscode" mkdir .vscode

REM 生成settings.json
(
echo {
echo     // clangd配置（推荐）
echo     "clangd.path": "clangd",
echo     "clangd.arguments": [
echo         "--compile-commands-dir=${workspaceFolder}/build",
echo         "--header-insertion=never",
echo         "--query-driver=!TOOLCHAIN_PATH!/*"
echo     ],
echo.
echo     // C/C++扩展配置（备用）
echo     "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
echo     "C_Cpp.default.compileCommands": "${workspaceFolder}/build/compile_commands.json",
echo.
echo     // CMake配置
echo     "cmake.buildDirectory": "${workspaceFolder}/build",
echo     "cmake.configureArgs": [
echo         "-DCMAKE_TOOLCHAIN_FILE=${workspaceFolder}/toolchain-riscv.cmake"
echo     ],
echo     "cmake.environment": {
echo         "PATH": "!TOOLCHAIN_PATH!;${env:PATH}"
echo     },
echo.
echo     // 文件关联
echo     "files.associations": {
echo         "*.h": "c",
echo         "*.c": "c"
echo     },
echo.
echo     // 排除build目录
echo     "files.exclude": {
echo         "build": true
echo     }
echo }
) > .vscode\settings.json

REM 生成tasks.json
(
echo {
echo     "version": "2.0.0",
echo     "tasks": [
echo         {
echo             "label": "CMake: 配置",
echo             "type": "shell",
echo             "command": "cmake",
echo             "args": [
echo                 "-B",
echo                 "build",
echo                 "-DCMAKE_TOOLCHAIN_FILE=${workspaceFolder}/toolchain-riscv.cmake",
echo                 "-DCMAKE_BUILD_TYPE=Debug"
echo             ],
echo             "group": "build",
echo             "problemMatcher": [],
echo             "detail": "CMake配置项目",
echo             "options": {
echo                 "env": {
echo                     "PATH": "!TOOLCHAIN_PATH!;${env:PATH}"
echo                 }
echo             }
echo         },
echo         {
echo             "label": "CMake: 构建",
echo             "type": "shell",
echo             "command": "cmake",
echo             "args": ["--build", "build"],
echo             "group": {
echo                 "kind": "build",
echo                 "isDefault": true
echo             },
echo             "problemMatcher": ["$gcc"],
echo             "detail": "构建项目",
echo             "dependsOn": "CMake: 配置",
echo             "options": {
echo                 "env": {
echo                     "PATH": "!TOOLCHAIN_PATH!;${env:PATH}"
echo                 }
echo             }
echo         },
echo         {
echo             "label": "CMake: 清理",
echo             "type": "shell",
echo             "command": "if exist build rd /s /q build",
echo             "group": "build",
echo             "problemMatcher": [],
echo             "detail": "清理构建目录"
echo         },
echo         {
echo             "label": "CMake: 重新构建",
echo             "type": "shell",
echo             "command": "if exist build rd /s /q build ^&^& cmake -B build -DCMAKE_TOOLCHAIN_FILE=${workspaceFolder}/toolchain-riscv.cmake -DCMAKE_BUILD_TYPE=Debug ^&^& cmake --build build",
echo             "group": "build",
echo             "problemMatcher": ["$gcc"],
echo             "detail": "清理并重新构建",
echo             "options": {
echo                 "env": {
echo                     "PATH": "!TOOLCHAIN_PATH!;${env:PATH}"
echo                 }
echo             }
echo         },
echo         {
echo             "label": "CMake: 烧录",
echo             "type": "shell",
echo             "command": "cmake",
echo             "args": ["--build", "build", "--target", "flash"],
echo             "group": "build",
echo             "problemMatcher": [],
echo             "detail": "烧录到MCU",
echo             "options": {
echo                 "env": {
echo                     "PATH": "!TOOLCHAIN_PATH!;${env:PATH}"
echo                 }
echo             }
echo         }
echo     ]
echo }
) > .vscode\tasks.json

REM 生成keybindings.json
(
echo [
echo     {
echo         "key": "f7",
echo         "command": "workbench.action.tasks.runTask",
echo         "args": "CMake: 构建"
echo     },
echo     {
echo         "key": "ctrl+shift+b",
echo         "command": "workbench.action.tasks.runTask",
echo         "args": "CMake: 构建"
echo     },
echo     {
echo         "key": "ctrl+shift+c",
echo         "command": "workbench.action.tasks.runTask",
echo         "args": "CMake: 清理"
echo     },
echo     {
echo         "key": "ctrl+shift+r",
echo         "command": "workbench.action.tasks.runTask",
echo         "args": "CMake: 重新构建"
echo     },
echo     {
echo         "key": "ctrl+shift+f",
echo         "command": "workbench.action.tasks.runTask",
echo         "args": "CMake: 烧录"
echo     }
echo ]
) > .vscode\keybindings.json

REM 生成extensions.json
(
echo {
echo     "recommendations": [
echo         "llvm-vs-code-extensions.vscode-clangd",
echo         "ms-vscode.cpptools",
echo         "ms-vscode.cmake-tools",
echo         "twxs.cmake"
echo     ]
echo }
) > .vscode\extensions.json

echo.
echo VS Code配置已生成完成！
echo 请重新加载VS Code使配置生效。
echo.
pause
