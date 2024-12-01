#include <tbb/tbb.h>
#include <vector>
#include <random>
#include <iostream>
#include <chrono>

int const block = 1000;
int const runs = 5;
int const correctness = 20;
int const threads = 4;

void quicksort(std::vector<int>& data, int low, int high) {
    if (low < high) {
        int mid = data[high];
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (data[j] < mid) {
                i++;
                std::swap(data[i], data[j]);
            }
        }
        int midIndex = i + 1;
        std::swap(data[midIndex], data[high]);
        quicksort(data, low, midIndex - 1);
        quicksort(data, midIndex + 1, high);
    }
}

void parallel_quicksort(std::vector<int>& data, int low, int high) {
    if (low < high) {
        int mid = data[high];
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (data[j] < mid) {
                i++;
                std::swap(data[i], data[j]);
            }
        }
        int midIndex = i + 1;
        std::swap(data[midIndex], data[high]);

        if (high - low < block) {
            parallel_quicksort(data, low, midIndex - 1);
            parallel_quicksort(data, midIndex + 1, high);
            return;
        }

        tbb::parallel_invoke(
                [&data, low, midIndex] { parallel_quicksort(data, low, midIndex - 1); },
                [&data, midIndex, high] { parallel_quicksort(data, midIndex + 1, high); }
        );
    }
}

int main() {
    tbb::global_control control(tbb::global_control::max_allowed_parallelism, threads);


    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distrib(0, 1e9*2);

    for (int c = 0; c < correctness; c++) {
        const int size = 1e5;
        std::vector<int> par(size);
        std::vector<int> seq(size);
        std::vector<int> stdS(size);
        for (int i = 0; i < size; i++) {
            par[i] = seq[i] = stdS[i] = distrib(gen);
        }
        quicksort(seq, 0, size - 1);
        parallel_quicksort(par, 0, size - 1);
        std::sort(stdS.begin(), stdS.end());
        assert(seq == par && par == stdS);
    }

    std::cout << "Correctness passed\n\n";

    const int size = 1e8;
    std::vector<int> data(size);
    std::chrono::duration<double> par_time{};
    std::chrono::duration<double> seq_time{};


    for (int run = 0; run < runs; run++) {
        for (auto& d : data)
            d = distrib(gen);
        auto start = std::chrono::steady_clock::now();
        parallel_quicksort(data, 0, size - 1);
        auto finish = std::chrono::steady_clock::now();
        std::chrono::duration<double> result = finish - start;
        std::cout << "Par time, run " << run + 1 << ": " << result.count() << " seconds.\n";
        par_time += result;
    }

    auto midP = par_time.count() / 5.0;
    std::cout << "Parallel mid time: " << midP << "\n";

    for (int run = 0; run < runs; run++) {
        for (auto& d : data)
            d = distrib(gen);
        auto start = std::chrono::steady_clock::now();
        quicksort(data, 0, size - 1);
        auto finish = std::chrono::steady_clock::now();
        std::chrono::duration<double> result = finish - start;
        std::cout << "Seq time, run " << run + 1 << ": " << result.count() << " seconds.\n";
        seq_time += result;
    }

    auto midS = seq_time.count() / 5.0;
    std::cout << "Sequence mid time: " << midS << "\n";

    assert(midP * 3 < midS);

    std::cout << "Performance passed, perform = " << midS / midP;

    return 0;
}