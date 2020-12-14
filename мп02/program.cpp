#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>
#include <random>

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count()); // для рандома

int N = 2, M = 2;
bool **garden;
std::mutex **cellLocks;
std::mutex printLock;

// проверка валидности хода
bool isValidMove(int i, int j, int di, int dj) {
    return i + di >= 0 && i + di < N && j + dj >= 0 && j + dj < M;
}

// класс садовника
class Gardener {
private:
    int id;

public:
    Gardener(int id) : id(id) {}

    // обработка сада
    void work(int i, int j, int di, int dj) {
        for (;;) {
            // фиксируем текущую клетку сада
            cellLocks[i][j].lock();
            if (!garden[i][j]) {
                int workTime = std::uniform_int_distribution<int>(0, 999)(rng);
                printLock.lock();
                std::cout << "Gardener: " << id << " started working in cell (" << i << "," << j << ") for " << workTime / 60 << " minutes\n";
                printLock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(workTime));
                garden[i][j] = true;
            } else {
                printLock.lock();
                std::cout << "Gardener: " << id << " skipped cell (" << i << "," << j << ")\n";
                printLock.unlock();
            }
            cellLocks[i][j].unlock();

            // переходим к следующей клетке
            if (isValidMove(i, j, di, dj)) {
                i += di;
                j += dj;
            } else {
                if (id == 0) {
                    if (i < N - 1) {
                        i++;
                        dj *= -1;
                    } else {
                        break;
                    }
                } else {
                    if (j > 0) {
                        j--;
                        di *= -1;
                    } else {
                        break;
                    }
                }
            }
        }
    }
};

int main(int argc, char** argv) {
    N = std::stoi(argv[1]);
    M = std::stoi(argv[2]);

    if (N <= 0 || M <= 0 || N * M > 1e6) {
        std::cout << "Incorrect input data\n";
        return -1;
    }

    // аллокация памяти
    garden = new bool *[N];
    cellLocks = new std::mutex *[N];
    for (int i = 0; i < N; i++) {
        garden[i] = new bool[M];
        memset(garden[i], false, sizeof(bool) * M);
        cellLocks[i] = new std::mutex[M];
    }

    std::thread gardeners[2];
    gardeners[0] = std::thread(&Gardener::work, Gardener(0), 0, 0, 0, 1);
    gardeners[1] = std::thread(&Gardener::work, Gardener(1), N - 1, M - 1, -1, 0);
    gardeners[0].join();
    gardeners[1].join();

    // деаллокация памяти
    for (int i = 0; i < N; i++) {
        delete[] garden[i];
        delete[] cellLocks[i];
    }
    delete[] garden;
    delete[] cellLocks;


    return 0;
}
