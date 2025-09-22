#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** readMatrix(FILE* file, int line_size, int column_size){
    char **matrix = malloc(line_size * sizeof(char *));
    if (!matrix) {
        printf("Erro ao alocar memória para matriz.\n");
        return NULL;
    }

    for (int i = 0; i < line_size; i++) {
        matrix[i] = malloc((column_size + 1) * sizeof(char)); // +1 para '\0'
        if (!matrix[i]) {
            printf("Erro ao alocar memória para linha %d.\n", i);

            // libera o que já foi alocado
            for (int j = 0; j < i; j++) free(matrix[j]);
            free(matrix);
            return NULL;
        }

        if (fscanf(file, "%s", matrix[i]) != 1) {
            printf("Erro ao ler linha %d da matriz.\n", i);

            // libera o que já foi alocado
            for (int j = 0; j <= i; j++) free(matrix[j]);
            free(matrix);
            return NULL;
        }
    }

    return matrix;
}

int saveMatrixToFile(const char **matrix, int line_size, const char *filename) {
    FILE *out = fopen(filename, "w");
    if (!out) {
        printf("Erro ao criar arquivo de saída: %s\n", filename);
        return 0; // falha
    }

    for (int i = 0; i < line_size; i++) {
        fprintf(out, "%s\n", matrix[i]);
    }

    fclose(out);
    return 1; // sucesso
}

int main(){
    FILE *file;
    int line_size = 0;
    int column_size = 0;

    file = fopen("./cacapalavras.txt", "r");
    
    if(file == NULL){
        printf("Error opening file!\n");
        return 1;
    }

    // lê dimensões
    while(fscanf(file, "%d %d", &line_size, &column_size) != 2){
        printf("Error on reading dimensions\n");
        fclose(file);
        return 1;
    }

    // lê matriz
    char **matrix = readMatrix(file, line_size, column_size);
    if (!matrix) {
        fclose(file);
        return 1;
    }

    printf("Número de linhas: %d\n", line_size);
    printf("Número de colunas: %d\n", column_size);

    // salva matriz em arquivo diferente
    if (saveMatrixToFile((const char**)matrix, line_size, "result.txt")) {
        printf("Matriz gravada em result.txt com sucesso!\n");
    } else {
        printf("Falha ao salvar matriz em arquivo.\n");
    }

    // imprime matriz
    for (int i = 0; i < line_size; i++) {
        printf("%s\n", matrix[i]);
    }

    // libera memória
    for (int i = 0; i < line_size; i++) {
        free(matrix[i]);
    }
    free(matrix);

    fclose(file);

    return 0;
}