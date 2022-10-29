#include <iostream>
#include <vector>
#include <condition_variable>

std::mutex val_mutex;
std::condition_variable val_cond;
int box = 0;
bool ready = false;

std::mutex done_mutex;
std::condition_variable done_cond;
bool done = false;

int finalResult = 0;

void produce(const int size, const std::vector<int> &first, const std::vector<int> &second) {
    std::cout << "Producer starts working" << "\n";

    for (int i = 0; i < size; i++) {
        int a = first[i];
        int b = second[i];
        int inter = a * b;

        std::unique_lock<std::mutex> lock(val_mutex);
        val_cond.wait(lock, []() { return !ready; });

        box = inter;
        ready = true;

        std::cout << "Producer put value " << std::to_string(inter) << " in the box\n";
        val_cond.notify_one();
    }

    std::unique_lock<std::mutex> lock(done_mutex);
    std::cout << "Producer tries to close the box\n";
    done_cond.wait(lock, []() { return !ready; });
    done = true;
    std::cout << "Producer has closed the box\n";
    done_cond.notify_one();
}

void consume() {
    std::cout << "Consumer starts working" << "\n";
    int result = 0;

    done_mutex.lock();
    while (!done) {
        done_mutex.unlock();

        std::unique_lock<std::mutex> lock(val_mutex);
        std::cout << "Consumer tries to take value from the box\n";
        val_cond.wait(lock, []() { return ready; });

        result = result + box;
        ready = false;
        std::cout << "Consumer took value " << std::to_string(box) << " from the box\n";

        val_cond.notify_one();

        done_mutex.lock();
    }

    finalResult = result;
}

int main() {
    int size = 5;
    std::vector<int> first{1, 2, 3, 4, 5};
    std::vector<int> second{10, 20, 30, 40, 50};

    std::thread producerThread(produce, size, first, second);
    std::thread consumerThread(consume);

    producerThread.join();
    consumerThread.join();

    std::cout << "Scalar product: " << std::to_string(finalResult);

    return 0;
}
