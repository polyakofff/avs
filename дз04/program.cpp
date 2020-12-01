#include <iostream>
#include <chrono>
#include <math.h>
#include <omp.h>
#include <random>

using namespace std;

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count()); // для рандома

/**
 * класс библиотеки университета
 * M - число рядов, N - число шкафов в ряду, K - число книг в шкафу
 * bookcases - булевый трех-мерный массив,
 * где bookcases[i][j][k] - есть ли книга в (i,j,k)-ой ячейке
 */
struct Library {
    int M, N, K;
    bool ***bookcases;

    // конструктор
    Library(int M, int N, int K) : M(M), N(N), K(K) {
        bookcases = new bool **[M];
        for (int i = 0; i < M; i++) {
            bookcases[i] = new bool *[N];
            for (int j = 0; j < N; j++) {
                bookcases[i][j] = new bool[K];
            }
        }
    }

    // деструктор
    ~Library() {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                delete[] bookcases[i][j];
            }
            delete[] bookcases[i];
        }
        delete[] bookcases;
    }
};

int main() {
    system("chcp 65001");
    cout << "start\n";

    int M, N, K, nThreads;
    cout << "Введите кол-во рядов, шкафов в ряду, книг в шкафу и число студентов\n";
    // при nThreads > 'число локальных ядер' улучшений не будет
    cin >> M >> N >> K >> nThreads;

    Library lib(M, N, K);
    // заполняю рандомно ячейки true или false
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < K; k++) {
                lib.bookcases[i][j][k] = (bool) uniform_int_distribution<int>(0, 2)(rng);
                cout << "(" << i << "," << j << "," << k << ") = " << lib.bookcases[i][j][k] << "\n";
            }
        }
    }

    // разделяю все задачи между процессами
    int i, j, k;
    #pragma omp parallel for private(i, j, k) num_threads(nThreads)
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            for (k = 0; k < K; k++) {
                // если в текущей ячейке нет книги
                if (!lib.bookcases[i][j][k]) {

                    #pragma omp critical
                    {
                        int id = omp_get_thread_num();
                        cout << "Студент " << id << ": идет покупать книгу (" << i << "," << j << "," << k << ") в библиотеку\n";
                    }

                    // имитация какого-то процесса (студент идет покупать книгу)
                    int nOps = 1e7;
                    while (nOps) {
                        double smth = sqrt(nOps * 7 + 23);
                        nOps--;
                    }

                    lib.bookcases[i][j][k] = true;

                    #pragma omp critical
                    {
                        int id = omp_get_thread_num();
                        cout << "Студент " << id << ": отдает книгу (" << i << "," << j << "," << k << ") в библиотеку\n";
                    }

                }
            }
        }
    }

    cout << "finish\n";
    return 0;
}
