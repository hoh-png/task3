# MiniSearch

一个基于倒排索引的英文文本检索小程序，支持关键词和短语查询。

## 功能

- 扫描指定文件夹下的所有 `.txt` 文件，自动分词并统计词频
- 构建倒排索引，实现快速文本检索
- 支持普通词查询和引号短语查询
- 交互式命令行界面

## 模块说明

| 模块 | 作用 |
|------|------|
| `file_reader` | 读取 txt 文件，英文分词，统计词频 |
| `query_engine` | 解析用户输入的查询语句，识别普通词和短语 |
| `text_processor` | 构建和加载倒排索引 |

## 环境要求

- CMake 3.15 或以上
- 支持 C++17 的编译器（MinGW / MSVC / GCC）
- Git

## 编译和运行

```bash
# 克隆仓库（注意要带子模块）
git clone --recurse-submodules https://github.com/hoh-png/MiniSearch
cd MiniSearch

# CMake 配置
cmake -B build

# 编译
cmake --build build --target MiniSearch

# 运行（指定一个包含 txt 文件的文件夹）
cd build
.\bin\MiniSearch.exe (/* 此处替换为实际的 txt 文件夹路径 */)
# 例如：.\bin\MiniSearch.exe "C:\Users\HOHUI\Desktop\task3\test"
```

## 使用方法

启动后进入交互模式，输入查询内容即可检索：

```
>> apple
  找到 3 篇匹配文档:
    SnowWhite.txt    (16 次)
    fruits.txt    (1 次)
    mobile-phone.txt    (1 次)

>> airplane
  找到 1 篇匹配文档:
    transports.txt    (1 次)

>> quit
再见!
```

- 普通词（如 `hello world`）：查找包含任意一个词的文档
- 引号短语（如 `"machine learning"`）：查找同时包含所有词的文档
- 输入 `quit` 或 `exit` 退出

## 性能测试 (Benchmark)

```bash
cmake --build build --target mini_search_benchmark
.\bin\mini_search_benchmark.exe
```

## 项目结构

```
MiniSearch/
├── src/
│   ├── main.cpp
│   ├── file_reader/       # 文件读取和分词
│   ├── query_engine/      # 查询解析
│   └── text_processor/    # 倒排索引
├── benchmark/             # 性能测试
├── third_party/           # 第三方库 (Google Benchmark)
└── CMakeLists.txt
```