/***

	IMPLEMENTAÇÃO B+ EM ARQUIVO
	CODIFICADOR: WILSON ESTÉCIO MARCÍLIO JÚNIOR

*/

#ifndef BMAISSTREAM_INCLUDED
#define BMAISSTREAM_INCLUDED

#define M 5

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <windows.h>


struct elementos
{
    int meuInteiro;
};

struct __nodeind__
{
	int keys;
	int numero_elem;
	int filhos[2*M];
};

struct __nodefol__
{
	int numero_elem;
	int records;
	int prox;
	int ant;
};

struct __unionnode__
{
	bool interno;
	union
	{
		struct __nodeind__ no_interno;
		struct __nodefol__ no_folha;
	};
};

struct __pilha__
{
	int position;
	struct __pilha__* prox;
};


struct __bplusnode__
{
	FILE* stream;
	FILE* regs_stream;
	FILE* ind_stream;
	int pos_stream_root;
	struct __pilha__* free_pos_stream;
	struct __pilha__* free_pos_ind;
	struct __pilha__* free_pos_regs;
};

void remover(struct __bplusnode__* root, void* key, size_t n_key, size_t n_record, int (*functions_comp[])(const void*, const void*),
    void (*functions_atrib[])(void*, void*, size_t, size_t));

void insert_b(struct __bplusnode__* root, void* key, size_t n_key, void* record, size_t n_record,
		int (*functions_comp[])(const void*, const void*), void (*functions_atrib[])(void*, void*, size_t, size_t));

void gera_indice(struct __bplusnode__* root, char* file_name, int (*functions_comp[])(const void*, const void*),
	void (*functions_atrib[])(void*, void*, size_t, size_t), size_t n_key, size_t n_record);

void grava_root(struct __bplusnode__* root, char* file_name);

void init_btree(struct __bplusnode__* root, char* file_name);

void find_free_positions(struct __bplusnode__* root);

bool search_b(struct __bplusnode__ root, void* key, int (*functions_comp[])(const void*, const void*), size_t n_key, size_t n_record);

bool search_aux(FILE* node_stream, FILE* regs_stream, FILE* ind_stream, int pos, void* key,
	int (*functions_comp[])(const void*, const void*), size_t n_key, size_t n_record);

void init_bplus(struct __bplusnode__* root, void* record, size_t n_record, void (*functions_atrib[])(void*, void*, size_t, size_t));



void insert_aux(FILE* stream, FILE* ind_stream, FILE* regs_stream, int pos_stream_root, void* key,
	size_t n_key, void* record, size_t n_record, int (*functions_comp[])(const void*, const void*),
	void (*functions_atrib[])(void*, void*, size_t, size_t), bool* flag, struct __pilha__** free_pos_stream,
	struct __pilha__** free_pos_ind, struct __pilha__** free_pos_regs);

void divide_bmais(FILE* stream, FILE* regs_stream, FILE* ind_stream, int pos_stream_root,
	int pos_filho, int ind, void (*functions_atrib[])(void*, void*, size_t, size_t),
	struct __pilha__** free_pos_stream, struct __pilha__** free_pos_ind, struct __pilha__** free_pos_regs,
	size_t n_key, size_t n_record);


void remove_aux(FILE* stream, FILE* ind_stream, FILE* regs_stream, int pos_stream_root, void* key, size_t n_key, size_t n_record,
	int (*functions_comp[])(const void*, const void*), void (*functions_atrib[])(void*, void*, size_t, size_t), bool* entra);

void fix(FILE* stream, FILE* ind_stream, FILE* regs_stream, int pos_stream_root, int pos_filho, int ind, size_t n_key, size_t n_record,
 	void (*functions_atrib[])(void*, void*, size_t, size_t), int (*functions_comp[])(const void*, const void*));

void push_list(struct __pilha__** p, int position);

void pop_list(struct __pilha__** p);



/**
*
*  functions_comp[](const void*, const void*):
*		< 0: primeiro menor
*		> 0: primeiro maior
*		= 0: iguais
*	 --> functions_comp[0](key, key)
*	 --> functions_comp[1](record, record)
*    --> functions_comp[2](record, key)

*
*  functions_atrib[](void*, void*, size_t, size_t):
*	 --> functions_atrib[0](key, key, size_t, size_t)
*    --> functions_atrib[1](record, record, size_t, size_t)
*    --> functions_atrib[2](key, record, size_t, size_t)
*
*
*/


#endif
