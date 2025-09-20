#include <stdio.h>

int main(){
    FILE *fptr;
    fptr = fopen("cacapalavras.txt", "r");

    if(fptr == NULL){
        printf("Error opening file!\n");
        return 1;
    }

    return 0;
}