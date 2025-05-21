#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MIN 1000
#define MAX 100000

pthread_mutex_t mutex;

typedef struct NodeList {
    int val;
    struct NodeList* next;
    struct NodeList* prev;
} NodeList;

typedef struct List {
    NodeList* begin_list;
    NodeList* end_list;
    int size;
} List;

typedef struct ArgProcFunc {
    int option; // 1 - единицы с конца, 0 - нули с головы 
    int count_bits;//кол-во битов
    int pass_elements;//кол-во обработанных эл-в
    List* list;
} ArgProcFunc;

int sumBits(const int* num, const int* option) {
    int count_bit = 0;
    for (int i = 0; i < sizeof(int) * 8; i++) {
        if (((*num >> i) & 1) == *option) {
            ++count_bit;
        }
		
    }
    return count_bit;
}


void* threadFunc(void* arg_func) {
    ArgProcFunc* arg_procfunc = (ArgProcFunc*)arg_func;
    List* list = arg_procfunc->list;
    int list_empty = 0;
    while (list->size) {
        pthread_mutex_lock(&mutex);
        NodeList* current_node = NULL;

        if (list->size > 1) {//выбор эл-та
            if ((arg_procfunc->option) == 1) { // с конца
			
                current_node = list->end_list;
                list->end_list = list->end_list->prev;
                list->end_list->next = NULL;
            }
            else { // с головы
                current_node = arg_procfunc->list->begin_list;
                list->begin_list = list->begin_list->next;
                list->begin_list->prev = NULL;
            }
            --(arg_procfunc->list->size);
        }

        else {
            current_node = arg_procfunc->list->begin_list;
            list->begin_list = NULL;
            list->end_list = NULL;
            list->size = 0;
        }
		
        pthread_mutex_unlock(&mutex);

        if (current_node) {//подсчет битов
            arg_procfunc->count_bits += sumBits(&(current_node->val), &(arg_procfunc->option));
            arg_procfunc->pass_elements++;
            free(current_node);
        }
    }
    return NULL;
}

void printList(const List* list) {
    NodeList* current_node = list->begin_list;
    while (current_node) {
		
        printf("%d\t", current_node->val);
        current_node = current_node->next;
    }
	
    printf("\n");
}

List* createList(int n) {
    List* list = (List*)malloc(sizeof(List));
    list->begin_list = NULL;
    list->end_list = NULL;
    list->size = 0;
	
    for (int i = 0; i < n; ++i) {
        NodeList* current_node = (NodeList*)malloc(sizeof(NodeList));
        current_node->val = rand() % (MAX - MIN + 1) + MIN;
        current_node->next = NULL;
        current_node->prev = list->end_list;
        if (list->end_list) {
            list->end_list->next = current_node;
        }
        else {
			
            list->begin_list = current_node;
        }
        list->end_list = current_node;
        ++(list->size);
    }
    return list;
}

void intitalArgProcFunc(ArgProcFunc* arg_proc_func, List* list, int option, int count_bits, int pass_elements) {
    arg_proc_func->list = list;
    arg_proc_func->option = option;
    arg_proc_func->count_bits = count_bits;
    arg_proc_func->pass_elements = pass_elements;
}
int main() {
	
    pthread_mutex_init(&mutex, NULL);
	
	
    int n;
    printf("Vvod kolichestva elementov: ");
    scanf_s("%d", &n);
    srand(time(NULL));
    List* list = createList(n);
    //printList(list);

    ArgProcFunc* arg_procfunc_first = (ArgProcFunc*)malloc(sizeof(ArgProcFunc));
    intitalArgProcFunc(arg_procfunc_first, list, 0, 0, 0);// option = 0;с головы, считаем нули

    ArgProcFunc* arg_procfunc_second = (ArgProcFunc*)malloc(sizeof(ArgProcFunc));
    intitalArgProcFunc(arg_procfunc_second, list, 1, 0, 0);// option = 1; с конца, считаем единицы

    pthread_t first_zero, second_one;
    pthread_create(&first_zero, NULL, threadFunc, arg_procfunc_first);
    pthread_create(&second_one, NULL, threadFunc, arg_procfunc_second);

    pthread_join(first_zero, NULL);
    pthread_join(second_one, NULL);
    printf("Potok 1 (c golovi):\n  elementov: %d\n  0 bit: %d\n", arg_procfunc_first->pass_elements, arg_procfunc_first->count_bits);
    printf("Potok 2 (c hvosta):\n  eleventov: %d\n  1 bit: %d\n", arg_procfunc_second->pass_elements, arg_procfunc_second->count_bits);
    free(arg_procfunc_first);
    free(arg_procfunc_second);
    free(list);
    pthread_mutex_destroy(&mutex);
    return 0;
}



