// // benchmark/benchmark_search.cpp
// #include <benchmark/benchmark.h>
// #include "query_engine.h"      // 直接包含，不需要路径
// #include "text_processor.h"    // 直接包含，不需要路径
// #include "file_reader.h"

// // 基准测试：查询性能
// static void BM_QueryEngine(benchmark::State& state) {
//     QueryEngine engine;
//     engine.load_data("test_data.txt");
    
//     std::string query = "test query";
    
//     for (auto _ : state) {
//         auto results = engine.search(query);
//         benchmark::DoNotOptimize(results);
//     }
// }
// BENCHMARK(BM_QueryEngine)->Range(1, 1 << 10);

// // 基准测试：文本处理
// static void BM_TextProcessor(benchmark::State& state) {
//     TextProcessor processor;
//     std::string text(1024 * state.range(0), 'a');
    
//     for (auto _ : state) {
//         auto result = processor.process(text);
//         benchmark::DoNotOptimize(result);
//     }
// }
// BENCHMARK(BM_TextProcessor)->RangeMultiplier(2)->Range(1, 64);

// // 需要有一个主函数
// BENCHMARK_MAIN();