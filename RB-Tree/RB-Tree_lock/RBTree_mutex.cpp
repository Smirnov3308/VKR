// g++ RBTree_mutex.cpp -O2 -Wall -std=c++11 -lpthread
#include <thread>
#include <mutex>
#include <stdio.h>
#include <sys/time.h>

#define SIZE 4000000
// #define NUM_OF_THREADS 4
int NUM_OF_THREADS = 4;
typedef enum { insertNode, removeNode, lookup} operation; // Операции
operation Operations[SIZE];                 // Массив операций
typedef int T; // Тип данных, хранящихся в дереве
int TEST;
typedef enum { BLACK, RED } nodeColor;

typedef struct Node_ {
    struct Node_ *left;         // Указатель на левый дочерний узел
    struct Node_ *right;        // Указатель на правый дочерний узел
    struct Node_ *parent;       // Указатель на родительский узел
    nodeColor color;            // Цвет узла
    T data;                     // Данные, хранящиеся в узле
} Node;

#define NIL &sentinel
Node sentinel = { NIL, NIL, 0, BLACK, 0};

class RBTree{
    Node *root; // Корень дерева
    mutable std::mutex m; // Мьютекс
public:
    RBTree() {root=NIL;}

    void rotateLeft(Node *x) {
        Node *y = x->right;
        x->right = y->left;
        if (y->left != NIL) y->left->parent = x;

        if (y != NIL) y->parent = x->parent;
        if (x->parent) {
            if (x == x->parent->left)
                x->parent->left = y;
            else
                x->parent->right = y;
        } else {
            root = y;
        }

        y->left = x;
        if (x != NIL) x->parent = y;
    }

    void rotateRight(Node *x) {
        Node *y = x->left;
        x->left = y->right;
        if (y->right != NIL) y->right->parent = x;

        if (y != NIL) y->parent = x->parent;
        if (x->parent) {
            if (x == x->parent->right)
                x->parent->right = y;
            else
                x->parent->left = y;
        } else {
            root = y;
        }
        y->right = x;
        if (x != NIL) x->parent = y;
    }

    void insertBalance(Node *x) {
        // Проверка свойств дерева
        while (x != root && x->parent->color == RED) {
            if (x->parent == x->parent->parent->left) {
                Node *y = x->parent->parent->right;
                if (y->color == RED) {

                    // Дядя красный
                    x->parent->color = BLACK;
                    y->color = BLACK;
                    x->parent->parent->color = RED;
                    x = x->parent->parent;
                } else {

                    // Дядя чёрный
                    if (x == x->parent->right) {
                        x = x->parent;
                        rotateLeft(x);
                    }
                    // Перекрасить и повернуть
                    x->parent->color = BLACK;
                    x->parent->parent->color = RED;
                    rotateRight(x->parent->parent);
                }
            } else {
                Node *y = x->parent->parent->left;
                if (y->color == RED) {

                    // Дядя красный
                    x->parent->color = BLACK;
                    y->color = BLACK;
                    x->parent->parent->color = RED;
                    x = x->parent->parent;
                } else {

                    // Дядя чёрный
                    if (x == x->parent->left) {
                        x = x->parent;
                        rotateRight(x);
                    }
                    // Перекрасить и повернуть
                    x->parent->color = BLACK;
                    x->parent->parent->color = RED;
                    rotateLeft(x->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }

    bool insertNode(T data) {
        Node *current, *parent;
        Node *x = new Node;
        x->data = data;
        x->left = NIL;
        x->right = NIL;
        x->color = RED;

        m.lock(); // Захват мьютекса
        // Поиск места для нового узла
        current = root;
        parent = 0;
        while (current != NIL) {
            if (data == current->data) {
                m.unlock(); // Освобождение мьютекса
                delete x;
                return 0;
            }
            parent = current;
            if (data < current->data) 
                current = current->left;
            else 
                current = current->right;
        }
        x->parent = parent;

        // Добавление узла
        if(parent) {
            if(data < parent->data)
                parent->left = x;
            else
                parent->right = x;
        } else {                    // В дереве нет узлов, добавление на место корня
            root = x;
        }
        
        //Проверка и восстановление свойств
        insertBalance(x);
        m.unlock(); // Освобождение мьютекса
        return 1;
    }

    void removeBalance(Node *x) {

        while (x != root && x->color == BLACK) {
            if (x == x->parent->left) {
                Node *w = x->parent->right;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rotateLeft (x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->right->color == BLACK) {
                        w->left->color = BLACK;
                        w->color = RED;
                        rotateRight (w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    rotateLeft (x->parent);
                    x = root;
                }
            } else {
                Node *w = x->parent->left;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rotateRight (x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->left->color == BLACK) {
                        w->right->color = BLACK;
                        w->color = RED;
                        rotateLeft (w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    rotateRight (x->parent);
                    x = root;
                }
            }
        }
        x->color = BLACK;
    }

    void removeNode(T data) {
        m.lock();
        Node *z, *x, *y, *current;
        current = root;
        z = 0;
        while(current != NIL) {
            if(data == current->data) {
                z = current;
                break;
            } else if (data < current->data) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        
        if (z == 0) {
            m.unlock();
            return;
        }
        if (z->left == NIL || z->right == NIL) {
            // y имеет листового потомка
            y = z;
        } else {
            // поиск узла, имеющего листового потомка
            y = z->right;
            while (y->left != NIL) y = y->left;
        }

        if (y->left != NIL)
            x = y->left;
        else
            x = y->right;

        x->parent = y->parent;
        if (y->parent)
            if (y == y->parent->left)
                y->parent->left = x;
            else
                y->parent->right = x;
        else
            root = x;

        if (y != z) z->data = y->data;

        if (y->color == BLACK)
            removeBalance (x);
        delete y;
       m.unlock();
    }

    Node *lookup(T data) {
        m.lock();
        Node *current = root;
        while(current != NIL)
            if(data == current->data){
                m.unlock();
                return (current);
            }
            else
                if (data < current->data) 
                    current = current->left;
                else 
                    current = current->right;
        m.unlock();
        return(0);
    }

    void printTree(){
        printf("Дерево:\n");
        printTree(root, 0);
    }

    void printTree(Node *current, int n) {

        char ch;
        if(current!=NIL){
            printTree(current->right, n+5);
            for (int i = 0; i < n; i++) 
                printf(" ");
            if(current->color) ch = 'R';
            else ch = 'B';
            printf("%c%d\n", ch, current->data);
            printTree(current->left, n+5);
        }
    }
};

// Таймер
long long mtime()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  long long mt = (long long)t.tv_sec * 1000 + t.tv_usec / 1000;
  return mt;
}


// Код, выполняемый каждым потоком
void ThrFunc(RBTree &Tree, int num) {
    int portion = SIZE / NUM_OF_THREADS;
    unsigned int seed = 345634*num;
    for(int i = num*portion; i<(num*portion+portion); ++i) {
        switch (Operations[i]) {
            case lookup:
                Tree.lookup(rand_r(&seed) % SIZE);
                break;
            case insertNode:
                Tree.insertNode(rand_r(&seed) % SIZE);
                break;
            case removeNode:
                Tree.removeNode(rand_r(&seed) % SIZE);
                break;
        }
    }
}

void ThrFuncLookup(RBTree &Tree, int num) {
    int portion = SIZE / NUM_OF_THREADS;
    unsigned int seed = 345634*num;
    for(int i = num*portion; i<(num*portion+portion); ++i) {
        Tree.lookup(rand_r(&seed) % SIZE);
    }
}
void ThrFuncRemove(RBTree &Tree, int num) {
    int portion = SIZE / NUM_OF_THREADS;
    unsigned int seed = 345634*num;
    for(int i = num*portion; i<(num*portion+portion); ++i) {
        Tree.removeNode(rand_r(&seed) % SIZE);
    }
}
void ThrFuncAdd(RBTree &Tree, int num) {
    int portion = SIZE / NUM_OF_THREADS;
    unsigned int seed = 345634*num;
    for(int i = num*portion; i<(num*portion+portion); ++i) {
        Tree.insertNode(rand_r(&seed) % SIZE);
    }
}



int main(int argc, char **argv) {
    NUM_OF_THREADS = atoi(argv[1]);
    RBTree Tree;
    TEST = atoi(argv[2]);
    int temp_op;

    if (TEST == 0) {
        for(int i = 0; i < SIZE; ++i){
            temp_op = rand() % 100;
            if (temp_op < 40 ) {
                Operations[i] = lookup;
            } else if (temp_op > 70) {
                Operations[i] = insertNode;
            } else {
                Operations[i] = removeNode;
            }
        }
        for(int i = 0; i < SIZE/2; ++i){
            Tree.insertNode(rand() % SIZE);
        }
    } else if ((TEST == 1)||(TEST == 2)) {
        for(int i = 0; i < SIZE; ++i){
            Tree.insertNode(rand() % SIZE);
        }
    }

    std::thread thr[NUM_OF_THREADS];
    
    // Запуск таймера
    long long time = mtime();

    for(int i = 0; i<NUM_OF_THREADS; ++i){
        switch (TEST) {
            case 0:
                thr[i] = std::thread(ThrFunc, std::ref(Tree), i);
                break;
            case 1:
                thr[i] = std::thread(ThrFuncLookup, std::ref(Tree), i);
                break;
            case 2:
                thr[i] = std::thread(ThrFuncRemove, std::ref(Tree), i);
                break;
            case 3:
                thr[i] = std::thread(ThrFuncAdd, std::ref(Tree), i);
                break;
        }
    }
    for(int i = 0; i<NUM_OF_THREADS; ++i){
        thr[i].join();
    }

    time = mtime() - time;
    printf("%llu\n", time);
    return 0;
}
