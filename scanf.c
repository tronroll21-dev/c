#include <stdio.h>

typedef struct {
    int a;
    int b;
} Data;

void printData(Data* d);
void printDataArray(Data* d, int size);

int main(int argc, char* argv[]) {

    if(argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    Data d;

    FILE* f = fopen(argv[1], "r");

    if(!f){
        fprintf(stderr, "Could not open file: %s\n", argv[1]);
        return 1;
    }

    int read = 0;

    read = fscanf(f, "%d\t%d", &d.a, &d.b);
    printData(&d);    
    
    read = fscanf(f, "%d\t%d", &d.a, &d.b);
    printData(&d);
    
    printf("Data: a=%d, b=%d\n", d.a, d.b);
    
    fclose(f);    

    Data data[2] = {d, {++d.a, ++d.b}};

    printDataArray(data, 2);

    return 0;
}

void printData(Data* d) {
    printf("Data: a=%d, b=%d\n", d->a, d->b);
    
    d->a += 1;
    d->b += 1;

}

void printDataArray(Data* d, int size){
    for(int i = 0; i < size; i++) {
        printf("Data[%d]: a=%d, b=%d\n", i, d[i].a, d[i].b);
    }
}