#include "funcionarios.h"
#include "particoes.h"
#include <stdlib.h>
#include <limits.h>

#include "particoes.h"

TFunc* pegaFunc(FILE* arq, int* lidos)
{
    fseek(arq, *lidos * tamanho_registro(), SEEK_SET);
    TFunc* funcionario = le_funcionario(arq);
    (*lidos)++;
    return funcionario;
}

int moveParaInicio(TFunc* vFunc[], int M, int menor)
{
    TFunc* auxFunc;
    if(menor == 0)
        return 0;
    else
    {
        for(int i = M-1 ; i > 0 ; i--)
        {
            if(i == menor)
            {
                auxFunc = vFunc[menor];
                vFunc[i] = vFunc[i-1];
                vFunc[i-1] = auxFunc;
                menor--;
            }
        }
    }
    return menor;
}

int elementosVetor(int ctrl[], int tam)
{
    int aux = 0;
    for(int i = 0 ; i < tam ; i++)
    {
        if(ctrl[i] == 0)
            aux++;
    }
    return aux;
}

int encontraMenor(TFunc* vFunc[], int tam)
{
    int menor = 0;
    for(int i = 1 ; i < tam ; i++)
    {
        if(vFunc[menor]->cod > vFunc[i]->cod)
            menor = i;
    }
    return menor;
}

void attNomeParticao(Lista* nomesF, int* numPrtc)
{
    if(nomesF->prox == NULL)
    {
        char* novoNome = malloc(5 * sizeof(char));
        (*numPrtc)++;
        sprintf(novoNome, "p%d.dat", *numPrtc);
        nomesF->prox = cria(novoNome, NULL);
    }
}

int temElemento(TFunc *vFunc[], int tamanho)
{
    for (int j=0; j<tamanho; j++)
    {
        if(vFunc[j] != NULL)
        {
            return 1;
        }
    }
    return 0;
}


void classificacao_interna(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc)
{
    rewind(arq); //posiciona cursor no inicio do arquivo


    int reg = 0;

    while (reg != nFunc)
    {
        //le o arquivo e coloca no vetor
        TFunc *v[M];
        int i = 0;
        while (!feof(arq))
        {
            fseek(arq, (reg) * tamanho_registro(), SEEK_SET);
            v[i] = le_funcionario(arq);
            //     imprime_funcionario(v[i]);
            i++;
            reg++;
            if(i>=M) break;
        }

        //ajusta tamanho M caso arquivo de entrada tenha terminado antes do vetor
        if (i != M)
        {
            M = i;
        }

        //faz ordenacao
        for (int j = 1; j < M; j++)
        {
            TFunc *f = v[j];
            i = j - 1;
            while ((i >= 0) && (v[i]->cod > f->cod))
            {
                v[i + 1] = v[i];
                i = i - 1;
            }
            v[i + 1] = f;
        }

        //cria arquivo de particao e faz gravacao
        char *nome_particao = nome_arquivos_saida->nome;
        nome_arquivos_saida = nome_arquivos_saida->prox;
        printf("\n%s\n", nome_particao);
        FILE *p;
        if ((p = fopen(nome_particao, "wb+")) == NULL)
        {
            printf("Erro criar arquivo de saida\n");
        }
        else
        {
            for (int i = 0; i < M; i++)
            {
                fseek(p, (i) * tamanho_registro(), SEEK_SET);
                salva_funcionario(v[i], p);
                imprime_funcionario(v[i]);
            }
            fclose(p);
        }
        for(int jj = 0; jj<M; jj++)
            free(v[jj]);
    }
}

void selecao_natural(FILE *arq, Lista *nome_arquivos_saida, int M, int nFunc, int n)
{
    int nPrtc = 10;
    Lista* nomesSaida = nome_arquivos_saida;
    Lista* nomePrtc = nomesSaida;
    FILE* prtc = NULL;
    int prtcCursor = 0;
    int totalLidos = 0;

    TFunc** vFunc = malloc(M * sizeof(TFunc*));

    FILE* reserv = fopen("reservatorio.dat", "w+");
    int reservCursor = 0;
    int reservCheio = 0;

    int ctrlVet[M];
    int i = 0;

    while(i < M && !feof(arq))
    {
        vFunc[i] = pegaFunc(arq, &totalLidos);
        ctrlVet[i] = 0;
        i++;
    }

    if(prtc == NULL)
    {
        char* nome = nomesSaida->nome;
        prtc = fopen(nome, "w+b");
        nomePrtc = nomesSaida;

        attNomeParticao(nomesSaida, nPrtc);
        nomesSaida = nomesSaida->prox;
    }

    while (temElemento(vFunc, M) && totalLidos <= nFunc)
    {
        reservCheio = 0;
        while(!feof(arq) && reservCheio == 0) {

            if(prtc == NULL){
                char* nome = nomesSaida->nome;
                prtc = fopen(nome, "w+b");
                nomePrtc = nomesSaida;

                attNomeParticao(nomesSaida, nPrtc);
                nomesSaida = nomesSaida->prox;
            }

            if(prtcCursor >= M)
            {
                fclose(prtc);
                prtc = NULL;
                prtcCursor = 0;
                continue;
            }

            int menor = encontraMenor(vFunc, M);
            menor = moveParaInicio(vFunc, M, menor);

            fseek(prtc, prtcCursor * tamanho_registro(), SEEK_SET);
            salva_funcionario(vFunc[menor], prtc);

            int ultimoCod = vFunc[menor]->cod;
            prtcCursor++;
            ctrlVet[menor] = 1;

            while(ctrlVet[menor] && reservCheio == 0 && !feof(arq) && totalLidos < nFunc)
            {
                if(feof(arq))
                {
                    fseek(reserv, reservCursor * tamanho_registro(), SEEK_SET);
                    salva_funcionario(vFunc[menor], reserv);
                    reservCursor++;
                    if(reservCursor == n)
                        reservCheio = 1;
                }
                else
                {
                    vFunc[menor] = pegaFunc(arq, &totalLidos);
                    if(vFunc[menor]->cod >= ultimoCod)
                    {
                        if(prtcCursor >= M)
                        {
                            fclose(prtc);
                            prtc = NULL;
                            prtcCursor = 0;

                            char* nome = nomesSaida->nome;
                            prtc = fopen(nome, "w+b");
                            nomePrtc;

                            attNomeParticao(nomesSaida, nPrtc);
                            nomesSaida = nomesSaida->prox;
                        }
                        fseek(prtc, prtcCursor * tamanho_registro(), SEEK_SET);
                        salva_funcionario(vFunc[menor], prtc);
                        ctrlVet[menor] = 0;
                    }
                }
            }
        }
    }
    if(totalLidos == nFunc)
    {
        fclose(prtc);
        prtc = NULL;
        prtcCursor = 0;

        char* nome = nomesSaida->nome;
        prtc = fopen(nome, "w+b");
        nomePrtc = nomesSaida;

        for(i = 0 ; i < M ; i++) {
            fseek(prtc, i * tamanho_registro(), SEEK_SET);
            salva_funcionario(vFunc[i], prtc);
        }
        prtcCursor = 0;
        reservCursor = 0;
    }

    if(prtcCursor >= M) {
        fclose(prtc);
        prtc = NULL;
        prtcCursor = 0;

        char* nome = nomesSaida->nome;
        prtc = fopen(nome, "w+b");
        nomePrtc = nomesSaida;

        attNomeParticao(nomesSaida, nPrtc);
        nomesSaida = nomesSaida->prox;
    }

    if (reservCursor > 0) {
        for(i = 0 ; i < reservCursor ; i++) {
            fseek(reserv, i * tamanho_registro(), SEEK_SET);
            vFunc[i] = le_funcionario(reserv);
            ctrlVet[i] = 0;
            reservCursor--;
        }
    }

    int count = elementosVetor(ctrlVet, M);

    if(count != M && !feof(arq)) {
        while(count < M && !feof(arq)) {
            vFunc[count] = pegaFunc(arq, &totalLidos);
            ctrlVet[count] = 0;
            count++;
        }
    }

    for(i = 0 ; i < M; i++) {
        free(vFunc[i]);
    }
    fclose(prtc);
    free(vFunc);
}
