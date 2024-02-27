import benchmark_util as bench_util 
import os


def run_benchmarks(repeats: int = 3) -> None:
    '''
    Run the benchmark suite, repeats indicates the number of times we test a file
    '''
    # Gets the dir to PhotoshopBenchmark/
    base_dir = os.path.join(os.path.dirname(__file__), "../")

    # Store the benchmark name as well as in- and output- file paths in that order,
    # we start with the 32-bit files as these need manual supervision
    benchmark_items = {
        "Automotive Data (32-bit) ~3.65GB" : ["documents/read/large_file_32bit.psb", "documents/write/large_file_32bit.psb"],
        "Automotive Data (8-bit) ~1.27GB" : ["documents/read/large_file_8bit.psb", "documents/write/large_file_8bit.psb"],
        "Automotive Data (16-bit) ~1.97GB" : ["documents/read/large_file_16bit.psb", "documents/write/large_file_16bit.psb"],
        "Glacius Hyundai Sample (8-bit) ~.75GB" : ["documents/read/HyundaiGenesis_GlaciusCreations_8bit.psd", "documents/write/HyundaiGenesis_GlaciusCreations_8bit.psd"],
        "Deep Nested Layers (8-bit) ~.5GB" : ["documents/read/deep_nesting_8bit.psb", "documents/write/deep_nesting_8bit.psb"],
    }

    for bench_name in benchmark_items:
        in_file_path = os.path.join(base_dir, benchmark_items[bench_name][0])
        out_file_path = os.path.join(base_dir, benchmark_items[bench_name][1])
        bench_harness = bench_util.PhotoshopTestHarness(in_file_path, out_file_path, bench_name)
        # Its perfectly safe to repeat the benchmark in this fashion as run() closes and opens photoshop
        for _i in range(repeats):
            bench_harness.run()


if __name__ == "__main__":
    run_benchmarks()