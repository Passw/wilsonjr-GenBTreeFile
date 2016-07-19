/***

	IMPLEMENTAÇÃO B+ EM ARQUIVO
	CODIFICADOR: WILSON ESTÉCIO MARCÍLIO JÚNIOR

*/

#include "bplus.h"

void pop(struct __pilha__** p)
{
	if (*p) {
		struct __pilha__* q = *p;
		*p = (*p)->prox;
		free(q);
	}
}

void push(struct __pilha__** p, int position)
{
	struct __pilha__* novo = malloc(sizeof(struct __pilha__));
	if (novo) {
		novo->position = position;

		novo->prox = *p;
		*p = novo;
	}
}

void init_btree(struct __bplusnode__* root, char* file_name)
{
    char auxiliar[100];
    strcpy(auxiliar, file_name);

    root->stream = fopen(strcat(file_name, "_arvore_stream.dat"), "r+b");
    if (!root->stream) {
        strcpy(file_name, auxiliar);
        root->stream = fopen(strcat(file_name, "_arvore_stream.dat"), "w+b");
    }

    strcpy(file_name, auxiliar);
    root->regs_stream = fopen(strcat(file_name, "_indice_stream.dat"), "r+b");
    if (!root->regs_stream) {
        strcpy(file_name, auxiliar);
        root->regs_stream = fopen(strcat(file_name, "_indice_stream.dat"), "w+b");
    }

    strcpy(file_name, auxiliar);
    root->ind_stream = fopen(strcat(file_name, "_registro_stream.dat"), "r+b");
    if (!root->ind_stream) {
        strcpy(file_name, auxiliar);
        root->ind_stream = fopen(strcat(file_name, "_registro_stream.dat"), "w+b");
    }
    root->pos_stream_root = 0;
    root->free_pos_stream = 0;
    root->free_pos_ind = 0;
    root->free_pos_regs = 0;

    strcpy(file_name, auxiliar);
    FILE* pos_root_stream = fopen(strcat(file_name, "_pos_root_stream.dat"), "rb");
    fread(&root->pos_stream_root, sizeof(int), 1, pos_root_stream);
    fclose(pos_root_stream);

    strcpy(file_name, auxiliar);
    FILE* pos_regs_free_stream = fopen(strcat(file_name, "_pos_regs_free_stream.dat"), "rb");
    if (pos_regs_free_stream) {
        while (true) {
            int pos = 0;
            fread(&pos, sizeof(int), 1, pos_regs_free_stream);
            if  (feof(pos_regs_free_stream) || ferror(pos_regs_free_stream))
                break;
            push(&root->free_pos_regs, pos);
        }
    }
    fclose(pos_regs_free_stream);

    strcpy(file_name, auxiliar);
    FILE* pos_ind_free_stream = fopen(strcat(file_name, "_pos_ind_free_stream.dat"), "rb");
    if (pos_ind_free_stream) {
        while (true) {
            int pos = 0;
            fread(&pos, sizeof(int), 1, pos_ind_free_stream);
            if  (feof(pos_ind_free_stream) || ferror(pos_ind_free_stream))
                break;
            push(&root->free_pos_ind, pos);
        }
    }
    fclose(pos_ind_free_stream);

    strcpy(file_name, auxiliar);
    FILE* pos_node_free_stream =  fopen(strcat(file_name, "_pos_node_free_stream.dat"), "rb");
    if (pos_node_free_stream) {
        while (true) {
            int pos = 0;
            fread(&pos, sizeof(int), 1, pos_node_free_stream);
            if  (feof(pos_node_free_stream) || ferror(pos_node_free_stream))
                break;
            push(&root->free_pos_stream, pos);
        }
    }
    fclose(pos_node_free_stream);

    strcpy(file_name, auxiliar);
}


bool search_b(struct __bplusnode__ root, void* key, int (*functions_comp[])(const void*, const void*), size_t n_key, size_t n_record)
{
	return search_aux(root.stream, root.regs_stream, root.ind_stream, root.pos_stream_root, key, functions_comp, n_key, n_record);
}

bool search_aux(FILE* node_stream, FILE* regs_stream, FILE* ind_stream, int pos, void* key, int (*functions_comp[])(const void*, const void*),
	size_t n_key, size_t n_record)
{

	struct __unionnode__ node;
	fseek(node_stream, sizeof(struct __unionnode__) * pos, SEEK_SET);
	if (fread(&node, sizeof(struct __unionnode__), 1, node_stream) != 1)
		return false;

	if (node.interno == true) {

		int i = 0;
		void* value = (void*) malloc(n_key * (2*M - 1));

		/* Recupera o vetor de índices */
		fseek(ind_stream, (n_key * (2*M - 1)) * node.no_interno.keys, SEEK_SET);
		fread(value, n_key, 2*M - 1, ind_stream);

		/* Compara coms*/
		for (; i < node.no_interno.numero_elem && functions_comp[0](value + i*n_key, key) <= 0; ++i);
		free(value);

		return search_aux(node_stream, regs_stream, ind_stream, node.no_interno.filhos[i], key, functions_comp, n_key, n_record);
	} else {
		int i = 0;
		void* value = (void*) malloc(n_record * (2*M - 1));

		/* Recupera o vetor de registros pertencentes a este nó */

		fseek(regs_stream, (n_record * (2*M - 1)) * node.no_folha.records, SEEK_SET);
		fread(value, n_record, (2*M - 1), regs_stream);

		/* Compara o vetor de registro com a chave */
		for (; i < node.no_folha.numero_elem && functions_comp[2](value + i*n_record, key) < 0; ++i);


		if (i == node.no_folha.numero_elem || functions_comp[2](value + i*n_record, key) > 0) {
            free(value);
			return false;
		} else {
		    free(value);
			return true;
        }
	}
}


void imprime(void* record)
{
    struct elementos* rec = (struct elementos*) record;

    printf("\n\n func: %d\n\n", rec->meuInteiro);
}

void init_bplus(struct __bplusnode__* root, void* record, size_t n_record, void (*functions_atrib[])(void*, void*, size_t, size_t))
{
	struct __unionnode__* node = (struct __unionnode__*) malloc(sizeof(struct __unionnode__));
	if (node) {
		node->interno = false;
		node->no_folha.prox = -1;
		node->no_folha.ant = -1;
		node->no_folha.numero_elem = 1;

		/* Aloca espaço para o registro*/
		void* value = (void*) malloc(n_record * (2*M-1));
		functions_atrib[1](value, record, n_record, n_record);

		/* Insere no disco o primeiro nó modificado */
		fseek(root->regs_stream, 0, SEEK_SET);
		node->no_folha.records = ftell(root->regs_stream) / ((2*M - 1) * n_record);
		fwrite(value, n_record, 2*M - 1, root->regs_stream);

		fseek(root->stream, sizeof(struct __unionnode__) * root->pos_stream_root, SEEK_SET);
		fwrite(node, sizeof(struct __unionnode__), 1, root->stream);
	}
}

void insert_b(struct __bplusnode__* root, void* key, size_t n_key, void* record, size_t n_record,
    int (*functions_comp[])(const void*, const void*), void (*functions_atrib[])(void*, void*, size_t, size_t))
{
	bool flag = false;
	struct __unionnode__ node;

	/* Recupera o a raiz da arvore, se houver. Caso contrário, um raiz é criada */
	fseek(root->stream, sizeof(struct __unionnode__) * root->pos_stream_root, SEEK_SET);
	if (fread(&node, sizeof(struct __unionnode__), 1, root->stream) != 1) {
		init_bplus(root, record, n_record, functions_atrib);
		return;
    }
    /* Verifica se o nó está cheio*/
	if ((node.interno && node.no_interno.numero_elem == 2*M-1) ||
		(!node.interno && node.no_folha.numero_elem == 2*M-1)) {
		/* Cria um novo nó, que será o pai */

		struct __unionnode__* novo = (struct __unionnode__*) malloc(sizeof(struct __unionnode__));
		if (novo) {
			/* Inicializa os elementos do novo pai */
            novo->no_interno.numero_elem = 0;
			novo->interno = true;
			novo->no_interno.keys = -1;
			for (int i = 0; i < 2*M; ++i)
				novo->no_interno.filhos[i] = -1;

			/* O primeiro filho do pai será o local da raiz corrente*/
			novo->no_interno.filhos[0] = root->pos_stream_root;

			/* Coloca o novo nó final (Recuperando sua posição)*/
			if (!root->free_pos_stream) // posição livre é a do final do arquivo
				fseek(root->stream, 0, SEEK_END);
			else {			// tem posição livre no início do aquivo
				// usa a primeira posição disponivel, elimina-a
				fseek(root->stream, sizeof(struct __unionnode__) * root->free_pos_stream->position, SEEK_SET);
				pop(&root->free_pos_stream);
			}
			int position_new = ftell(root->stream) / sizeof(struct __unionnode__) ;
			fwrite(novo, sizeof(struct __unionnode__), 1, root->stream);
			int p = root->pos_stream_root;
			root->pos_stream_root = position_new;

			// Passa o arquivo e as duas posições, correspondentes ao pai e ao filho, além do índice do array
			divide_bmais(root->stream, root->regs_stream, root->ind_stream, root->pos_stream_root, p, 0, functions_atrib,
				&root->free_pos_stream, &root->free_pos_ind, &root->free_pos_regs, n_key, n_record);


		}
	}

	insert_aux(root->stream, root->ind_stream, root->regs_stream, root->pos_stream_root, key, n_key, record, n_record, functions_comp,
        functions_atrib, &flag, &root->free_pos_stream, &root->free_pos_ind, &root->free_pos_regs);
}



void insert_aux(FILE* stream, FILE* ind_stream, FILE* regs_stream, int pos_stream_root, void* key,
	size_t n_key, void* record, size_t n_record, int (*functions_comp[])(const void*, const void*),
    void (*functions_atrib[])(void*, void*, size_t, size_t), bool* flag, struct __pilha__** free_pos_stream,
	struct __pilha__** free_pos_ind, struct __pilha__** free_pos_regs)
{
    struct __unionnode__ node;
	fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
	fread(&node, sizeof(struct __unionnode__), 1, stream);

	int i = 0;

    if (node.interno == true) {
 		/* Busca o vetor de índices do nó corrente no arquivo de índices*/
		void* value = (void*) malloc(n_key * (2*M - 1));
		fseek(ind_stream, (n_key * (2*M-1)) * node.no_interno.keys, SEEK_SET);
		fread(value, n_key, (2 * M-1), ind_stream);

		/* Buscar o ponteiro para descer na árvore */

		for (; i < node.no_interno.numero_elem && functions_comp[0](value + i*n_key, key) < 0; ++i) ;
		free(value);
		insert_aux(stream, ind_stream, regs_stream, node.no_interno.filhos[i], key, n_key, record, n_record, functions_comp, functions_atrib,
             flag, free_pos_stream, free_pos_ind, free_pos_regs);
	} else {
		/* Encontramos o nó folha, agora verifica se é possível inserir */

		if (node.no_folha.numero_elem == 2*M-1) // cheio
 			*flag = true;
		else {
         	void* value = (void*) malloc(n_record * (2*M - 1));
         	fseek(regs_stream, (n_record * (2*M - 1)) * node.no_folha.records, SEEK_SET);
        	fread(value, n_record, (2*M - 1), regs_stream);
			int j = node.no_folha.numero_elem;

			/* Abre um espaço no lugar correto, para inserir ordenado*/
			while (j > 0 && functions_comp[2](value + (j-1)*n_record, key) > 0) {
				functions_atrib[1](value + j*n_record, value + (j-1)*n_record, n_record, n_record);
				j--;
			}
			functions_atrib[1](value + j*n_record, record, n_record, n_record);
			node.no_folha.numero_elem++;
			*flag = false;

			/* Atualiza no disco o nó modificado */
			fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
			fwrite(&node, sizeof(struct __unionnode__), 1, stream);

			/* Atualiza no disco o vetor de registro do nó modificado */
			fseek(regs_stream, (n_record * (2*M-1)) * node.no_folha.records, SEEK_SET);
			fwrite(value, n_record, (2 * M-1), regs_stream);
		}
		return;
	}

	if (*flag) {
		if (node.no_interno.numero_elem != 2*M-1) {
            divide_bmais(stream, regs_stream, ind_stream, pos_stream_root, node.no_interno.filhos[i], i, functions_atrib,
				free_pos_stream, free_pos_ind, free_pos_regs, n_key, n_record);
			insert_aux(stream, ind_stream, regs_stream, pos_stream_root, key, n_key, record, n_record, functions_comp,
				functions_atrib, flag, free_pos_stream, free_pos_ind, free_pos_regs);
		}
	}
}

void divide_bmais(FILE* stream, FILE* regs_stream, FILE* ind_stream, int pos_stream_root,
	int pos_filho, int ind, void (*functions_atrib[])(void*, void*, size_t, size_t),
	struct __pilha__** free_pos_stream, struct __pilha__** free_pos_ind, struct __pilha__** free_pos_regs,
	size_t n_key, size_t n_record)
{

	/**
	* function_atrib[0]: atribuição de keys para keys
	* function_atrib[1]: atribuição de records para records
	* function_atrib[2]: atribuição de records para keys
	*/
	struct __unionnode__* new = (struct __unionnode__*) malloc(sizeof(struct __unionnode__));
	if (new) {
		struct __unionnode__ node;
		int pos_new = -1;
		int pos_reg_ind = -1;
		/* Recupera a nó filho do arquivo */
		fseek(stream, sizeof(struct __unionnode__) * pos_filho, SEEK_SET);
		fread(&node, sizeof(struct __unionnode__), 1, stream);

		new->interno = node.interno;


		if (new->interno == true) {
			new->no_interno.numero_elem = M-1;
			for (int i = 0; i < 2*M; ++i)
				new->no_interno.filhos[i] = -1;
			int i = M;
			/* Cria o novo nó de indices */
			void* value = (void*) malloc(n_key * (2*M-1));
			void* value_ind = (void*) malloc(n_key * (2*M-1));

			/* Recupera o vetor de índices pertencente aquele nó */
			fseek(ind_stream, (n_key * (2*M-1)) * node.no_interno.keys, SEEK_SET);
			fread(value, n_key, 2*M-1, ind_stream);

			/* O novo nó criado receberá a metade superior de elementos do filho (nó cheio) */
			for (; i < 2*M-1; ++i) {
				functions_atrib[0](value_ind + (i-M)*n_key, value + i*n_key, n_key, n_key);
				new->no_interno.filhos[i-M] = node.no_interno.filhos[i];
				node.no_interno.filhos[i] = -1;
			}

			new->no_interno.filhos[i-M] = node.no_interno.filhos[i];
			node.no_interno.filhos[i] = -1;
			node.no_interno.numero_elem = M-1;

			/* Atualiza no disco o nó que estava cheio */
			fseek(stream, sizeof(struct __unionnode__) * pos_filho, SEEK_SET);
			fwrite(&node, sizeof(struct __unionnode__), 1, stream);

			fseek(ind_stream, (n_key * (2*M-1)) * node.no_interno.keys, SEEK_SET);
			fwrite(value, n_key, 2*M-1, ind_stream);

			/* Escreve o novo nó criado, guardando sua posição lógica */
			if (!*free_pos_stream) // posição livre é a do final do arquivo
				fseek(stream, 0, SEEK_END);
			else {			// tem posição livre no início do aquivo
				// usa a primeira posição disponivel, elimina-a
				fseek(stream, sizeof(struct __unionnode__) * (*free_pos_stream)->position, SEEK_SET);
				pop(free_pos_stream);
			}

			/* Escreve o vetor de índices criado, guardando a sua posição lógica */
			if (!*free_pos_ind)
				fseek(ind_stream, 0, SEEK_END);
			else {
				fseek(ind_stream, n_key * (2*M-1) * (*free_pos_ind)->position, SEEK_SET);
				pop(free_pos_ind);
			}

			// Escreve no arquivo
			pos_reg_ind = ftell(ind_stream) / (n_key * (2*M-1));
			pos_new = ftell(stream) / sizeof(struct __unionnode__);
			new->no_interno.keys = pos_reg_ind;
			fwrite(value_ind, n_key, (2 * M-1), ind_stream);
			fwrite(new, sizeof(struct __unionnode__), 1, stream);

			free(value);
            free(value_ind);
		} else {
            new->no_folha.numero_elem = M;
			int i = M-1;

			/* Cria o novo nó de registros */
			void* value = (void*) malloc(n_record * (2*M - 1));
			void* value_rec = (void*) malloc(n_record * (2*M - 1));


			/* Recupera o vetor de registros pertencente aquele nó */
			fseek(regs_stream, (n_record * (2*M-1)) * node.no_folha.records, SEEK_SET);
			fread(value, n_record, 2*M-1, regs_stream);

            /* Atribuição dos registros */
			for (; i < 2*M-1; ++i)
				functions_atrib[1](value_rec + (i - (M-1))*n_record, value + i*n_record, n_record, n_record);

            /* Atualiza as ligações*/
			new->no_folha.prox = node.no_folha.prox;
			new->no_folha.ant = pos_filho;

            /* Escreve o novo nó criado, guardando sua posição lógica */
			if (!*free_pos_stream) // posição livre é a do final do arquivo
				fseek(stream, 0, SEEK_END);
			else {			// tem posição livre no início do aquivo
				// usa a primeira posição disponivel, elimina-a
				fseek(stream, sizeof(struct __unionnode__) * (*free_pos_stream)->position, SEEK_SET);
				pop(free_pos_stream);
			}

            /* Escreve o vetor de registros criado, guardando a sua posição lógica */
			if (!*free_pos_regs)
				fseek(regs_stream, 0, SEEK_END);
			else {
				fseek(regs_stream, n_record * (2*M-1) * (*free_pos_regs)->position, SEEK_SET);
				pop(free_pos_regs);
			}

			pos_reg_ind = ftell(regs_stream) / (n_record * (2*M-1));
			pos_new = ftell(stream) / sizeof(struct __unionnode__);

			new->no_folha.records = pos_reg_ind;

			fwrite(value_rec, n_record, (2*M-1), regs_stream);
			fwrite(new, sizeof(struct __unionnode__), 1, stream);

			node.no_folha.prox = pos_new;
			node.no_folha.numero_elem = M-1;

            /* Atualiza no disco o nó que estava cheio */
            fseek(regs_stream, (n_record * (2*M-1)) * node.no_folha.records, SEEK_SET);
			fwrite(value, n_record, 2*M-1, regs_stream);

			fseek(stream, sizeof(struct __unionnode__) * pos_filho, SEEK_SET);
			fwrite(&node, sizeof(struct __unionnode__), 1, stream);



			/* Atualiza o ponteiro do proximo */
			if (new->no_folha.prox != -1) {

				struct __unionnode__ auxnode;
				fseek(stream, sizeof(struct __unionnode__) * new->no_folha.prox, SEEK_SET);
				fread(&auxnode, sizeof(struct __unionnode__), 1, stream);

				auxnode.no_folha.ant = pos_new;
				fseek(stream, sizeof(struct __unionnode__) * new->no_folha.prox, SEEK_SET);
				fwrite(&auxnode, sizeof(struct __unionnode__), 1, stream);

			}

			free(value);
		}

		struct __unionnode__ pai;

		/* Busca o elemento pai */
		fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
		fread(&pai, sizeof(struct __unionnode__), 1, stream);

		/* Leitura do vetor de índices do pai */
		void* value = (void*) malloc(n_key * (2*M-1));

        if (pai.no_interno.keys != -1) {
            fseek(ind_stream, (n_key * (2*M-1)) * pai.no_interno.keys, SEEK_SET);
            fread(value, n_key, (2*M-1), ind_stream);
        }

		/* Abre um espaço para inserção da nova chave */
		for (int k = pai.no_interno.numero_elem; k > ind; --k) {
			pai.no_interno.filhos[k+1] = pai.no_interno.filhos[k];
			functions_atrib[0](value + k*n_key, value + (k-1)*n_key, n_key, n_key);
		}

		/* Insere a nova chave no espaço aberto*/
		if (new->interno == true) {
			void* value_ind = (void*) malloc(n_key * (2*M-1));

			/* Recupera o vetor de índices pertencente aquele nó */
			fseek(ind_stream, (n_key * (2*M-1)) * node.no_interno.keys, SEEK_SET);
			fread(value_ind, n_key, 2*M-1, ind_stream);

			functions_atrib[0](value + ind*n_key, value_ind + (M-1)*n_key, n_key, n_key);

			free(value_ind);
		} else {
			void* value_rec = (void*) malloc(n_record * (2*M-1));

			/* Recupera o vetor de registros pertencente aquele nó */
			fseek(regs_stream, (n_record * (2*M-1)) * node.no_folha.records, SEEK_SET);
			fread(value_rec, n_record, 2*M-1, regs_stream);

			functions_atrib[2](value + ind*n_key, value_rec + (M-1)*n_record, n_key, n_record);

			free(value_rec);
		}


		pai.no_interno.filhos[ind] = pos_filho;
		pai.no_interno.filhos[ind+1] = pos_new;
		pai.no_interno.numero_elem++;



		/* Atauliza em disco, o vetor de índices do pai */
        if (pai.no_interno.keys != -1) {
            fseek(ind_stream, (n_key * (2*M-1)) * pai.no_interno.keys, SEEK_SET);

		} else {

		    /* Escreve o vetor de índices criado, guardando a sua posição lógica */
			if (!*free_pos_ind)
				fseek(ind_stream, 0, SEEK_END);
			else {
				fseek(ind_stream, n_key * (2*M-1) * (*free_pos_ind)->position, SEEK_SET);
				pop(free_pos_ind);
			}
		}
		pai.no_interno.keys = ftell(ind_stream) / ((2*M-1) * n_key);
        fwrite(value, n_key, (2*M-1), ind_stream);
        /* Atualiza em disco o nó pai*/
		fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
		fwrite(&pai, sizeof(struct __unionnode__), 1, stream);

		free(value);
	}
}

void remover(struct __bplusnode__* root, void* key, size_t n_key, size_t n_record, int (*functions_comp[])(const void*, const void*),
	void (*functions_atrib[])(void*, void*, size_t, size_t))
{
    bool flag = true;
    remove_aux(root->stream, root->ind_stream, root->regs_stream, root->pos_stream_root, key, n_key, n_record, functions_comp, functions_atrib, &flag);

	struct __unionnode__ node;

	fseek(root->stream, sizeof(struct __unionnode__) * root->pos_stream_root, SEEK_SET);
	fread(&node, sizeof(struct __unionnode__), 1, root->stream);

	/* Atualiza o novo nó raiz, caso a mesma esteja nula */
	if (node.interno && !node.no_interno.numero_elem)
		root->pos_stream_root = node.no_interno.filhos[0];
	else if (!node.interno && !node.no_folha.numero_elem)
		root->pos_stream_root = 0;

	find_free_positions(root);
}


void find_free_positions(struct __bplusnode__* root)
{
	struct __unionnode__ node;
	fseek(root->stream, 0, SEEK_SET);

	while (true) {
		int pos = ftell(root->stream) / sizeof(struct __unionnode__);
		fread(&node, sizeof(struct __unionnode__), 1, root->stream);

		/* O nó recuperado não possui elementos, logo é uma posição vaga
		* Assim, seu índice, ou registro também é invalido e também é uma posição válida.
		*/
		if ((node.interno && !node.no_interno.numero_elem) || (!node.interno && !node.no_folha.numero_elem)) {
			push(&root->free_pos_stream, pos);

			if (node.interno)
				push(&root->free_pos_ind, node.no_interno.keys);
			else
				push(&root->free_pos_regs, node.no_folha.records);
		}

		/* Verifica se deu erro na leitura ou chegamos ao final do arquivo */
		if(feof(root->stream) || ferror(root->stream))
			break;
	}
}


void remove_aux(FILE* stream, FILE* ind_stream, FILE* regs_stream, int pos_stream_root, void* key, size_t n_key, size_t n_record,
	int (*functions_comp[])(const void*, const void*),	void (*functions_atrib[])(void*, void*, size_t, size_t), bool* entra)
{
	struct __unionnode__ node;

	fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
	if (fread(&node, sizeof(struct __unionnode__), 1, stream) != 1)
		return;

	if (node.interno) {
		void* value = (void*) malloc(n_key * (2*M-1));
		fseek(ind_stream, (n_key * (2*M-1)) * node.no_interno.keys, SEEK_SET);
		fread(value, n_key, (2*M - 1), ind_stream);

		int i = 0;
		/* Procura pelo possível nó */
		for (; i < node.no_interno.numero_elem && functions_comp[0](value + i*n_key, key) <= 0; ++i);
		free(value);
		/* Desce na arvore */
		remove_aux(stream, ind_stream, regs_stream, node.no_interno.filhos[i], key, n_key, n_record, functions_comp, functions_atrib, entra);
		/* Verifica se as propriedades de arvore B ainda estão válidas */
		if( *entra )
            fix(stream, ind_stream, regs_stream, pos_stream_root, node.no_interno.filhos[i], i, n_key, n_record,
                functions_atrib, functions_comp);

	} else {
		int i = 0;

		void* value = (void*) malloc(n_record * (2*M - 1));
		fseek(regs_stream, (n_record * (2*M - 1)) * node.no_folha.records, SEEK_SET);
		fread(value, n_record, (2*M - 1), regs_stream);

		for (; i < node.no_folha.numero_elem && functions_comp[2](value + i*n_record, key); ++i);

		if (i != node.no_folha.numero_elem) {
                *entra = true;
            printf("achou\n");
			/* Remove o elemento */
			while (i < node.no_folha.numero_elem) {
				functions_atrib[2](value + i*n_record, value + (i+1)*n_record, n_record, n_record);
				++i;
			}
			node.no_folha.numero_elem--;
			/* Atualiza o nó modificado */
			fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
			fwrite(&node, sizeof(struct __unionnode__), 1, stream);

			/* Atualiza o vetor de registros do nó */
			fseek(regs_stream, (n_record * (2*M - 1)) * node.no_folha.records, SEEK_SET);
			fwrite(value, n_record, (2*M - 1), regs_stream);
		} else{
            printf("nao achou");
            *entra = false;
            }
	}
}


void fix(FILE* stream, FILE* ind_stream, FILE* regs_stream, int pos_stream_root, int pos_filho, int i, size_t n_key, size_t n_record,
 	void (*functions_atrib[])(void*, void*, size_t, size_t), int (*functions_comp[])(const void*, const void*))
{
	struct __unionnode__ painode, filhonode, irmaonode;

	/* Recupera o nó pai do arquivo */
	fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
	if( fread(&painode, sizeof(struct __unionnode__), 1, stream)  != 1)
        return;

	/* Recupera o nó para verificação */
	fseek(stream, sizeof(struct __unionnode__) * pos_filho, SEEK_SET);
	if( fread(&filhonode, sizeof(struct __unionnode__), 1, stream)  !=  1)
        return;
    if( filhonode.interno )
        printf(">> %d <<\n", filhonode.no_interno.numero_elem);
    else
        printf(">> %d << \n", filhonode.no_folha.numero_elem);
    printf("passou\n");
	if ((filhonode.interno && filhonode.no_interno.numero_elem < M-1) ||
		(!filhonode.interno && filhonode.no_folha.numero_elem < M-1)) {

		if (filhonode.interno) {
			/* Verifica se o irmão da esquerda possui um elemento para emprestar */
			if (i) {
				/* Recupera o irmao da esquerda*/
				fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i-1], SEEK_SET);
				fread(&irmaonode, sizeof(struct __unionnode__), 1, stream);

				/* Verifica se o irmao pode emprestar */
				if (irmaonode.no_interno.numero_elem > M-1) {
					void* vet_ind_filho = (void*) malloc(n_key * (2*M - 1));
					void* vet_ind_irmao = (void*) malloc(n_key * (2*M - 1));

					/* Recupera o vetor correspondente ao nó filhonode */
					fseek(ind_stream, (n_key * (2*M - 1)) * filhonode.no_interno.keys, SEEK_SET);
					fread(vet_ind_filho, n_key, (2*M - 1), ind_stream);
					/* Recupera o vetor correspondente ao nó filhonode */
					fseek(ind_stream, (n_key * (2*M - 1)) * irmaonode.no_interno.keys, SEEK_SET);
					fread(vet_ind_irmao, n_key, (2*M - 1), ind_stream);

					functions_atrib[0](vet_ind_filho + (i-1)*n_key, vet_ind_irmao + (irmaonode.no_interno.numero_elem-1)*n_key, n_key, n_key);

					irmaonode.no_interno.numero_elem--;

					int j = filhonode.no_interno.numero_elem;
					/* Abre um espaço na primeira posição do vetor para inserir o elemento emprestado */
					for (; j > 0; --j) {
						functions_atrib[0](vet_ind_filho + j*n_key, vet_ind_filho + (j-1)*n_key, n_key, n_key);
						filhonode.no_interno.filhos[j+1] = filhonode.no_interno.filhos[j];
					}
					filhonode.no_interno.filhos[j+1] = filhonode.no_interno.filhos[j];

					filhonode.no_interno.numero_elem++;

					struct __unionnode__ auxnode = irmaonode;
					int auxpos = painode.no_interno.filhos[i-1];

					/* Procura o elemento pra onde a chave emprestava apontava*/
					while (auxnode.interno) {
						fseek(stream, sizeof(struct __unionnode__) * auxnode.no_interno.filhos[auxnode.no_interno.numero_elem], SEEK_SET);
						fread(&auxnode, sizeof(struct __unionnode__), 1, stream);
					}

					void* vet_reg = (void*) malloc(n_record * (2*M - 1));
					fseek(regs_stream, (n_record * (2*M - 1)) * auxnode.no_folha.records, SEEK_SET);
					fread(vet_reg, n_record, 1, regs_stream);

					functions_atrib[2](vet_ind_filho, vet_reg + (auxnode.no_folha.numero_elem-1)*n_record , n_key, n_record);


					/* Atualiza os nós em arquivo */
					fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i-1], SEEK_SET);
					fwrite(&irmaonode, sizeof(struct __unionnode__), 1, stream);

					fseek(stream, sizeof(struct __unionnode__) * pos_filho, SEEK_SET);
					fwrite(&filhonode, sizeof(struct __unionnode__), 1, stream);

					fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
					fwrite(&painode, sizeof(struct __unionnode__), 1, stream);

					/* Atualiza os índices modificados */
					fseek(ind_stream, (n_key * (2*M - 1)) * filhonode.no_interno.keys, SEEK_SET);
					fwrite(vet_ind_filho, n_key, (2*M - 1), ind_stream);

					fseek(ind_stream, (n_key * (2*M - 1)) * irmaonode.no_interno.keys, SEEK_SET);
					fwrite(vet_ind_irmao, n_key, (2*M - 1), ind_stream);

					free(vet_ind_irmao);
					free(vet_ind_filho);
					free(vet_reg);
					return;
				}
			}

			/* Verifica se o irmão da direita possui um elemento para emprestar */
			if (i != painode.no_interno.numero_elem) {

				/* Recupera o irmao da direita */
				fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i+1], SEEK_SET);
				fread(&irmaonode, sizeof(struct __unionnode__), 1, stream);

				/* Verifica se o irmao pode emprestar*/
				if (irmaonode.no_interno.numero_elem > M-1) {
					void* copy = (void*) malloc(n_key);
					void* vet_ind_filho = (void*) malloc(n_key * (2*M - 1));
					void* vet_ind_irmao = (void*) malloc(n_key * (2*M - 1));

					/* Recupera os vetores dos arquivos dos nós filho e irmao */
					fseek(ind_stream, (n_key * (2*M - 1)) * filhonode.no_interno.keys, SEEK_SET);
					fread(vet_ind_filho, n_key, (2*M - 1), ind_stream);
					fseek(ind_stream, (n_key * (2*M - 1)) * irmaonode.no_interno.keys, SEEK_SET);
					fread(vet_ind_irmao, n_key, (2*M - 1), ind_stream);

					functions_atrib[0](copy, vet_ind_irmao, n_key, n_key);

					int j = 0;

					/* Remove o primeiro elemento (elemento emprestado) do nó que emprestou */
					for (; j < irmaonode.no_interno.numero_elem; ++j) {
						functions_atrib[0](vet_ind_irmao + j*n_key, vet_ind_irmao + (j+1)*n_key, n_key, n_key);
						irmaonode.no_interno.filhos[j] = irmaonode.no_interno.filhos[j+1];
					}
					irmaonode.no_interno.filhos[j] = irmaonode.no_interno.filhos[j+1];

					irmaonode.no_interno.numero_elem--;

					struct __unionnode__ filhoaux, copia_irmao = irmaonode;

					fseek(stream, sizeof(struct __unionnode__) * irmaonode.no_interno.filhos[0], SEEK_SET);
					fread(&filhoaux, sizeof(struct __unionnode__), 1, stream);

					/* Busca o menor registro */
					while (filhoaux.interno) {
						copia_irmao = filhoaux;
						fseek(stream, sizeof(struct __unionnode__) * filhoaux.no_interno.filhos[0], SEEK_SET);
						fread(&filhoaux, sizeof(struct __unionnode__), 1, stream);
					}

					/* Recupera o vetor de registros*/
					void* vet_regs = (void*) malloc(n_record * (2*M - 1));
					fseek(regs_stream, (n_record * (2*M-1)) * filhoaux.no_folha.records, SEEK_SET);
					fread(vet_regs, n_record, (2*M - 1), regs_stream);

					/* O menor registro do irmao passa ser o maior registro do filho */
					functions_atrib[2](vet_ind_filho + filhonode.no_interno.numero_elem*n_key , vet_regs, n_key, n_record);

					/* Recupera o vetor de inddices do pai para atualizar as chaves */
					void* vet_ind_pai = (void*) malloc(n_key * (2*M - 1));
					fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
					fread(vet_ind_pai, n_key, (2*M - 1), ind_stream);

					functions_atrib[0](vet_ind_pai + i*n_key, copy, n_key, n_key);
					filhonode.no_interno.numero_elem++;

					/* Grava as modificações feitas nos índices*/
					fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
					fwrite(vet_ind_pai, n_key, (2*M - 1), ind_stream);

					fseek(ind_stream, (n_key * (2*M - 1)) * irmaonode.no_interno.keys, SEEK_SET);
					fwrite(vet_ind_irmao, n_key, (2*M - 1), ind_stream);

					fseek(ind_stream, (n_key * (2*M - 1)) * filhonode.no_interno.keys, SEEK_SET);
					fwrite(vet_ind_filho, n_key, (2*M - 1), ind_stream);

					/* Atualiza os nós em arquivo */
					fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i+1], SEEK_SET);
					fwrite(&irmaonode, sizeof(struct __unionnode__), 1, stream);

					fseek(stream, sizeof(struct __unionnode__) * pos_filho, SEEK_SET);
					fwrite(&filhonode, sizeof(struct __unionnode__), 1, stream);

					fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
					fwrite(&painode, sizeof(struct __unionnode__), 1, stream);

					free(vet_ind_filho);
					free(vet_ind_irmao);
					free(vet_ind_pai);
					free(vet_regs);

					return;
				}
			}

			/* Nenhum irmão tem elemento suficiente que possa emprestar,
			* é necessário então concatenar dois nós
			*/
			if (i == painode.no_interno.numero_elem)
				i--;

            fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i], SEEK_SET);
            fread(&filhonode, sizeof(struct __unionnode__), 1, stream);

			struct __unionnode__ irmaonode;
			/* Busca o irmao da direita (temos certeza que ele existe) */
			fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i+1], SEEK_SET);
			fread(&irmaonode, sizeof(struct __unionnode__), 1, stream);

			/* Verifica se é possível concatenar os elementos do filho e do irmao da direita */
			if (filhonode.no_interno.numero_elem + irmaonode.no_interno.numero_elem <= (2*M - 1)) {

				int k = irmaonode.no_interno.numero_elem;

				/* Aloca e busca em arquivo os vetores de índices dos nós correspondentes */
				void* vet_ind_pai = (void*) malloc(n_key * (2*M - 1));
				void* vet_ind_filho = (void*) malloc(n_key * (2*M - 1));
				void* vet_ind_irmao = (void*) malloc(n_key * (2*M - 1));
				fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
				fread(vet_ind_pai, n_key, (2*M - 1), ind_stream);
				fseek(ind_stream, (n_key * (2*M - 1)) * irmaonode.no_interno.keys, SEEK_SET);
				fread(vet_ind_irmao, n_key, (2*M - 1), ind_stream);
				fseek(ind_stream, (n_key * (2*M - 1)) * filhonode.no_interno.keys, SEEK_SET);
				fread(vet_ind_filho, n_key, (2*M - 1), ind_stream);

				/* Abre um espaço no no irmao, e insere a chave do pai */
				for (; k > 0; --k) {
					functions_atrib[0](vet_ind_irmao + k*n_key, vet_ind_irmao + (k-1)*n_key, n_key, n_key);
					irmaonode.no_interno.filhos[k+1] = irmaonode.no_interno.filhos[k];
				}
				irmaonode.no_interno.filhos[k+1] = irmaonode.no_interno.filhos[k];
				functions_atrib[0](vet_ind_irmao, vet_ind_pai + i*n_key, n_key, n_key);
				irmaonode.no_interno.numero_elem++;

				/* Copia todas as chaves do no irmao para o no filho */
				int j = filhonode.no_interno.numero_elem;
				for (k = 0; j < 2*M && k < irmaonode.no_interno.numero_elem; ++k, ++j) {
					functions_atrib[0](vet_ind_filho + j*n_key, vet_ind_irmao + k*n_key, n_key, n_key);
					filhonode.no_interno.filhos[j+1] = irmaonode.no_interno.filhos[k+1];
				}

				filhonode.no_interno.numero_elem += irmaonode.no_interno.numero_elem;
            	/* Atualiza os ponteiros do pai */
				k = i;
				for (; k < painode.no_interno.numero_elem-1; ++k)
					functions_atrib[0](vet_ind_pai + k*n_key, vet_ind_pai + (k+1)*n_key, n_key, n_key);

				k = i+1;
				for (; k < painode.no_interno.numero_elem; ++k) {
					painode.no_interno.filhos[k] = painode.no_interno.filhos[k+1];
					painode.no_interno.filhos[k+1] = -1;
				}
				painode.no_interno.numero_elem--;

				/*Zera a quantidade de elementos do no removido para sua posição ser incluída na lista de posições livres*/
				irmaonode.no_interno.numero_elem = 0;

				/* Grava as modificações feitas nos índices*/
				fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
				fwrite(vet_ind_pai, n_key, (2*M - 1), ind_stream);
				fseek(ind_stream, (n_key * (2*M - 1)) * irmaonode.no_interno.keys, SEEK_SET);
				fwrite(vet_ind_irmao, n_key, (2*M - 1), ind_stream);
				fseek(ind_stream, (n_key * (2*M - 1)) * filhonode.no_interno.keys, SEEK_SET);
				fwrite(vet_ind_filho, n_key, (2*M - 1), ind_stream);

				/* Atualiza os nós em arquivo */
				fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i+1], SEEK_SET);
				fwrite(&irmaonode, sizeof(struct __unionnode__), 1, stream);
				fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i], SEEK_SET);
				fwrite(&filhonode, sizeof(struct __unionnode__), 1, stream);
				fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
				fwrite(&painode, sizeof(struct __unionnode__), 1, stream);
				free(vet_ind_filho);
				free(vet_ind_irmao);
				free(vet_ind_pai);
				return;

			} else if (i) {

				/* Busca o irmao da esquerda (temos certeza que ele existe) */
				fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i-1], SEEK_SET);
				fread(&irmaonode, sizeof(struct __unionnode__), 1, stream);
				if (filhonode.no_interno.numero_elem + irmaonode.no_interno.numero_elem <= (2*M - 1)) {

					/* Aloca e busca em arquivo os vetores de índices dos nós correspondentes */
					void* vet_ind_pai = (void*) malloc(n_key * (2*M - 1));
					void* vet_ind_filho = (void*) malloc(n_key * (2*M - 1));
					void* vet_ind_irmao = (void*) malloc(n_key * (2*M - 1));
					fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
					fread(vet_ind_pai, n_key, (2*M - 1), ind_stream);
					fseek(ind_stream, (n_key * (2*M - 1)) * irmaonode.no_interno.keys, SEEK_SET);
					fread(vet_ind_irmao, n_key, (2*M - 1), ind_stream);
					fseek(ind_stream, (n_key * (2*M - 1)) * filhonode.no_interno.keys, SEEK_SET);
					fread(vet_ind_filho, n_key, (2*M - 1), ind_stream);

					/* Atribui o elemento i ao ultimo elemento do irmao */
					functions_atrib[0](vet_ind_irmao + irmaonode.no_interno.numero_elem*n_key, vet_ind_pai + i*n_key, n_key, n_key);
					irmaonode.no_interno.numero_elem++;
                    int k;

					/*Joga todos elementos do filho para o irmao*/
					int j = irmaonode.no_interno.numero_elem;
					for (k = 0; j < 2*M && k < filhonode.no_interno.numero_elem; ++k, ++j) {
						functions_atrib[0](vet_ind_irmao + j*n_key, vet_ind_filho + k*n_key, n_key, n_key);
						irmaonode.no_interno.filhos[j+1] = filhonode.no_interno.filhos[k+1];
					}
					irmaonode.no_interno.numero_elem += filhonode.no_interno.numero_elem;

					/* Atualiza os ponteiros do pai */
					k = i;
					for (; k < painode.no_interno.numero_elem-1; ++k)
						functions_atrib[0](vet_ind_pai + k*n_key, vet_ind_pai + (k+1)*n_key, n_key, n_key);

					k = i+1;
					for (; k < painode.no_interno.numero_elem; ++k) {
						painode.no_interno.filhos[k] = painode.no_interno.filhos[k+1];
						painode.no_interno.filhos[k+1] = -1;
					}
					painode.no_interno.numero_elem--;

					/* Zera a quantidade de elementos do no removido para sua posição ser incluída na lista de posições livres*/
					filhonode.no_interno.numero_elem = 0;

					/* Grava as modificações feitas nos índices*/
					fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
					fwrite(vet_ind_pai, n_key, (2*M - 1), ind_stream);
					fseek(ind_stream, (n_key * (2*M - 1)) * irmaonode.no_interno.keys, SEEK_SET);
					fwrite(vet_ind_irmao, n_key, (2*M - 1), ind_stream);
					fseek(ind_stream, (n_key * (2*M - 1)) * filhonode.no_interno.keys, SEEK_SET);
					fwrite(vet_ind_filho, n_key, (2*M - 1), ind_stream);

					/* Atualiza os nós em arquivo */
					fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i-1], SEEK_SET);
					fwrite(&irmaonode, sizeof(struct __unionnode__), 1, stream);
					fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i], SEEK_SET);
					fwrite(&filhonode, sizeof(struct __unionnode__), 1, stream);
					fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
					fwrite(&painode, sizeof(struct __unionnode__), 1, stream);

					free(vet_ind_filho);
					free(vet_ind_irmao);
					free(vet_ind_pai);
					return;
                }
			}
		} else
			/** Nó folha */
		{
		    /* Verifica se o irmão da esquerda possui um elemento para emprestar */
			if (i) {

				struct __unionnode__ irmaonode;
				fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i-1], SEEK_SET);
				fread(&irmaonode, sizeof(struct __unionnode__), 1, stream);
				if (irmaonode.no_folha.numero_elem > M-1) {

					/* Aloca e recupera do arquivo os vetores dos nós correspondentes */
					void* vet_reg_irmao = (void*) malloc(n_record * (2*M - 1));
					void* vet_reg_filho = (void*) malloc(n_record * (2*M - 1));
					void* vet_ind_pai = (void*) malloc(n_key * (2*M - 1));
					fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
					fread(vet_ind_pai, n_key, (2*M - 1), ind_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * irmaonode.no_folha.records, SEEK_SET);
					fread(vet_reg_irmao, n_record, (2*M - 1), regs_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * filhonode.no_folha.records, SEEK_SET);
					fread(vet_reg_filho, n_record, (2*M - 1), regs_stream);

					/* Atualiza a chave do pai */
					functions_atrib[2](vet_ind_pai + (i-1)*n_key, vet_reg_irmao + (irmaonode.no_folha.numero_elem-1)*n_record, n_key, n_record);

					/* Abre um espaço para inserir o empréstimo*/
					int j = filhonode.no_folha.numero_elem;
					for (; j > 0; --j)
						functions_atrib[1](vet_reg_filho + j*n_record, vet_reg_filho + (j-1)*n_record, n_record, n_record);
					filhonode.no_folha.numero_elem++;

					/* Coloca o empréstimo na primeira posição do nó violado */
					functions_atrib[1](vet_reg_filho, vet_reg_irmao + (irmaonode.no_folha.numero_elem-1)*n_record, n_record, n_record);
					irmaonode.no_folha.numero_elem--;

					/* Grava as modificações feitas nos registros*/
					fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
					fwrite(vet_ind_pai, n_key, (2*M - 1), ind_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * irmaonode.no_folha.records, SEEK_SET);
					fwrite(vet_reg_irmao, n_record, (2*M - 1), regs_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * filhonode.no_folha.records, SEEK_SET);
					fwrite(vet_reg_filho, n_record, (2*M - 1), regs_stream);

					/* Atualiza os nós em arquivo */
					fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i-1], SEEK_SET);
					fwrite(&irmaonode, sizeof(struct __unionnode__), 1, stream);
					fseek(stream, sizeof(struct __unionnode__) * pos_filho, SEEK_SET);
					fwrite(&filhonode, sizeof(struct __unionnode__), 1, stream);
					fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
					fwrite(&painode, sizeof(struct __unionnode__), 1, stream);

					free(vet_reg_filho);
					free(vet_reg_irmao);
					free(vet_ind_pai);
					return;
				}
			}

			/* Verifica se o irmão da direita possui um elemento para emprestar */
			if (i != painode.no_interno.numero_elem) {
				/* Recupera o irmao da direita */
				struct __unionnode__ irmaonode;
				fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i+1], SEEK_SET);
				fread(&irmaonode, sizeof(struct __unionnode__), 1, stream);

				if (irmaonode.no_folha.numero_elem > M-1) {
        			/* Aloca e recupera do arquivo os vetores dos nós correspondentes */
					void* vet_reg_irmao = (void*) malloc(n_record * (2*M - 1));
					void* vet_reg_filho = (void*) malloc(n_record * (2*M - 1));
					void* vet_ind_pai = (void*) malloc(n_key * (2*M - 1));
					fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
					fread(vet_ind_pai, n_key, (2*M - 1), ind_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * irmaonode.no_folha.records, SEEK_SET);
					fread(vet_reg_irmao, n_record, (2*M - 1), regs_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * filhonode.no_folha.records, SEEK_SET);
					fread(vet_reg_filho, n_record, (2*M - 1), regs_stream);

					void* copy = (void*) malloc(n_record);

					functions_atrib[1](copy, vet_reg_irmao, n_record, n_record);

					/* Retira o primeiro elemento do irmao (o que foi emprestado) */
					int j = 0;
					for (; j < irmaonode.no_folha.numero_elem; ++j)
						functions_atrib[1](vet_reg_irmao + j*n_record, vet_reg_irmao + (j+1)*n_record, n_record, n_record);
					irmaonode.no_folha.numero_elem--;

					/* Atribui o primeiro elemento (emprestado e armazenado em copy) ao ultimo elemento do nó violado*/
					functions_atrib[1](vet_reg_filho + filhonode.no_folha.numero_elem*n_record, copy, n_record, n_record);
					functions_atrib[2](vet_ind_pai + i*n_key, vet_reg_irmao, n_key, n_record);
					filhonode.no_folha.numero_elem++;

					/* Grava as modificações feitas nos registros*/
					fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
					fwrite(vet_ind_pai, n_key, (2*M - 1), ind_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * irmaonode.no_folha.records, SEEK_SET);
					fwrite(vet_reg_irmao, n_record, (2*M - 1), regs_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * filhonode.no_folha.records, SEEK_SET);
					fwrite(vet_reg_filho, n_record, (2*M - 1), regs_stream);

					/* Atualiza os nós em arquivo */
					fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i+1], SEEK_SET);
					fwrite(&irmaonode, sizeof(struct __unionnode__), 1, stream);
					fseek(stream, sizeof(struct __unionnode__) * pos_filho, SEEK_SET);
					fwrite(&filhonode, sizeof(struct __unionnode__), 1, stream);
					fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
					fwrite(&painode, sizeof(struct __unionnode__), 1, stream);

					free(vet_reg_filho);
					free(vet_reg_irmao);
					free(vet_ind_pai);
					return;
				}
			}


			if (i == painode.no_interno.numero_elem)
				i--;

            fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i], SEEK_SET);
            fread(&filhonode, sizeof(struct __unionnode__), 1, stream);

			struct __unionnode__ irmaonode;
			fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i+1], SEEK_SET);
			fread(&irmaonode, sizeof(struct __unionnode__), 1, stream);

			/* Verifica se é possível concatenar elementos com o irmao da direita */
			if (filhonode.no_folha.numero_elem + irmaonode.no_folha.numero_elem <= 2*M-1) {
                /* Aloca e recupera do arquivo os vetores dos nós correspondentes */
				void* vet_reg_irmao = (void*) malloc(n_record * (2*M - 1));
				void* vet_reg_filho = (void*) malloc(n_record * (2*M - 1));
				void* vet_ind_pai = (void*) malloc(n_key * (2*M - 1));
				fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
				fread(vet_ind_pai, n_key, (2*M - 1), ind_stream);
				fseek(regs_stream, (n_record * (2*M - 1)) * irmaonode.no_folha.records, SEEK_SET);
				fread(vet_reg_irmao, n_record, (2*M - 1), regs_stream);
				fseek(regs_stream, (n_record * (2*M - 1)) * filhonode.no_folha.records, SEEK_SET);
				fread(vet_reg_filho, n_record, (2*M - 1), regs_stream);

				/* Coloca todos elementos do irmao no nó filho*/
				int j = filhonode.no_folha.numero_elem;
				int k;
				for (k = 0; j < 2*M && k < irmaonode.no_folha.numero_elem; ++k, ++j)
					functions_atrib[1](vet_reg_filho + j*n_record, vet_reg_irmao + k*n_record, n_record, n_record);
				filhonode.no_folha.numero_elem += irmaonode.no_folha.numero_elem;

				/* Atualiza os ponteiros de prox e anterior */
				filhonode.no_folha.prox = irmaonode.no_folha.prox;
				if (filhonode.no_folha.prox != -1) {
					struct __unionnode__ auxnode;
					/* Lê o prox e faz com que o mesmo aponte para o filho */
					fseek(stream, sizeof(struct __unionnode__) * filhonode.no_folha.prox, SEEK_SET);
					fread(&auxnode, sizeof(struct __unionnode__), 1, stream);
					auxnode.no_folha.ant = pos_filho;
					fseek(stream, sizeof(struct __unionnode__) * filhonode.no_folha.prox, SEEK_SET);
					fwrite(&auxnode, sizeof(struct __unionnode__), 1, stream);
				}

				/* Atualiza os ponteiros do pai */
            	k = i;
				for (; k < painode.no_interno.numero_elem-1; ++k)
					functions_atrib[0](vet_ind_pai + k*n_key, vet_ind_pai + (k+1)*n_key, n_key, n_key);

                k = i+1;
				for (; k < painode.no_interno.numero_elem; ++k) {
					painode.no_interno.filhos[k] = painode.no_interno.filhos[k+1];
					painode.no_interno.filhos[k+1] = 0;
				}
				painode.no_interno.numero_elem--;

				/* Zera a quantidade de elementos do no removido para sua posição ser incluída na lista de posições livres */
				irmaonode.no_folha.numero_elem = 0;


				/* Grava as modificações feitas nos registros*/
				fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
				fwrite(vet_ind_pai, n_key, (2*M - 1), ind_stream);
				fseek(regs_stream, (n_record * (2*M - 1)) * irmaonode.no_folha.records, SEEK_SET);
				fwrite(vet_reg_irmao, n_record, (2*M - 1), regs_stream);
				fseek(regs_stream, (n_record * (2*M - 1)) * filhonode.no_folha.records, SEEK_SET);
				fwrite(vet_reg_filho, n_record, (2*M - 1), regs_stream);

				/* Atualiza os nós em arquivo */
				fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i+1], SEEK_SET);
				fwrite(&irmaonode, sizeof(struct __unionnode__), 1, stream);
				fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i], SEEK_SET);
				fwrite(&filhonode, sizeof(struct __unionnode__), 1, stream);
				fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
				fwrite(&painode, sizeof(struct __unionnode__), 1, stream);

				free(vet_reg_filho);
				free(vet_reg_irmao);
				free(vet_ind_pai);
				return;


			} else if (i) {
				/* Recupera o irmao esquerdo */
				fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i-1], SEEK_SET);
				fread(&irmaonode, sizeof(struct __unionnode__), 1, stream);

				/* Verifica se a junção do nó esquerdo e do filho podem formar um nó */
				if (irmaonode.no_folha.numero_elem + filhonode.no_folha.numero_elem <= (2*M - 1)) {

					/* Aloca e recupera do arquivo os vetores dos nós correspondentes */
					void* vet_reg_irmao = (void*) malloc(n_record * (2*M - 1));
					void* vet_reg_filho = (void*) malloc(n_record * (2*M - 1));
					void* vet_ind_pai = (void*) malloc(n_key * (2*M - 1));
					fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
					fread(vet_ind_pai, n_key, (2*M - 1), ind_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * irmaonode.no_folha.records, SEEK_SET);
					fread(vet_reg_irmao, n_record, (2*M - 1), regs_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * filhonode.no_folha.records, SEEK_SET);
					fread(vet_reg_filho, n_record, (2*M - 1), regs_stream);

					/* Coloca todos elementos do filho no irmao esquerdo*/
					int j = irmaonode.no_folha.numero_elem;
					int k;
					for (k = 0; j < 2*M && k < filhonode.no_folha.numero_elem; ++k, ++j)
						functions_atrib[1](vet_reg_irmao + j*n_record, vet_reg_filho + k*n_record, n_record, n_record);
					irmaonode.no_folha.numero_elem += filhonode.no_folha.numero_elem;

					/* Atualiza os ponteiros de prox e anterior */
					irmaonode.no_folha.prox = filhonode.no_folha.prox;
					if (irmaonode.no_folha.prox != -1) {
						struct __unionnode__ auxnode;
						/* Lê o prox e faz com que o mesmo aponte para o filho */
						fseek(stream, sizeof(struct __unionnode__) * irmaonode.no_folha.prox, SEEK_SET);
						fread(&auxnode, sizeof(struct __unionnode__), 1, stream);
						auxnode.no_folha.ant = painode.no_interno.filhos[i-1];
						fseek(stream, sizeof(struct __unionnode__) * irmaonode.no_folha.prox, SEEK_SET);
						fwrite(&auxnode, sizeof(struct __unionnode__), 1, stream);
					}

					/* Atualiza os ponteiros do pai */
	            	k = i;
					for (; k < painode.no_interno.numero_elem-1; ++k)
						functions_atrib[0](vet_ind_pai + k*n_key, vet_ind_pai + (k+1)*n_key, n_key, n_key);

	                k = i+1;
					for (; k < painode.no_interno.numero_elem; ++k) {
						painode.no_interno.filhos[k] = painode.no_interno.filhos[k+1];
						painode.no_interno.filhos[k+1] = 0;
					}
					painode.no_interno.numero_elem--;

					/* Zera a quantidade de elementos do no removido para sua posição ser incluída na lista de posições livres */
					irmaonode.no_folha.numero_elem = 0;


					/* Grava as modificações feitas nos registros*/
					fseek(ind_stream, (n_key * (2*M - 1)) * painode.no_interno.keys, SEEK_SET);
					fwrite(vet_ind_pai, n_key, (2*M - 1), ind_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * irmaonode.no_folha.records, SEEK_SET);
					fwrite(vet_reg_irmao, n_record, (2*M - 1), regs_stream);
					fseek(regs_stream, (n_record * (2*M - 1)) * filhonode.no_folha.records, SEEK_SET);
					fwrite(vet_reg_filho, n_record, (2*M - 1), regs_stream);

					/* Atualiza os nós em arquivo */
					fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i-1], SEEK_SET);
					fwrite(&irmaonode, sizeof(struct __unionnode__), 1, stream);
					fseek(stream, sizeof(struct __unionnode__) * painode.no_interno.filhos[i], SEEK_SET);
					fwrite(&filhonode, sizeof(struct __unionnode__), 1, stream);
					fseek(stream, sizeof(struct __unionnode__) * pos_stream_root, SEEK_SET);
					fwrite(&painode, sizeof(struct __unionnode__), 1, stream);

					free(vet_reg_filho);
					free(vet_reg_irmao);
					free(vet_ind_pai);

                }
            } // if verifica com esquerdo
        } // else

    }
}

void gera_indice(struct __bplusnode__* root, char* file_name, int (*functions_comp[])(const void*, const void*),
	void (*functions_atrib[])(void*, void*, size_t, size_t), size_t n_key, size_t n_record)
{
	/* Aloca espaço para conter o registro e a chave que será criada */
	void* rec = (void*) malloc(n_record);
	void* key = (void*) malloc(n_key);

    FILE* arquivo_elementos = fopen(strcat(file_name, "_elementos.dat"), "rb");
    if (!arquivo_elementos)
        perror("Nao foi possivel criar o arquivo: ");

	while (true) {
		fread(rec, n_record, 1, arquivo_elementos);
		/* Verifica eventuais erros na leitura */
		if (feof(arquivo_elementos) || ferror(arquivo_elementos))
			break;
		/* Recupera a chave correspondente aquele registro */
		functions_atrib[2](key, rec, n_key, n_record);
		/* Insere o registro no arquivo da arvore b+ */
		insert_b(root, key, n_key, rec, n_record, functions_comp, functions_atrib);
	}

	fclose(arquivo_elementos);
}


void grava_root(struct __bplusnode__* root, char* file_name)
{
    char auxiliar[100];

    strcpy(auxiliar, file_name);

    FILE* pos_root_stream = fopen(strcat(file_name, "_pos_root_stream.dat"), "w+b");
    fwrite(&root->pos_stream_root, sizeof(int), 1, pos_root_stream);
    fclose(pos_root_stream);

    strcpy(file_name, auxiliar);
    FILE* pos_regs_free_stream = fopen(strcat(file_name, "_pos_regs_free_stream.dat"), "w+b");
    while (root->free_pos_regs) {
        fwrite(&root->free_pos_regs->position, sizeof(int), 1, pos_regs_free_stream);
        root->free_pos_regs = root->free_pos_regs->prox;
    }
    fclose(pos_regs_free_stream);

    strcpy(file_name, auxiliar);
    FILE* pos_ind_free_stream = fopen(strcat(file_name, "_pos_ind_free_stream.dat"), "w+b");
    while (root->free_pos_ind) {
        fwrite(&root->free_pos_ind->position, sizeof(int), 1, pos_ind_free_stream);
        root->free_pos_ind = root->free_pos_ind->prox;
    }
    fclose(pos_ind_free_stream);

    strcpy(file_name, auxiliar);
    FILE* pos_node_free_stream =  fopen(strcat(file_name, "_pos_node_free_stream.dat"), "w+b");
    while (root->free_pos_stream) {
        fwrite(&root->free_pos_stream->position, sizeof(int), 1, pos_node_free_stream);
        root->free_pos_stream =  root->free_pos_stream->prox;
    }
    fclose(pos_node_free_stream);

    fclose(root->ind_stream);
    fclose(root->stream);
    fclose(root->regs_stream);
}
