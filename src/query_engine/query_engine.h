#pragma once
#include <vector>
#include <string>

#ifdef _WIN32
#ifdef QUERY_PARSE_EXPORTS
#define QUERY_API __declspec(dllexport)
#else
#define QUERY_API __declspec(dllimport)
#endif
#else
#define QUERY_API
#endif

// 检索单元类型
enum class QueryUnitType
{
    NormalWord,
    Phrase
};

// 单个检索单元：普通词 / 短语
struct QueryUnit
{
    QueryUnitType type;
    std::vector<std::string> words; // 分词后的内容
    std::string rawContent;         // 原始文本
};

// 完整解析结果
struct QueryResult
{
    std::vector<QueryUnit> units;   // 所有检索单元（支持普通词+短语混合）
    bool hasPhrase;                 // 是否包含短语（快速判断标记）
};

// 核心接口：解析输入检索语句，自动识别普通词与带引号短语
QUERY_API QueryResult parseQuery(const std::string& inputStr);

// 工具接口：文本清洗、分词（统一预处理规则）
QUERY_API std::vector<std::string> cleanAndSplit(const std::string& rawText);