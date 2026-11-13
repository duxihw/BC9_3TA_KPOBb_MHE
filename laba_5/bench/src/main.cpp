// =============================================================================
//  grid_bench — сбор данных для исследования (CSV -> python/analyze.py).
//
//    grid_bench [--out DIR] [--quick] [--only copy_move|access|construct]
// =============================================================================

#include <exception>
#include <iostream>
#include <string>

#include "bench/experiments.hpp"
#include "bench/timer.hpp"

#ifndef GRID_DEFAULT_DATA_DIR
#define GRID_DEFAULT_DATA_DIR "../data/laba_5"
#endif

int main(int argc, char** argv) {
    bench::Config cfg;
    cfg.out_dir = GRID_DEFAULT_DATA_DIR;
    std::string only;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--out" && i + 1 < argc) {
            cfg.out_dir = argv[++i];
        } else if (arg == "--quick") {
            cfg.quick = true;
        } else if (arg == "--only" && i + 1 < argc) {
            only = argv[++i];
        } else {
            std::cout << "Usage: " << argv[0]
                      << " [--out DIR] [--quick] [--only copy_move|access|construct]\n";
            return arg == "--help" || arg == "-h" ? 0 : 2;
        }
    }

    try {
        bench::Stopwatch total;
        std::cout << "Output directory: " << cfg.out_dir.string()
                  << (cfg.quick ? "  (quick mode)" : "") << "\n";
        if (only.empty() || only == "copy_move") {
            std::cout << "[1/3] Copy vs move\n";
            bench::run_copy_move(cfg);
        }
        if (only.empty() || only == "construct") {
            std::cout << "[2/3] Construction / allocations\n";
            bench::run_construct(cfg);
        }
        if (only.empty() || only == "access") {
            std::cout << "[3/3] Element access\n";
            bench::run_access(cfg);
        }
        std::cout << "Done in " << total.elapsed_ns() / 1e9 << " s\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
