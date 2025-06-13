#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>
using namespace std;

struct Message {
    int generator_id;
    int value;
};

queue<Message> message_queue;
mutex queue_mutex;
condition_variable queue_cv;
atomic<bool> running(true);

void generator_thread(int id, int min_delay, int max_delay, int min_value, int max_value) {
    mt19937 gen(random_device{}());
    uniform_int_distribution<> val_dist(min_value, max_value);
    uniform_int_distribution<> delay_dist(min_delay, max_delay);

    while (running) {
        int value = val_dist(gen);
        {
            lock_guard<mutex> lock(queue_mutex);
            message_queue.push({id, value});
        }
        queue_cv.notify_one();

        this_thread::sleep_for(chrono::milliseconds(delay_dist(gen)));
    }
}

void processor_thread() {
    while (running) {
        unique_lock<mutex> lock(queue_mutex);
        queue_cv.wait(lock, [] { return !message_queue.empty() || !running; });

        while (!message_queue.empty()) {
            Message msg = message_queue.front();
            message_queue.pop();
            cout << "Generator #" << msg.generator_id << " produced value: " << msg.value << "\n";
        }
    }
}

int main() {
    const int N = 5; // кількість генераторів
    vector<thread> generators;
    for (int i = 0; i < N; ++i) {
        generators.emplace_back(generator_thread, i + 1, 100, 1000, 1, 100);
    }

    thread processor(processor_thread);

    cout << "Press Enter to stop...\n";
    cin.get();
    cin.get();

    running = false;
    queue_cv.notify_all();

    for (auto& t : generators) {
        if (t.joinable()) t.join();
    }

    if (processor.joinable()) processor.join();

    return 0;
}
