#include <stdio.h>
#include <stdlib.h>

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


int main(){
    FILE *file;
    int line_size = 0;
    int column_size = 0;
    char word_to_find[] = "deadlock";

    file = fopen("./teste.txt", "r");
    
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
    char **matriz = readMatrix(file, line_size, column_size);
    if (!matriz) {
        fclose(file);
        return 1;
    }

    printf("Número de linhas: %d\n", line_size);
    printf("Número de colunas: %d\n", column_size);

    // imprime parte da matriz (teste)
    for (int i = 0; i < 5; i++) {
        printf("%s\n", matriz[i]);
    }

    // libera memória
    for (int i = 0; i < line_size; i++) {
        free(matriz[i]);
    }
    free(matriz);

    fclose(file);

    return 0;
}