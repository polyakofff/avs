#include <iostream>
#include <random>
#include <chrono>
#include <stack>
#include <thread>
#include <mutex>
#include <fstream>

using namespace std;

//ofstream out("output.txt");
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count()); // для рандома
mutex mutGet;
mutex mutSet;
mutex mutPrint;

// книга задается рядом, шкафом и полкой
struct Book {
    int i, j, k;

    Book(int i, int j, int k) : i(i), j(j), k(k) {
    }
};

// класс библиотеки университета
// M - число рядов, N - число шкафов в ряду, K - число книг в шкафу
// bookcases - булевый трех-мерный массив, 
// где bookcases[i][j][k] - есть ли книга в (i,j,k)-ой ячейке
// lostBooks - стек нехватающих книг
class Library {
public:
    int M, N, K;
    bool ***bookcases;
    stack<Book> lostBooks;

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

    // проверка нехватающих книг
    void checkLostBooks() {
        while (!lostBooks.empty()) {
            lostBooks.pop();
        }
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                for (int k = 0; k < K; k++) {
                    if (!bookcases[i][j][k])
                        lostBooks.push(Book(i, j, k));
                }
            }
        }
    }
};

// класс студента
// содержит единственную функция - вечный цикл
class Student {
public:
    int id;

    Student(int id) : id(id) {
    }

    // пока есть недостающие книги в библиотеке - поставлять
    void supplyBooks(Library &lib) {
        int waitBefore = uniform_int_distribution<int>(0, 1000)(rng);
        this_thread::sleep_for(chrono::milliseconds(waitBefore));

        while (true) {

            // узнаю какой книги не хватает
            mutGet.lock();
            // если таких нет
            if (lib.lostBooks.empty()) {
                mutGet.unlock();
                mutPrint.lock();
                cout << "Студент:" << id << ". Свободен\n";
                mutPrint.unlock();
                break;
            }
            Book b = lib.lostBooks.top();
            lib.lostBooks.pop();
            mutGet.unlock();

            mutPrint.lock();
            cout << "Студент:" << id << ". Иду покупать книгу (" << b.i << "," << b.j << "," << b.k << ")\n";
            mutPrint.unlock();

            // покупаю эту книгу
            this_thread::sleep_for(chrono::milliseconds(1000));

            // отдаю в библиотеку (можно отдавать и асинхронно)
//            mutSet.lock();
            lib.bookcases[b.i][b.j][b.k] = true;
//            mutSet.unlock();

            mutPrint.lock();
            cout << "Студент:" << id << ". Отдаю книгу в библиотеку (" << b.i << "," << b.j << "," << b.k << ")\n";
            mutPrint.unlock();


        }
    }
};



int main() {
    system("chcp 65001");

    cout << "start\n";

    int M, N, K, threadsCnt;
    cout << "Введите кол-во рядов, шкафов в ряду, книг в шкафу и число студентов\n";
    cin >> M >> N >> K >> threadsCnt;

    Library lib(M, N, K);
    // заполнить рандомно ячейки true или false
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < K; k++) {
                lib.bookcases[i][j][k] = uniform_int_distribution<int>(0, 2)(rng);
                // посмотреть есть ли (i,j,k)-ая книга
//                cout << i << "," << j << "," << k << "=" << lib.bookcases[i][j][k] << "\n";
            }
        }
    }

    // проверка на нехватку книг
    lib.checkLostBooks();

    // создаю потоки, к каждому прявязываю одного студента
    thread students[threadsCnt];
    for (int i = 0; i < threadsCnt; i++) {
        students[i] = thread(&Student::supplyBooks, Student(i + 1), ref(lib));
    }

    // жду окончанию работы потоков
    for (int i = 0; i < threadsCnt; i++) {
        if (students[i].joinable())
            students[i].join();
    }



    cout << "finish\n";
    return 0;
}
