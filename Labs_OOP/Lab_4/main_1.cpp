#include <iostream>
#include <vector>
#include <numeric>
#include <future>
#include <random>
#include <chrono>
#include <string>

using namespace std;
using namespace std::chrono;

// Функція для обчислення суми елементів підмасиву
long long calculate_partial_sum(const vector<int>& arr, size_t start, size_t end) {
    return accumulate(arr.begin() + start, arr.begin() + end, 0LL);
}

// Генерація випадкового масиву
vector<int> generate_random_array(size_t size, int min_val, int max_val) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(min_val, max_val);

    vector<int> arr(size);
    for (auto& val : arr) {
        val = dis(gen);
    }
    return arr;
}

// Паралельне обчислення суми з використанням std::async
long long parallel_sum(const vector<int>& arr, size_t parts, std::launch policy) {
    size_t n = arr.size();
    size_t chunk_size = n / parts;

    vector<future<long long>> futures;

    for (size_t i = 0; i < parts; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == parts - 1) ? n : start + chunk_size;
        futures.push_back(async(policy, calculate_partial_sum, cref(arr), start, end));
    }

    long long total_sum = 0;
    for (auto& f : futures) {
        total_sum += f.get();
    }
    return total_sum;
}

// Основна функція
int main() {
    size_t array_size;
    size_t parts;
    string policy_input;

    cout << "Enter the array size: ";
    cin >> array_size;

    cout << "Enter the number of parts to split: ";
    cin >> parts;

    cout << "Choose a launch policy (async or deferred): ";
    cin >> policy_input;

    std::launch policy;
    if (policy_input == "async") {
        policy = std::launch::async;
    } else if (policy_input == "deferred") {
        policy = std::launch::deferred;
    } else {
        cerr << "Невірна політика запуску! Використовується async за замовчуванням.\n";
        policy = std::launch::async;
    }

    const int min_value = 1;
    const int max_value = 100;

    // Генерація масиву
    vector<int> arr = generate_random_array(array_size, min_value, max_value);

    // Послідовна обробка
    auto start_time = high_resolution_clock::now();
    long long seq_sum = calculate_partial_sum(arr, 0, arr.size());
    auto end_time = high_resolution_clock::now();
    auto seq_duration = duration_cast<milliseconds>(end_time - start_time).count();
    cout << "Sequential sum: " << seq_sum << ", Time: " << seq_duration << " ms\n";

    // Паралельна обробка
    start_time = high_resolution_clock::now();
    long long par_sum = parallel_sum(arr, parts, policy);
    end_time = high_resolution_clock::now();
    auto par_duration = duration_cast<milliseconds>(end_time - start_time).count();

    cout << "Parallel sum: " << par_sum << ", Time: " << par_duration << " ms\n";

    return 0;
}
