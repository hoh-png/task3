#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <map>
#include <windows.h>
#include "file_reader.h"
#include "query_engine.h"
#include "text_processor.h"

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::string dirPathBuf;
    if (argc >= 2) {
        dirPathBuf = argv[1];
    } else {
        std::cout << "请输入 txt 文件夹路径: ";
        std::getline(std::cin, dirPathBuf);
        if (dirPathBuf.empty()) {
            std::cerr << "错误: 未输入路径" << std::endl;
            system("pause");
            return 1;
        }
    }
    const char* dirPath = dirPathBuf.c_str();

    // 第一步：用 FilePreproc 读取文件夹，分词
    std::cout << "[1/3] 扫描文件夹: " << dirPath << std::endl;

    ResetDocId();
    DocData* docArr = nullptr;
    int docCount = 0;

    int ret = ProcessTxtFolder(dirPath, &docArr, &docCount);
    if (ret <= 0) {
        std::cerr << "错误: 没有找到有效的 txt 文件 (code=" << ret << ")" << std::endl;
        system("pause");
        return 1;
    }
    std::cout << "  找到 " << docCount << " 篇文档" << std::endl;

    // 第二步：DocData → DocInfo，用 TextProcessor 建倒排索引
    std::cout << "[2/3] 构建倒排索引..." << std::endl;

    std::vector<DocInfo> docInfoList;
    docInfoList.reserve(docCount);
    std::map<int, std::string> docPaths;

    for (int i = 0; i < docCount; ++i) {
        DocInfo info;
        info.doc_id = static_cast<int>(docArr[i].docId);
        docPaths[info.doc_id] = docArr[i].filePath;
        for (uint32_t j = 0; j < docArr[i].wordItemCnt; ++j) {
            info.word_tf[docArr[i].wordTfArr[j].word] =
                static_cast<int>(docArr[i].wordTfArr[j].tf);
        }
        docInfoList.push_back(info);
    }

    const std::string indexFile = "invert_index.bin";
    build_invert_index(docInfoList, indexFile);

    InvertMap invertMap = load_invert_index(indexFile);
    int totalDocs = get_total_doc_count();

    // 释放 DocData 数组，后面不需要了
    FreeDocDataArray(docArr, docCount);

    std::cout << "  索引就绪，共 " << totalDocs << " 篇文档，"
              << invertMap.size() << " 个不同的词" << std::endl;

    // 第三步：交互式检索循环
    std::cout << "[3/3] 开始检索 (输入 quit 退出)" << std::endl;
    std::cout << "========================================" << std::endl;

    std::string input;
    while (true) {
        std::cout << "\n>> ";
        std::getline(std::cin, input);
        if (input.empty()) continue;
        if (input == "quit" || input == "exit") break;

        QueryResult query = parseQuery(input);

        if (query.units.empty()) {
            std::cout << "  (无法解析查询，请重新输入)" << std::endl;
            continue;
        }

        // 收集所有匹配的文档 ID
        std::set<int> matchedDocs;
        bool firstUnit = true;

        for (const auto& unit : query.units) {
            if (unit.type == QueryUnitType::NormalWord) {
                // 普通词：找包含该词的文档
                std::set<int> unitDocs;
                for (const auto& word : unit.words) {
                    auto it = invertMap.find(word);
                    if (it != invertMap.end()) {
                        for (const auto& posting : it->second) {
                            unitDocs.insert(posting.doc_id);
                        }
                    }
                }
                if (firstUnit) {
                    matchedDocs = unitDocs;
                    firstUnit = false;
                } else {
                    // 多个普通词取并集
                    matchedDocs.insert(unitDocs.begin(), unitDocs.end());
                }

            } else if (unit.type == QueryUnitType::Phrase) {
                // 短语：找同时包含所有词的文档（取交集）
                std::set<int> phraseDocs;
                bool firstWord = true;

                for (const auto& word : unit.words) {
                    auto it = invertMap.find(word);
                    if (it == invertMap.end()) {
                        phraseDocs.clear();
                        break;
                    }
                    std::set<int> wordDocs;
                    for (const auto& posting : it->second) {
                        wordDocs.insert(posting.doc_id);
                    }
                    if (firstWord) {
                        phraseDocs = wordDocs;
                        firstWord = false;
                    } else {
                        std::set<int> intersection;
                        std::set_intersection(
                            phraseDocs.begin(), phraseDocs.end(),
                            wordDocs.begin(), wordDocs.end(),
                            std::inserter(intersection, intersection.begin()));
                        phraseDocs = intersection;
                    }
                }

                if (firstUnit) {
                    matchedDocs = phraseDocs;
                    firstUnit = false;
                } else {
                    matchedDocs.insert(phraseDocs.begin(), phraseDocs.end());
                }
            }
        }

        // 收集所有查询词
        std::set<std::string> queryWords;
        for (const auto& unit : query.units) {
            for (const auto& word : unit.words) {
                queryWords.insert(word);
            }
        }

        // 显示结果
        if (matchedDocs.empty()) {
            std::cout << "  未找到匹配的文档" << std::endl;
        } else {
            // 计算每篇文档的相关度得分（查询词TF之和）
            struct ScoredDoc {
                int docId;
                int totalTf;
            };
            std::vector<ScoredDoc> scoredDocs;
            for (int docId : matchedDocs) {
                int totalTf = 0;
                for (const auto& word : queryWords) {
                    auto it = invertMap.find(word);
                    if (it != invertMap.end()) {
                        for (const auto& posting : it->second) {
                            if (posting.doc_id == docId) {
                                totalTf += posting.tf;
                            }
                        }
                    }
                }
                scoredDocs.push_back({docId, totalTf});
            }

            // 按相关度降序排序
            std::sort(scoredDocs.begin(), scoredDocs.end(),
                [](const ScoredDoc& a, const ScoredDoc& b) {
                    return a.totalTf > b.totalTf;
                });

            std::cout << "  找到 " << matchedDocs.size() << " 篇匹配文档:" << std::endl;
            for (const auto& sd : scoredDocs) {
                std::string path = docPaths[sd.docId];
                size_t lastSlash = path.find_last_of("\\/");
                std::string fileName = (lastSlash != std::string::npos)
                    ? path.substr(lastSlash + 1) : path;
                std::cout << "    " << fileName << "    "
                          << "(" << sd.totalTf << " 次)" << std::endl;
            }
        }
    }

    std::cout << "再见!" << std::endl;
    system("pause");
    return 0;
}