#include "intercalacao.h"
#include "funcionarios.h"
#include "arvore_binaria.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

void intercalacao_basico(char *nome_arquivo_saida, int num_p, Lista *nome_particoes) {

    int fim = 0; //variavel que controla fim do procedimento
    FILE *out; //declara ponteiro para arquivo

    //abre arquivo de saida para escrita
    if ((out = fopen(nome_arquivo_saida, "wb")) == NULL) {
        printf("Erro ao abrir arquivo de sa?da\n");
    } else {
        //cria vetor de particoes
        TVet v[num_p];

        //abre arquivos das particoes, colocando variavel de arquivo no campo f do vetor
        //e primeiro funcionario do arquivo no campo func do vetor
        for (int i=0; i < num_p; i++) {
            v[i].f = fopen(nome_particoes->nome, "rb");
            v[i].aux_p = 0;
            if (v[i].f != NULL) {
                fseek(v[i].f, v[i].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[i].f);
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posi??o do vetor
                    v[i].func = funcionario(INT_MAX, "","","",0);
                }
                else {
                    //conseguiu ler funcionario, coloca na posi??o atual do vetor
                    v[i].func = f;
                }
            }
            else {
                fim = 1;
            }
            nome_particoes = nome_particoes->prox;
        }

        int aux = 0;
        while (!(fim)) { //conseguiu abrir todos os arquivos
            int menor = INT_MAX;
            int pos_menor;
            //encontra o funcionario com menor chave no vetor
            for(int i = 0; i < num_p; i++){
                if(v[i].func->cod < menor){
                    menor = v[i].func->cod;
                    pos_menor = i;
                }
            }
            if (menor == INT_MAX) {
                fim = 1; //terminou processamento
            }
            else {
                //salva funcionario no arquivo de saída
                fseek(out, aux * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[pos_menor].func, out);
                //atualiza posição pos_menor do vetor com pr?ximo funcionario do arquivo
                v[pos_menor].aux_p++;
                fseek(v[pos_menor].f, v[pos_menor].aux_p * tamanho_registro(), SEEK_SET);
                TFunc *f = le_funcionario(v[pos_menor].f);
                aux++;
                if (f == NULL) {
                    //arquivo estava vazio
                    //coloca HIGH VALUE nessa posiçao do vetor
                    v[pos_menor].func = funcionario(INT_MAX, "", "", "",0.0);
                }
                else {
                    v[pos_menor].func = f;
                }

            }
        }

        //fecha arquivos das partiÇões de entrada
        for(int i = 0; i < num_p; i++){
            fclose(v[i].f);
        //    free(v[i].func);
        }
        //fecha arquivo de saída
        fclose(out);
    }
}

void intercalacao_arvore_de_vencedores(TPilha **pilha, int *vetTop, char *nome_arquivo_saida, int num_p){

    FILE *saida =  fopen(nome_arquivo_saida, "wb");
    if(!saida)
    {
        printf("ARQUIVO DE SAIDA INVALIDO\n");
        return;
    }

    int nParticoes = 10;
    int nBase = nParticoes;
    int i;
    TNoA** base = malloc(nBase * sizeof(TNoA*));
    TFunc **hash = malloc(num_p * sizeof(TFunc*));

    for(i = 0 ; i < nParticoes ; i++) {
        TFunc *f = pop(pilha[i], 0, &vetTop[i]);
        hash[f->cod] = f;
        TNoA *no = criaNo_arvore_binaria(f->cod, -1);
        no->pos = i;
        base[i] = no;
    }

    TNoA *vencedor;
    TNoA **pai1 = malloc(nBase * sizeof(TNoA*));
    TNoA **pai2 = malloc(nBase * sizeof(TNoA*));
    int ciclo = 0;
    int nPai1 = 0;
    int nPai2 = 0;
    int nArqSaida;

    while(1) {
        TNoA **atual = base;
        int nCiclo = nBase;
        if(ciclo != 0) {
            atual = base;
            nCiclo = nPai1;
        }

        for(i = 0 ; i < nCiclo ; i += 2) {
            if(nCiclo == i+1) {
                TNoA *no = criaNo_arvore_binaria(atual[i]->info, -1);
                no->pos = atual[i]->pos;
                no->esq = atual[i];
                no->dir = NULL;
                no->esq->ant = no;
                if(ciclo == 0) {
                    pai1[nPai1] = no;
                    nPai1++;
                }
                else {
                    pai2[nPai2] = no;
                    nPai2++;
                }
            }
            if(atual[i]->info < atual[i+1]->info) {
                TNoA *no = criaNo_arvore_binaria(atual[i]->info, -1);
                no->pos = atual[i]->pos;
                no->esq = atual[i];
                no->dir = atual[i+1];
                no->esq->ant = no;
                no->dir->ant = no;
                if(ciclo == 0) {
                    pai1[nPai1] = no;
                    nPai1++;
                }
                else {
                    pai2[nPai2] = no;
                    nPai2++;
                }
            }
            else {
                TNoA *no = criaNo_arvore_binaria(atual[i+1]->info, -1);
                no->pos = atual[i+1]->pos;
                no->esq = atual[i];
                no->dir = atual[i+1];
                no->esq->ant = no;
                no->dir->ant = no;
                if(ciclo == 0) {
                    pai1[nPai1] = no;
                    nPai1++;
                }
                else {
                    pai2[nPai2] = no;
                    nPai2++;
                }
            }
        }
        int aux = nPai1;
        if(ciclo != 0)
            aux = nPai2;

        if(aux == 1) {
            vencedor = pai1[0];
            if(ciclo != 0)
                vencedor == pai2[0];
            break;
        }

        if(ciclo == 0)
            ciclo++;

        for(i = 0 ; i<nPai2; i++) {
            pai1[i] = pai2[i];
        }
        nPai1 = nPai2;
        nPai2 = 0;
        ciclo++;
    }

    TNoA *arv = vencedor;

    while(arv->info != INT_MAX) {
        int infoVenc = vencedor->info;

        while(arv->esq != NULL || arv->dir != NULL) {

            if(arv->esq != NULL && arv->esq->info == infoVenc)
                arv = arv->esq;

            if(arv->dir != NULL && arv->dir->info == infoVenc)
                arv = arv->dir;
        }

        TFunc *f = hash[infoVenc];
        fseek(saida, nArqSaida * tamanho_registro(), SEEK_SET);
        nArqSaida++;
        salva_funcionario(f, saida);

        int posicaoVenc = vencedor->pos;
        f = pop(pilha[posicaoVenc], 0, &vetTop[posicaoVenc]);

        if(f) {
            arv->info = f->cod;
            hash[f->cod] = f;
        }
        else
            arv->info = INT_MAX;

        while(arv->ant) {
            arv = arv->ant;
            if(arv->esq && arv->dir) {
                if(arv->esq->info < arv->dir->info) {
                    arv->info = arv->esq->info;
                    arv->pos = arv->esq->pos;
                }
                else {
                    arv->info = arv->dir->info;
                    arv->pos = arv->dir->pos;
                }
            }
            else {
                if(arv->esq) {
                    arv->info = arv->esq->info;
                    arv->pos = arv->esq->pos;
                }
                if(arv->dir)
                    arv->info = arv->dir->info;
                    arv->pos = arv->dir->pos;
            }
        }
    }
    free(base);
    free(hash);
    free(pai1);
    free(pai2);
}



