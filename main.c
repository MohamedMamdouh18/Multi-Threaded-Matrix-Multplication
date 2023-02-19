#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

typedef struct matrix {
    char *name;
    int row;
    int col;
    int **data;
} matrix;

typedef struct multArg {
    int firstRow;
    int secondCol;
} multArg;

matrix mat[3];
char *names[3] = {"a", "b", "c"};

void readData(int argc, char *argv[], int no);

void matrixReader(int *x, FILE *file);

void prepareResultMat(int argc, char *argv[]);

void allocateData(int no);

void saveResultAndReset(int mode);

void firstMethod();

void *matrixOneThread();

void secondMethod();

void *matrixRowThread(void *arg);

void thirdMethod();

void *matrixElemThread(void *arg);

void printTerminal(int method, int threads, unsigned long millie, unsigned long micro);

int main(int argc, char *argv[]) {
    readData(argc, argv, 0);
    readData(argc, argv, 1);
    if (mat[0].col != mat[1].row) {
        printf("Please enter right data");
        exit(0);
    }
    prepareResultMat(argc, argv);

    firstMethod();
    saveResultAndReset(1);

    secondMethod();
    saveResultAndReset(2);

    thirdMethod();
    saveResultAndReset(3);

    return 0;
}

/***
 * read elements of matrix
 * @param x matrix element to save in
 * @param file to read from
 */
void matrixReader(int *x, FILE *file) {
    char *c = malloc(sizeof(char) * 20);
    fscanf(file, "%s", c);
    *x = (int) strtol(c, NULL, 10);
    free(c);
}

/***
 * read data from txt files
 * @param argc number of argument that user entered
 * @param argv the arguments
 * @param no number of matrix to read
 */
void readData(int argc, char *argv[], int no) {
    if (argc == 4) mat[no].name = argv[no + 1];
    else mat[no].name = names[no];

    char *fileName = malloc(sizeof(char) * 20);
    strcpy(fileName, mat[no].name);
    FILE *file = fopen(strcat(fileName, ".txt"), "r");

    if (file == NULL) {
        printf("There is no such file");
        exit(0);
    }

    fscanf(file, "row=%i col=%i", &mat[no].row, &mat[no].col);
    allocateData(no);

    for (int i = 0; i < mat[no].row; ++i) {
        for (int j = 0; j < mat[no].col; ++j) {
            matrixReader(&mat[no].data[i][j], file);
        }
    }

    free(fileName);
    fclose(file);
}

/***
 * allocate memory for our structs
 * @param no number of matrix to allocate for
 */
void allocateData(int no) {
    //allocate array of pointers first
    mat[no].data = (int **) malloc(sizeof(int *) * mat[no].row);

    //allocate every element independent
    for (int i = 0; i < mat[no].row; ++i) {
        mat[no].data[i] = (int *) malloc(sizeof(int) * mat[no].col);
    }

    for (int i = 0; i < mat[no].row; ++i) {
        for (int j = 0; j < mat[no].col; ++j) {
            mat[no].data[i][j] = 0;
        }
    }
}

/***
 * prepare struct that will store result
 * @param argc
 * @param argv
 */
void prepareResultMat(int argc, char *argv[]) {
    mat[2].row = mat[0].row;
    mat[2].col = mat[1].col;

    if (argc == 4) mat[2].name = argv[2 + 1];
    else mat[2].name = names[2];
    allocateData(2);
}

/***
 * store result of the method and reset struct data to zeros
 * @param mode
 */
void saveResultAndReset(int mode) {
    FILE *file;
    char *fileName = malloc(sizeof(char) * 20);
    strcpy(fileName, mat[2].name);
    if (mode == 1) {
        file = fopen(strcat(fileName, "_per_matrix.txt"), "w");
        fprintf(file, "%s\n", "Method: A thread per matrix");
    } else if (mode == 2) {
        file = fopen(strcat(fileName, "_per_row.txt"), "w");
        fprintf(file, "%s\n", "Method: A thread per row");
    } else {
        file = fopen(strcat(fileName, "_per_element.txt"), "w");
        fprintf(file, "%s\n", "Method: A thread per element");
    }

    fprintf(file, "row=%i col=%i\n", mat[2].row, mat[2].col);

    for (int i = 0; i < mat[2].row; ++i) {
        for (int j = 0; j < mat[2].col; ++j) {
            fprintf(file, "%i ", mat[2].data[i][j]);
        }
        fprintf(file, "\n");
    }

    for (int i = 0; i < mat[2].row; ++i) {
        for (int j = 0; j < mat[2].col; ++j) {
            mat[2].data[i][j] = 0;
        }
    }

    fclose(file);
    free(fileName);
}

void firstMethod() {
    struct timeval stop, start;
    gettimeofday(&start, NULL); //start checking time

    pthread_t t1;
    int status = pthread_create(&t1, NULL, matrixOneThread, NULL);
    if (status) {
        printf("Error creating a thread");
        exit(0);
    }

    pthread_join(t1, NULL);
    gettimeofday(&stop, NULL); //end checking time

    printTerminal(1, 1,
                  stop.tv_sec - start.tv_sec, stop.tv_usec - start.tv_usec);
}

void *matrixOneThread() {
    for (int i = 0; i < mat[0].row; i++) {
        for (int j = 0; j < mat[1].col; j++) {
            for (int k = 0; k < mat[0].col; k++) {
                mat[2].data[i][j] += mat[0].data[i][k] * mat[1].data[k][j];
            }
        }
    }
    pthread_exit(NULL);
}

void secondMethod() {
    int threadNum = mat[0].row, status[threadNum];
    pthread_t threads[threadNum];

    struct timeval stop, start;
    gettimeofday(&start, NULL); //start checking time

    for (int i = 0; i < threadNum; ++i) {
        int *row = malloc(sizeof(int));
        *row = i;
        status[i] = pthread_create(&threads[i], NULL, matrixRowThread, (void *) row);
        if (status[i]) {
            printf("Error creating a thread");
            exit(0);
        }
    }

    for (int i = 0; i < threadNum; ++i) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&stop, NULL); //end checking time

    printTerminal(2, threadNum,
                  stop.tv_sec - start.tv_sec, stop.tv_usec - start.tv_usec);
}

void *matrixRowThread(void *arg) {
    int row = *(int *) arg;
    for (int i = 0; i < mat[1].row; ++i) {
        for (int j = 0; j < mat[1].col; ++j) {
            mat[2].data[row][j] += mat[0].data[row][i] * mat[1].data[i][j];
        }
    }
    free(arg);
    pthread_exit(NULL);
}

void thirdMethod() {
    int threadNum = mat[0].row * mat[1].col, status[threadNum], num = 0;
    pthread_t threads[threadNum];

    struct timeval stop, start;
    gettimeofday(&start, NULL); //start checking time

    for (int i = 0; i < mat[0].row; ++i) {
        for (int j = 0; j < mat[1].col; j++) {
            multArg *data = malloc(sizeof(multArg));
            data->firstRow = i;
            data->secondCol = j;
            status[i] = pthread_create(&threads[num++], NULL, matrixElemThread, (void *) data);
            if (status[i]) {
                printf("Error creating a thread");
                exit(0);
            }
        }
    }

    for (int i = 0; i < threadNum; ++i) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&stop, NULL); //end checking time

    printTerminal(3, threadNum,
                  stop.tv_sec - start.tv_sec, stop.tv_usec - start.tv_usec);
}

void *matrixElemThread(void *arg) {
    multArg *data = (multArg *) arg;
    for (int i = 0; i < mat[0].col; ++i) {
        mat[2].data[data->firstRow][data->secondCol] += mat[0].data[data->firstRow][i]
                                                        * mat[1].data[i][data->secondCol];
    }
    free(data);
    pthread_exit(NULL);
}

void printTerminal(int method, int threads, unsigned long millie, unsigned long micro) {
    if (method == 1)printf("\nPer Matrix Method\n");
    else if (method == 2) printf("\nPer Row Method\n");
    else printf("\nPer Element Method\n");

    printf("Number of Threads = %i\n", threads);
    printf("Seconds taken %lu\n", millie);
    printf("Microseconds taken: %lu\n", micro);
}
