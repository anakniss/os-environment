#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

typedef struct {
    int start_x, start_y;
    char direction[20];
} Occurrence;

typedef struct {
    Occurrence *occurrences;
    int occ_count;
    int capacity;
} FoundWord;

typedef struct {
    char **matrix;
    int line_size;
    int column_size;
    char *word;
    FoundWord *found;
} ThreadData;

char** readMatrix(FILE* file, int line_size, int column_size){
    char **matrix = malloc(line_size * sizeof(char *));
    if (!matrix) return NULL;

    for (int i = 0; i < line_size; i++) {
        matrix[i] = malloc((column_size + 1) * sizeof(char));
        fscanf(file, "%s", matrix[i]);
    }
    return matrix;
}

char** readWords(FILE* file, int* word_count) {
    int capacity = 10;
    int count = 0;
    char buffer[256];
    char **words = malloc(capacity * sizeof(char*));
    if (!words) return NULL;

    while (fscanf(file, "%255s", buffer) == 1) {
        if (count >= capacity) {
            capacity *= 2;
            words = realloc(words, capacity * sizeof(char*));
        }
        words[count] = malloc((strlen(buffer) + 1) * sizeof(char));
        strcpy(words[count], buffer);
        count++;
    }

    *word_count = count;
    return words;
}

int inBounds(int x, int y, int line_size, int column_size) {
    return (x >= 0 && x < line_size && y >= 0 && y < column_size);
}

const char* directionName(int dx, int dy) {
    if (dx == 0 && dy == 1) return "direita";
    if (dx == 0 && dy == -1) return "esquerda";
    if (dx == 1 && dy == 0) return "baixo";
    if (dx == -1 && dy == 0) return "cima";
    if (dx == 1 && dy == 1) return "baixo/direita";
    if (dx == -1 && dy == -1) return "cima/esquerda";
    if (dx == 1 && dy == -1) return "baixo/esquerda";
    if (dx == -1 && dy == 1) return "cima/direita";
    return "desconhecido";
}

void highlightWordStore(ThreadData *data, int start_x, int start_y, int dx, int dy, const char *word, FoundWord *found) {
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
        int x = start_x + i * dx;
        int y = start_y + i * dy;
        if (!inBounds(x, y, data->line_size, data->column_size)) return;
        if (tolower(data->matrix[x][y]) != tolower(word[i])) return;
    }

    // se chegou aqui, palavra encontrada > salva ocorrência
    if (found->occ_count >= found->capacity) {
        found->capacity *= 2;
        found->occurrences = realloc(found->occurrences, found->capacity * sizeof(Occurrence));
    }

    Occurrence occ;
    occ.start_x = start_x;
    occ.start_y = start_y;
    strcpy(occ.direction, directionName(dx, dy));

    found->occurrences[found->occ_count++] = occ;

    // coloca em maiúsculo na matriz
    for (int i = 0; i < len; i++) {
        int x = start_x + i * dx;
        int y = start_y + i * dy;
        data->matrix[x][y] = toupper(data->matrix[x][y]);
    }
}

void *thread_find_word(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    const char *word = data->word;
    FoundWord *found = data->found;

    int dx[] = {0, 0, 1, -1, 1, -1, 1, -1};
    int dy[] = {1, -1, 0, 0, 1, -1, -1, 1};

    for (int i = 0; i < data->line_size; i++) {
        for (int j = 0; j < data->column_size; j++) {
            for (int dir = 0; dir < 8; dir++) {
                highlightWordStore(data, i, j, dx[dir], dy[dir], word, found);
            }
        }
    }
    return NULL;
}

int saveResultsToFile(const char *filename, char **matrix, int line_size, char **words, int word_count, FoundWord *allFound) {
    FILE *out = fopen(filename, "w");
    if (!out) return 0;

    // salva a matriz
    for (int i = 0; i < line_size; i++) {
        fprintf(out, "%s\n", matrix[i]);
    }

    // salva as ocorrências
    fprintf(out, "\n");
    for (int w = 0; w < word_count; w++) {
        if (allFound[w].occ_count == 0) {
            fprintf(out, "%s: não encontrada\n", words[w]);
        } else {
            for (int o = 0; o < allFound[w].occ_count; o++) {
                fprintf(out, "%s – (%d,%d):%s\n",
                    words[w],
                    allFound[w].occurrences[o].start_x + 1, // +1 p/ humano
                    allFound[w].occurrences[o].start_y + 1,
                    allFound[w].occurrences[o].direction);
            }
        }
    }

    fclose(out);
    return 1;
}

int main(){
    FILE *file;
    int line_size = 0, column_size = 0, word_count = 0;

    file = fopen("./cacapalavras.txt", "r");
    if(file == NULL){
        printf("Error opening file!\n");
        return 1;
    }

    fscanf(file, "%d %d", &line_size, &column_size);

    char **matrix = readMatrix(file, line_size, column_size);
    char **words = readWords(file, &word_count);
    fclose(file);

    if (!words || word_count == 0) {
        printf("Nenhuma palavra encontrada.\n");
        return 1;
    }

    pthread_t *threads = malloc(word_count * sizeof(pthread_t));
    ThreadData *threadData = malloc(word_count * sizeof(ThreadData));
    FoundWord *allFound = malloc(word_count * sizeof(FoundWord));

    for (int w = 0; w < word_count; w++) {
        allFound[w].occurrences = malloc(10 * sizeof(Occurrence));
        allFound[w].occ_count = 0;
        allFound[w].capacity = 10;

        threadData[w].matrix = matrix;
        threadData[w].line_size = line_size;
        threadData[w].column_size = column_size;
        threadData[w].word = words[w];
        threadData[w].found = &allFound[w];

        pthread_create(&threads[w], NULL, thread_find_word, &threadData[w]);
    }

    for (int w = 0; w < word_count; w++) {
        pthread_join(threads[w], NULL);
    }

    if (saveResultsToFile("result_final2.txt", matrix, line_size, words, word_count, allFound)) {
        printf("Resultado salvo em result_final.txt!\n");
    }

    // libera memória
    for (int i = 0; i < line_size; i++) free(matrix[i]);
    free(matrix);

    for (int i = 0; i < word_count; i++) free(words[i]);
    free(words);

    for (int w = 0; w < word_count; w++) free(allFound[w].occurrences);
    free(allFound);
    free(threadData);
    free(threads);

    return 0;
}
