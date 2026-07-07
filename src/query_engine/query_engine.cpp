#include "query_engine.h"
#include <cctype>
#include <sstream>
#include <algorithm>

// 文本清洗、分词：转小写、剔除标点、按空格分割
std::vector<std::string> cleanAndSplit(const std::string& rawText)
{
    std::vector<std::string> wordList;
    std::stringstream iss(rawText);
    std::string word;

    while (iss >> word)
    {
        std::string cleanWord;
        for (char c : word)
        {
            if (isalpha(c))
                cleanWord += static_cast<char>(tolower(c));
        }
        if (!cleanWord.empty())
            wordList.push_back(cleanWord);
    }
    return wordList;
}

// 核心解析函数：识别双引号短语，区分检索类型
QueryResult parseQuery(const std::string& inputStr)
{
    QueryResult res;
    res.hasPhrase = false;  // 初始化
    
    size_t leftQuote = inputStr.find('"');
    size_t rightQuote = inputStr.find('"', leftQuote + 1);

    // 存在成对双引号 → 短语检索模式
    if (leftQuote != std::string::npos && rightQuote != std::string::npos && rightQuote > leftQuote)
    {
        // 提取引号内的短语
        std::string phraseContent = inputStr.substr(leftQuote + 1, rightQuote - leftQuote - 1);
        
        // 创建短语单元
        QueryUnit phraseUnit;
        phraseUnit.type = QueryUnitType::Phrase;
        phraseUnit.rawContent = phraseContent;
        phraseUnit.words = cleanAndSplit(phraseContent);
        
        res.units.push_back(phraseUnit);
        res.hasPhrase = true;
        
        // 处理引号外的普通词（如果有的话）
        // 引号前的内容
        std::string beforeQuote = inputStr.substr(0, leftQuote);
        // 引号后的内容
        std::string afterQuote = inputStr.substr(rightQuote + 1);
        std::string outsideText = beforeQuote + " " + afterQuote;
        
        // 清洗并添加普通词
        std::vector<std::string> normalWords = cleanAndSplit(outsideText);
        for (const auto& word : normalWords)
        {
            QueryUnit normalUnit;
            normalUnit.type = QueryUnitType::NormalWord;
            normalUnit.rawContent = word;
            normalUnit.words = {word};  // 普通词自身就是分词结果
            res.units.push_back(normalUnit);
        }
    }
    // 无引号 → 普通关键词检索
    else
    {
        res.hasPhrase = false;
        std::vector<std::string> words = cleanAndSplit(inputStr);
        
        for (const auto& word : words)
        {
            QueryUnit normalUnit;
            normalUnit.type = QueryUnitType::NormalWord;
            normalUnit.rawContent = word;
            normalUnit.words = {word};
            res.units.push_back(normalUnit);
        }
    }
    
    return res;
}