#ifndef PREPROC_H
#define PREPROC_H

// 固定结构体对齐，跨DLL兼容
#pragma pack(push, 8)

#ifdef FILEPREPROC_EXPORTS
#define PREPROC_API __declspec(dllexport)
#else
#define PREPROC_API __declspec(dllimport)
#endif

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// 单单词-词频结构体
typedef struct WordTfItem
{
    char word[128];     // UTF-8单词，末尾自动补'\0'
    uint32_t tf;        // 文档内词频
} WordTfItem;

// 单篇文档结构化数据
typedef struct DocData
{
    uint32_t docId;             // 全局唯一文档ID
    char filePath[512];         // 文件UTF-8路径，自动补'\0'
    WordTfItem* wordTfArr;      // 词频数组堆指针
    uint32_t wordItemCnt;       // 词频条目总数
} DocData;

#pragma pack(pop)

/**
 * @brief 释放DLL分配的文档数组内存
 * @param docArr DocData数组指针
 * @param docNum 文档数量，<=0直接返回
 */
PREPROC_API void FreeDocDataArray(DocData* docArr, int docNum);

/**
 * @brief 重置全局文档ID，多次构建索引前调用
 */
PREPROC_API void ResetDocId();

/**
 * @brief 批量遍历目录预处理全部txt文件
 * @param dirPath UTF-8目录路径
 * @param outDocArray 输出文档数组指针（堆分配，需调用FreeDocDataArray释放）
 * @param outDocCount 输出有效文档数量
 * @return 状态码
 * >0 成功处理文档数量
 * 0 目录无有效txt文件
 * -1 入参非法（空指针）
 * -2 内存分配失败
 */
PREPROC_API int ProcessTxtFolder(
    const char* dirPath,
    DocData** outDocArray,
    int* outDocCount
);

#ifdef __cplusplus
}
#endif

#endif // PREPROC_H