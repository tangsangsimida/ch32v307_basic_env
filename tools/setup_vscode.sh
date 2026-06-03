#!/bin/bash
# ============================================================================
# VS Code环境配置脚本 (Linux/macOS)
#
# 此脚本从env.cmake读取工具链路径，自动生成VS Code配置文件。
# 用法: ./tools/setup_vscode.sh
# ============================================================================

# 从env.cmake读取TOOLCHAIN_PATH
TOOLCHAIN_PATH=$(grep "set(TOOLCHAIN_PATH" env.cmake | sed 's/.*"\(.*\)".*/\1/')

if [ -z "$TOOLCHAIN_PATH" ]; then
    echo "错误: 无法从env.cmake读取TOOLCHAIN_PATH"
    exit 1
fi

echo "工具链路径: $TOOLCHAIN_PATH"

# 创建.vscode目录
mkdir -p .vscode

# 生成settings.json
cat > .vscode/settings.json << EOF
{
    // clangd配置（推荐）
    "clangd.path": "/usr/bin/clangd",
    "clangd.arguments": [
        "--compile-commands-dir=\${workspaceFolder}/build",
        "--header-insertion=never",
        "--query-driver=${TOOLCHAIN_PATH}/*"
    ],

    // C/C++扩展配置（备用）
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "C_Cpp.default.compileCommands": "\${workspaceFolder}/build/compile_commands.json",

    // CMake配置
    "cmake.buildDirectory": "\${workspaceFolder}/build",
    "cmake.configureArgs": [
        "-DCMAKE_TOOLCHAIN_FILE=\${workspaceFolder}/toolchain-riscv.cmake"
    ],
    "cmake.environment": {
        "PATH": "${TOOLCHAIN_PATH}:\${env:PATH}"
    },

    // 文件关联
    "files.associations": {
        "*.h": "c",
        "*.c": "c"
    },

    // 排除build目录
    "files.exclude": {
        "build": true
    }
}
EOF

# 生成tasks.json
cat > .vscode/tasks.json << EOF
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake: 配置",
            "type": "shell",
            "command": "cmake",
            "args": [
                "-B",
                "build",
                "-DCMAKE_TOOLCHAIN_FILE=\${workspaceFolder}/toolchain-riscv.cmake",
                "-DCMAKE_BUILD_TYPE=Debug"
            ],
            "group": "build",
            "problemMatcher": [],
            "detail": "CMake配置项目",
            "options": {
                "env": {
                    "PATH": "${TOOLCHAIN_PATH}:\${env:PATH}"
                }
            }
        },
        {
            "label": "CMake: 构建",
            "type": "shell",
            "command": "cmake --build build",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["\$gcc"],
            "detail": "构建项目",
            "dependsOn": "CMake: 配置",
            "options": {
                "env": {
                    "PATH": "${TOOLCHAIN_PATH}:\${env:PATH}"
                }
            }
        },
        {
            "label": "CMake: 清理",
            "type": "shell",
            "command": "rm -rf build",
            "group": "build",
            "problemMatcher": [],
            "detail": "清理构建目录"
        },
        {
            "label": "CMake: 重新构建",
            "type": "shell",
            "command": "rm -rf build && cmake -B build -DCMAKE_TOOLCHAIN_FILE=\${workspaceFolder}/toolchain-riscv.cmake -DCMAKE_BUILD_TYPE=Debug && cmake --build build",
            "group": "build",
            "problemMatcher": ["\$gcc"],
            "detail": "清理并重新构建",
            "options": {
                "env": {
                    "PATH": "${TOOLCHAIN_PATH}:\${env:PATH}"
                }
            }
        },
        {
            "label": "CMake: 烧录",
            "type": "shell",
            "command": "cmake --build build --target flash",
            "group": "build",
            "problemMatcher": [],
            "detail": "烧录到MCU",
            "options": {
                "env": {
                    "PATH": "${TOOLCHAIN_PATH}:\${env:PATH}"
                }
            }
        }
    ]
}
EOF

# 生成keybindings.json
cat > .vscode/keybindings.json << EOF
[
    {
        "key": "f7",
        "command": "workbench.action.tasks.runTask",
        "args": "CMake: 构建"
    },
    {
        "key": "ctrl+shift+b",
        "command": "workbench.action.tasks.runTask",
        "args": "CMake: 构建"
    },
    {
        "key": "ctrl+shift+c",
        "command": "workbench.action.tasks.runTask",
        "args": "CMake: 清理"
    },
    {
        "key": "ctrl+shift+r",
        "command": "workbench.action.tasks.runTask",
        "args": "CMake: 重新构建"
    },
    {
        "key": "ctrl+shift+f",
        "command": "workbench.action.tasks.runTask",
        "args": "CMake: 烧录"
    }
]
EOF

# 生成extensions.json
cat > .vscode/extensions.json << EOF
{
    "recommendations": [
        "llvm-vs-code-extensions.vscode-clangd",
        "ms-vscode.cpptools",
        "ms-vscode.cmake-tools",
        "twxs.cmake"
    ]
}
EOF

echo "VS Code配置已生成完成！"
echo "请重新加载VS Code使配置生效。"
