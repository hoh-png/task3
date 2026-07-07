#define FILEPREPROC_EXPORTS
#include "file_reader.h"

#include <windows.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <cctype>
#include <cstring>
#include <stdexcept>

// =========================
// 全局状态
// =========================
static uint32_t g_docId = 0;

PREPROC_API void ResetDocId()
{
    g_docId = 0;
}

// =========================
// 工具函数
// =========================

// 安全字符串拷贝（UTF-8）
static void SafeStrCopy(char* dest, size_t destSize, const char* src)
{
    if (!dest || destSize == 0 || !src)
        return;
    strncpy_s(dest, destSize, src, destSize - 1);
    dest[destSize - 1] = '\0';
}

// 清洗 + 英文分词
static std::vector<std::string> CleanAndTokenize(const std::string& raw)
{
    std::vector<std::string> tokens;
    std::string word;

    for (unsigned char ch : raw)
    {
        if (std::isdigit(ch))
            continue;

        if (std::ispunct(ch) || std::isspace(ch))
        {
            if (!word.empty())
            {
                for (auto& c : word)
                    c = static_cast<char>(std::tolower(c));
                tokens.push_back(word);
                word.clear();
            }
            continue;
        }
        word += static_cast<char>(ch);
    }

    if (!word.empty())
    {
        for (auto& c : word)
            c = static_cast<char>(std::tolower(c));
        tokens.push_back(word);
    }
    return tokens;
}

// UTF-8 → 宽字符
static std::wstring Utf8ToWide(const char* utf8)
{
    if (!utf8 || utf8[0] == '\0')
        return L"";

    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    if (len <= 0)
        return L"";

    std::wstring buf(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &buf[0], len);
    return buf;
}

// 读取 UTF-8 文本文件（支持中文路径）
static std::string ReadTxtFile(const std::wstring& widePath)
{
    HANDLE hFile = CreateFileW(
        widePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (hFile == INVALID_HANDLE_VALUE)
        return "";

    DWORD size = GetFileSize(hFile, nullptr);
    if (size == 0)
    {
        CloseHandle(hFile);
        return "";
    }

    std::string buffer(size, '\0');
    DWORD read = 0;
    BOOL ok = ReadFile(hFile, &buffer[0], size, &read, nullptr);
    CloseHandle(hFile);

    if (!ok || read != size)
        return "";

    // 去除 UTF-8 BOM
    if (buffer.size() >= 3 &&
        static_cast<uint8_t>(buffer[0]) == 0xEF &&
        static_cast<uint8_t>(buffer[1]) == 0xBB &&
        static_cast<uint8_t>(buffer[2]) == 0xBF)
    {
        return buffer.substr(3);
    }

    return buffer;
}

// 扫描一级目录下的 txt 文件
static std::vector<std::wstring> ScanTxtFiles(const std::wstring& wideDir)
{
    std::vector<std::wstring> files;
    std::wstring pattern = wideDir + L"\\*.txt";

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(pattern.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE)
        return files;

    do
    {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            files.push_back(wideDir + L"\\" + fd.cFileName);
    } while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
    return files;
}

// =========================
// 导出接口
// =========================

PREPROC_API void FreeDocDataArray(DocData* arr, int docNum)
{
    if (!arr || docNum <= 0)
        return;

    for (int i = 0; i < docNum; ++i)
        delete[] arr[i].wordTfArr;

    delete[] arr;
}

PREPROC_API int ProcessTxtFolder(
    const char* dirPath,
    DocData** outDocArray,
    int* outDocCount)
{
    if (!dirPath || !outDocArray || !outDocCount)
        return -1;

    *outDocArray = nullptr;
    *outDocCount = 0;

    try
    {
        std::wstring wideDir = Utf8ToWide(dirPath);
        std::vector<std::wstring> files = ScanTxtFiles(wideDir);

        if (files.empty())
            return 0;

        // 第一遍：解析 + 统计
        struct DocCache
        {
            std::wstring path;
            std::unordered_map<std::string, uint32_t> tf;
        };
        std::vector<DocCache> cache;
        cache.reserve(files.size());

        for (const auto& wpath : files)
        {
            std::string text = ReadTxtFile(wpath);
            if (text.empty())
                continue;

            auto tokens = CleanAndTokenize(text);
            if (tokens.empty())
                continue;

            std::unordered_map<std::string, uint32_t> tf;
            for (const auto& word : tokens)
                tf[word]++;

            cache.emplace_back(DocCache{wpath, std::move(tf)});
        }

        if (cache.empty())
            return 0;

        // 第二遍：一次性分配
        DocData* docs = new DocData[cache.size()]{};
        uint32_t docIdx = 0;

        for (const auto& item : cache)
        {
            DocData& doc = docs[docIdx];
            doc.docId = ++g_docId;

            // 路径：wide → UTF-8
            int utf8Len = WideCharToMultiByte(
                CP_UTF8, 0,
                item.path.c_str(), -1,
                nullptr, 0, nullptr, nullptr);
            std::string utf8Path(utf8Len, '\0');
            WideCharToMultiByte(
                CP_UTF8, 0,
                item.path.c_str(), -1,
                &utf8Path[0], utf8Len,
                nullptr, nullptr);
            SafeStrCopy(doc.filePath, sizeof(doc.filePath), utf8Path.c_str());

            doc.wordItemCnt = static_cast<uint32_t>(item.tf.size());
            doc.wordTfArr = new WordTfItem[doc.wordItemCnt]{};

            uint32_t wordIdx = 0;
            for (const auto& kv : item.tf)
            {
                SafeStrCopy(
                    doc.wordTfArr[wordIdx].word,
                    sizeof(doc.wordTfArr[wordIdx].word),
                    kv.first.c_str());
                doc.wordTfArr[wordIdx].tf = kv.second;
                ++wordIdx;
            }

            ++docIdx;
        }

        *outDocArray = docs;
        *outDocCount = static_cast<int>(cache.size());
        return static_cast<int>(cache.size());
    }
    catch (const std::bad_alloc&)
    {
        // 内存分配失败：统一返回 -2
        return -2;
    }
    catch (...)
    {
        // 其他异常：视为处理失败
        return -2;
    }
}