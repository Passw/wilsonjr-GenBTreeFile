#include "bplus.h"

#define MAX 2000000

/**
*
*  int functions_comp[](const void*, const void*):
*		< 0: primeiro menor
*		> 0: primeiro maior
*		= 0: iguais
*	 --> functions_comp[0](key, key) *
*	 --> functions_comp[1](record, record) *
*    --> functions_comp[2](record, key) *
*
*  void functions_atrib[](void*, void*, size_t, size_t):
*	 --> functions_atrib[0](key, key, size_t, size_t) *
*    --> functions_atrib[1](record, record, size_t, size_t) *
*    --> functions_atrib[2](key, record, size_t, size_t) *
*
*/

void atrib_key_record(void* key, void* record, size_t n_key, size_t n_record)
{
    char* k = (char*) key;
    struct elementos* elem =  (struct elementos*) record;
    void* inteiro  =  &elem->meuInteiro;
    char* new_key = (char*) inteiro;

    for (int i = 0; i < n_key; ++i)
        k[i] = new_key[i];
}

void atrib_record_record(void* record_1, void* record_2, size_t n_record_1, size_t n_record_2)
{
    char* bits_record_1 = (char*) record_1;
    char* bits_record_2 = (char*) record_2;

    for (unsigned int i = 0; i < n_record_1 && i < n_record_2; ++i) {
        char a = bits_record_2[i];
        bits_record_1[i] = a;
    }

}

void atrib_key_key(void* key_1, void* key_2, size_t n_key_1, size_t n_key_2)
{
    char* bits_key_1 = (char*) key_1;
    char* bits_key_2 = (char*) key_2;

    for (unsigned int i = 0; i < n_key_1 && i < n_key_2; ++i)
        bits_key_1[i] = bits_key_2[i];
}

//-----------------------------------------------------------------------------------
int comp_record_key(const void* record, const void* key)
{
    struct elementos* rec = (struct elementos*) record;
    int* k = (int*) key;

    return rec->meuInteiro - *k;
}

int comp_record_record(const void* record_1, const void* record_2)
{
    struct elementos* rec1 = (struct elementos*) record_1;
    struct elementos* rec2 = (struct elementos*) record_2;

    return rec1->meuInteiro - rec2->meuInteiro;
}

int comp_key_key(const void* elem_1, const void* elem_2)
{
    int* e1 = (int*) elem_1;
    int* e2 = (int*) elem_2;

    return *e1 - *e2;
}

void randomize() {
    time_t seed = time(NULL);
    srand(seed);
}


int main()
{

    void (*funcs_atrib[3])(void*, void*, size_t, size_t);
    int (*funcs_comp[3])(const void*, const void*);

    funcs_atrib[0] = atrib_key_key;
    funcs_atrib[1] = atrib_record_record;
    funcs_atrib[2] = atrib_key_record;
    funcs_comp[0] = comp_key_key;
    funcs_comp[1] = comp_record_record;
    funcs_comp[2] = comp_record_key;

    FILE* elementos_stream;
    struct elementos* elemento;
    int chave;
    int opcao;

    struct __bplusnode__ root1;
    struct __bplusnode__ root2;
    struct __bplusnode__ root3;
    struct __bplusnode__ root4;


    char nome_arquivo1[100];
    char nome_arquivo2[100];
    char nome_arquivo3[100];
    char nome_arquivo4[100];

    printf("\t\tInforme o nome do arquivo para arvore 1 (.dat): ");
    gets(nome_arquivo1);
    init_btree(&root1, nome_arquivo1);
    printf("\t\tInforme o nome do arquivo para arvore 2 (.dat): ");
    gets(nome_arquivo2);
    init_btree(&root2, nome_arquivo2);
    printf("\t\tInforme o nome do arquivo para arvore 3 (.dat): ");
    gets(nome_arquivo3);
    init_btree(&root3, nome_arquivo3);
    printf("\t\tInforme o nome do arquivo para arvore 4 (.dat): ");
    gets(nome_arquivo4);
    init_btree(&root4, nome_arquivo4);

    do {
        system("cls");
        printf("\t\t1- Inserir um elemento na base\n");
        printf("\t\t2- Remover um elemento da base\n");
        printf("\t\t3- Buscar um elemento na base\n");
        printf("\t\t4- Criar arvore a partir de arquivo\n");
        printf("\t\t5- Gerar arquivos com elementos\n");
        printf("\t\t6- Criar arvore a partir de dados da memoria\n");
        printf("\t\t7- Sair\n");
        printf(">> ");
        scanf("%d", &opcao);
        system("cls");

        switch (opcao) {
        case 1: {
                int arvore;
                printf("\t\tInforme a arvore: ");
                scanf("%d", &arvore);
                elemento = (struct elementos*) malloc(sizeof(struct elementos));
                printf("\t\tInforme o inteiro: ");
                scanf("%d", &elemento->meuInteiro);
                switch (arvore) {
                case 1:
                    insert_b(&root1, &elemento->meuInteiro, sizeof(int), elemento, sizeof(struct elementos), funcs_comp, funcs_atrib);
                    break;

                case 2:
                    insert_b(&root2, &elemento->meuInteiro, sizeof(int), elemento, sizeof(struct elementos), funcs_comp, funcs_atrib);
                    break;

                case 3:
                    insert_b(&root3, &elemento->meuInteiro, sizeof(int), elemento, sizeof(struct elementos), funcs_comp, funcs_atrib);
                    break;

                case 4:
                    insert_b(&root4, &elemento->meuInteiro, sizeof(int), elemento, sizeof(struct elementos), funcs_comp, funcs_atrib);
                    break;

                }

            }
            break;

        case 2: {


                fflush(stdin);
                long int quantidade;
                int arvore;

                printf("\t\tInforme a arvore: ");
                scanf("%d", &arvore);
                printf("\t\tInforme a quantidade de buscas elementos: ");
                scanf("%ld", &quantidade);
                printf("\n\n\t\t\tPressione uma tecla para iniciar as buscas... ");
                getchar();
                int* vet_elems = (int*) malloc(sizeof(int) * MAX);
                for (register int i = 0; i < MAX; ++i)
                    vet_elems[i] = 0;

                int indice;
                for (register long int i = 0; i < quantidade;) {
                    do
                       indice = rand() % quantidade;
                    while (vet_elems[indice]);
                    vet_elems[indice] = 1;

                    elemento = (struct elementos*) malloc(sizeof(struct elementos));
                    elemento->meuInteiro = indice;
                    remover(&root1, &elemento->meuInteiro, sizeof(int), sizeof(struct elementos), funcs_comp, funcs_atrib);

                    ++i;
                }
                free(vet_elems);


            }
            break;

        case 3: {
                int opcao_busca = 0;
                int arvore;
                fflush(stdin);
                printf("\t\t1- Buscar um elemento\n\t\t2- Buscar varios elementos aleatorios\n>> ");
                scanf("%d", &opcao_busca);
                printf("\t\tInforme a arvore: ");
                scanf("%d", &arvore);

                if (opcao_busca == 1) {
                    printf("\t\tInforme a chave: ");
                    scanf("%d", &chave);

                    switch (arvore) {
                    case 1:
                        if (search_b(root1, &chave, funcs_comp, sizeof(int), sizeof(struct elementos)) == true)
                            printf("\t\t\tENCONTRAMOS O ELEMENTO!\n");
                        else
                            printf("\t\t\tNAO ENCONTRAMOS O ELEMENTO!\n");

                        break;

                    case 2:
                        if (search_b(root2, &chave, funcs_comp, sizeof(int), sizeof(struct elementos)) == true)
                            printf("\t\t\tENCONTRAMOS O ELEMENTO!\n");
                        else
                            printf("\t\t\tNAO ENCONTRAMOS O ELEMENTO!\n");
                        break;

                    case 3:
                        if (search_b(root3, &chave, funcs_comp, sizeof(int), sizeof(struct elementos)) == true)
                            printf("\t\t\tENCONTRAMOS O ELEMENTOS!\n");
                        else
                            printf("\t\t\tNAO ENCONTRAMOS O ELEMENTO!\n");
                        break;

                    case 4:
                        if (search_b(root4, &chave, funcs_comp, sizeof(int), sizeof(struct elementos)) == true)
                            printf("\t\t\tENCONTRAMOS O ELEMENTO!\n");
                        else
                            printf("\t\t\tNAO ENCONTRAMOS O ELEMENTO!\n");
                        break;

                    }


                } else if (opcao_busca == 2) {

                    DWORD inicio = 0, fim = 0, soma = 0;

                    int quantidade_buscas;
                    int buscas_sucesso = 0;
                    int buscas_fracasso = 0;

                    printf("\t\tInforme a quantidade de buscas < 500: ");
                    scanf("%d", &quantidade_buscas);
                    printf("\n\n\t\t\tPressione uma tecla para iniciar as buscas... ");
                    getchar();
                    int* vet_elems = (int*) malloc(sizeof(int) * MAX);
                    for (register int i = 0; i < MAX; ++i)
                        vet_elems[i] = 0;

                    int indice;
                    for (int i = 0; i < quantidade_buscas;) {
                        do
                           indice = rand() % MAX;
                        while (vet_elems[indice]);
                        vet_elems[indice] = 1;
                        int valor = indice;

                        bool retorno = false;
                        inicio = GetTickCount();
                        switch (arvore) {
                        case 1:
                            retorno = search_b(root1, &valor, funcs_comp, sizeof(int), sizeof(struct elementos));
                            break;

                        case 2:
                            retorno = search_b(root2, &valor, funcs_comp, sizeof(int), sizeof(struct elementos));
                            break;

                        case 3:
                            retorno = search_b(root3, &valor, funcs_comp, sizeof(int), sizeof(struct elementos));
                            break;

                        case 4:
                            retorno = search_b(root4, &valor, funcs_comp, sizeof(int), sizeof(struct elementos));
                            break;

                        }
                        fim = GetTickCount();
                        soma += (fim - inicio);
                        if (retorno == true)
                            buscas_sucesso++;
                        else
                            buscas_fracasso++;



                        ++i;
                    }
                    free(vet_elems);

                    printf("\n\t\tQuantidade de valores encontrados: %d\n"
                           "\t\tQuantidade de valores nao encontados: %d\n", buscas_sucesso, buscas_fracasso);
                    printf("\t\tMedia de tempo de busca: %.4fs\n", ((float)soma / (float)quantidade_buscas) / 1000.0);
                }

            }
            break;

        case 4: {
                char nome[100];
                int arvore;
                fflush(stdin);
                DWORD inicio;
                DWORD fim;
                printf("\t\tInforme o nome do arquivo: ");
                gets(nome);

                printf("\t\tInforme a arvore desejada: ");
                scanf("%d", &arvore);
                // Iniciar a contagem do tempo aqui
                inicio = GetTickCount();
                switch (arvore) {
                case 1:
                    gera_indice(&root1, nome, funcs_comp, funcs_atrib, sizeof(int), sizeof(struct elementos));
                    break;

                case 2:
                    gera_indice(&root2, nome, funcs_comp, funcs_atrib, sizeof(int), sizeof(struct elementos));
                    break;

                case 3:
                    gera_indice(&root3, nome, funcs_comp, funcs_atrib, sizeof(int), sizeof(struct elementos));
                    break;

                case 4:
                    gera_indice(&root4, nome, funcs_comp, funcs_atrib, sizeof(int), sizeof(struct elementos));
                    break;

                }
                fim = GetTickCount();
                printf("\n\t\tTempo decorrido: %.4fs\n", (float)(fim - inicio) / 1000.0);
                getchar();
                // terminar a contagem do tempo aqui

            }
            break;

        case 5:
            {
                char nome[100];
                long int quantidade;
                DWORD inicio;
                DWORD fim;

                printf("\t\tInforme a quantidade de elementos que deseja criar: ");
                scanf("%ld", &quantidade);
                fflush(stdin);
                printf("\t\tInforme o nome do arquivo: ");
                gets(nome);
                elementos_stream = fopen(strcat(nome, "_elementos.dat"), "w+b");
                if (!elementos_stream) {
                    perror("Nao foi possivel criar o arquivo: ");
                    return 1;
                }


                int* vet_elems = (int*) malloc(sizeof(int) * MAX);
                for (int i = 0; i < MAX; ++i)
                    vet_elems[i] = 0;


                int indice;
                inicio = GetTickCount();
                for (int i = 0; i < quantidade;) {
                    do
                       indice = rand() % MAX;
                    while (vet_elems[indice]);
                    vet_elems[indice] = 1;

                    elemento = (struct elementos*) malloc(sizeof(struct elementos));
                    if (elemento) {
                       elemento->meuInteiro = indice;
                       fwrite(elemento, sizeof(struct elementos), 1, elementos_stream);
                    }
                    ++i;
                }
                fim = GetTickCount();
                printf("\n\t\tTempo decorrido: %.4f\n", (float)(fim - inicio) / 1000.0);
                getchar();

                fclose(elementos_stream);
            }
            break;

        case 6: {
                fflush(stdin);
                long int quantidade;
                int arvore;

                printf("\t\tInforme a arvore: ");
                scanf("%d", &arvore);
                printf("\t\tInforme a quantidade de buscas elementos: ");
                scanf("%ld", &quantidade);
                printf("\n\n\t\t\tPressione uma tecla para iniciar as buscas... ");
                getchar();
                int* vet_elems = (int*) malloc(sizeof(int) * MAX);
                for (register int i = 0; i < MAX; ++i)
                    vet_elems[i] = 0;

                int indice;
                for (register long int i = 0; i < quantidade;) {
                    do
                       indice = rand() % quantidade;
                    while (vet_elems[indice]);
                    vet_elems[indice] = 1;

                    elemento = (struct elementos*) malloc(sizeof(struct elementos));
                    elemento->meuInteiro = indice;
                    switch (arvore) {
                    case 1:
                        insert_b(&root1, &elemento->meuInteiro, sizeof(int), elemento, sizeof(struct elementos), funcs_comp, funcs_atrib);
                        break;

                    case 2:
                        insert_b(&root2, &elemento->meuInteiro, sizeof(int), elemento, sizeof(struct elementos), funcs_comp, funcs_atrib);
                        break;

                    case 3:
                        insert_b(&root3, &elemento->meuInteiro, sizeof(int), elemento, sizeof(struct elementos), funcs_comp, funcs_atrib);
                        break;

                    case 4:
                        insert_b(&root4, &elemento->meuInteiro, sizeof(int), elemento, sizeof(struct elementos), funcs_comp, funcs_atrib);
                        break;

                    }



                    ++i;
                }
                free(vet_elems);
            }
            break;


        case 7:
            grava_root(&root1, nome_arquivo1);
            grava_root(&root2, nome_arquivo2);
            grava_root(&root3, nome_arquivo3);
            grava_root(&root4, nome_arquivo4);
            break;


        }
        system("pause");

    } while (opcao != 7);



    return 0;
}
