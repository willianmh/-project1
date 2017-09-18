#include "montador.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


int whichToken(char *palavra);

int isDefRotulo(char *palavra);

int isDiretiva(char *palavra);

int isInstrucao(char *palavra);

int isHexadecimal(char *palavra);

int isDecimal(char *palavra);

int isRotulo(char *palavra);

int isNome(char *palavra);

int isAtoF(char c);
int isArgumento(char *palavra);
void debbug(char *string);

typedef struct tipoEachWord {
    Token palavra;
    struct tipoEachWord *next;
} eachWord;
typedef eachWord *pointerLine;

int verificaGramatica(pointerLine eachLine, int fim);

pointerLine insereToken(pointerLine eachLine, char *palavra, unsigned int linha);
pointerLine apagaLista(pointerLine inicio);
void imprimeLista(pointerLine inicio);
/* 
 * Argumentos:
 *  entrada: cadeia de caracteres com o conteudo do arquivo de entrada.
 *  tamanho: tamanho da cadeia.
 * Retorna:
 *  1 caso haja erro na montagem; 
 *  0 caso nÃ£o haja erro.
 */
int processarEntrada(char *entrada, unsigned tamanho) {
    int i, j;
    char caracter;
    char palavra[1000];

    // armazena em um lista uma linha de entrada
    pointerLine eachLine;
    pointerLine word_token;

    unsigned linha;

    for (int k = 0; k < tamanho-1; k++) {
        printf(" %d", entrada[k]);
    }
    putchar('\n');

    printf("--------------------\ntamanho = %u | entrada :\n%s\n----------------\n", tamanho, entrada);
    tamanho--;

    // inicializacoes
    eachLine = NULL; // lista vazia
    linha = 1;
    j = 0;

    for (i = 0; i <= tamanho; i++) {
        caracter = entrada[i];
        if (caracter == '#'){
            do {
                i++;
                caracter = entrada[i];
            } while (i < tamanho && caracter != 10);
        }

        // se tem quebra linha ou arquivo acaba
        if(caracter == 10 || i >= tamanho){
            linha++;
            printf("linha: %d\n", linha);

            // coloca fim da palavra
            palavra[j] = '\0';

            // cria um token temporario e adiciona em uma lista
            if(strlen(palavra) > 0){
                eachLine = insereToken(eachLine, palavra, linha);
                if(eachLine == NULL){
                    fprintf(stderr, "ERRO LEXICO: palavra inválida na linha %d!", linha);
                    i = tamanho + 2;
                }
                if(!verificaGramatica(eachLine, 1)) {
                    fprintf(stderr, "ERRO GRAMATICAL: palavra na linha %d!", linha);
                    i = tamanho + 2;
                }
            }


            imprimeLista(eachLine);
            eachLine = apagaLista(eachLine);

            j = 0;
        // se continua a 'frase'
        } else {
            // se a palavra acabou
            if(caracter == 32){
                palavra[j] = '\0'; // coloca fim

                // cria um token temporario e adiciona em uma lista
                if(strlen(palavra) > 0){
                    eachLine = insereToken(eachLine, palavra, linha);
                    if(eachLine == NULL){
                        fprintf(stderr, "ERRO LEXICO: palavra inválida na linha %d!", linha);
                        i = tamanho + 2;
                    }
                    if(!verificaGramatica(eachLine, 0)) {
                        fprintf(stderr, "ERRO GRAMATICAL: palavra na linha %d!", linha);
                        i = tamanho + 2;
                    }
                }
                j = 0;
            } else {
                // monta a palavra letra por letra
                palavra[j] = caracter;
                j++;
            }
        }
    }

    return 0;
}
int verificaGramatica(pointerLine eachLine, int fim) {
    if (eachLine == NULL)
        return 1; // gramatica correta para conj. vazio

    if(eachLine->next == NULL && fim == 0)
        return 1; // por enquanto ok, espera pelo fim da linha

    pointerLine token;
    token = eachLine;
    TipoDoToken tipoToken;

    if(token->palavra.tipo == DefRotulo){
        // so defRotulo na linha
        if (token->next == NULL && fim ==1)
            return 1;

        tipoToken = token->next->palavra.tipo;
        if(tipoToken != Instrucao && tipoToken != Diretiva)
            return 0;

        token = token->next;
    }

    if(token->palavra.tipo == Instrucao){
        if(!strcmp(token->palavra.palavra, "LSH") || !strcmp(token->palavra.palavra, "RSH")|| !strcmp(token->palavra.palavra, "LOADmq")) {
            if(token->next == NULL) // essas inst nao recebem argumento depois
                return 1;

            // qualquer que seja a prox palavra, gera erro
            return 0;
        }
        if(token->next == NULL && fim == 0)
            return 1; // por enquanto ok, espera pelo fim da linha

        if(token->next == NULL)
            return 0; // deveria ter argumento

        // verifica se argumento valido
        tipoToken = token->next->palavra.tipo;
        if (tipoToken != Hexadecimal && tipoToken != Decimal && tipoToken != Nome)
            return 0;

        token = token->next;
        if(token->next != NULL)
            return 0; // nao deve ter mais palavras alem do argumento
        else
            return 1;
    }

    if(token->palavra.tipo == Diretiva){

        /*
         * Verifica Gramatica DIRETIVA .set
         */
        if(!strcmp(token->palavra.palavra, ".set")){
            if (token->next == NULL && fim == 0)
                return 1;   // por enquanto ok, espera pelo fim da linha

            // linha com apenas .set, sem argumentos
            if(token->next == NULL)
                return 0;  // dispara erro

            // .set com apenas um argumento
            if(token->next->next == NULL && fim == 1)
                return 0;  // dispara erro

            // primeiro argumento deve ser NOME
            tipoToken = token->next->palavra.tipo;
            if(tipoToken != Nome)
                return 0;  // dispara erro

            // valida o seguundo argumento
            if (token->next->next) {
                // segundo argumento deve ser HEX ou DEC
                tipoToken = token->next->next->palavra.tipo;
                if (tipoToken != Hexadecimal && tipoToken != Decimal)
                    return 0;  // dispara erro

                // nao deve ter nada alem dos dois argumentos
                if (token->next->next->next != NULL)
                    return 0;
                else
                    return 1;
            }
        }
        /*
         * Verifica Gramatica DIRETIVA .org
         */
        if(!strcmp(token->palavra.palavra, ".org")){
            if (token->next == NULL && fim == 0)
                return 1; // por enquanto ok, espera pelo fim da linha

            // linha com apenas .word, sem argumentos
            if(token->next == NULL)
                return 0;

            // argumento deve ser HEX ou DEC
            if(token->next->palavra.tipo != Decimal && token->next->palavra.tipo != Hexadecimal)
                return 0;

            // nao deve ter mais palavras alem do argumento
            if(token->next->next != NULL)
                return 0;
            else
                return 1;
        }
        /*
         * Verifica Gramatica DIRETIVA .align
         */
        if(!strcmp(token->palavra.palavra, ".align")){
            if (token->next == NULL && fim == 0)
                return 1; // por enquanto ok, espera pelo fim da linha

            // linha com apenas .align, sem argumentos
            if(token->next == NULL)
                return 0;

            // argumento deve ser DEC
            if(token->next->palavra.tipo != Decimal)
                return 0;

            // nao deve ter mais palavras alem do argumento
            if(token->next->next != NULL)
                return 0;
            else
                return 1;
        }
        /*
         * Verifica Gramatica DIRETIVA .wfill
         */
        if(!strcmp(token->palavra.palavra, ".wfill")) {
            if (token->next == NULL && fim == 0)
                return 1;   // por enquanto ok, espera pelo fim da linha

            // linha com apenas .wfill, sem argumentos
            if (token->next == NULL)
                return 0;  // dispara erro

            // .wfill com apenas um argumento
            if (token->next->next == NULL && fim == 1)
                return 0;  // dispara erro

            // primeiro argumento deve ser DEC
            tipoToken = token->next->palavra.tipo;
            if (tipoToken != Decimal)
                return 0;  // dispara erro

            // valida o seguundo argumento
            if (token->next->next) {
                // segundo argumento deve ser HEX, DEC ou NOME
                tipoToken = token->next->next->palavra.tipo;
                if (tipoToken != Hexadecimal && tipoToken != Decimal && tipoToken != Nome)
                    return 0;  // dispara erro

                // nao deve ter nada alem dos dois argumentos
                if (token->next->next->next != NULL)
                    return 0;
                else
                    return 1;
            }
        }
        /*
         * Verifica Gramatica DIRETIVA .word
         */
        if(!strcmp(token->palavra.palavra, ".word")){
            if (token->next == NULL && fim == 0)
                return 1; // por enquanto ok, espera pelo fim da linha

            // linha com apenas .word, sem argumentos
            if(token->next == NULL)
                return 0;

            // argumento deve ser HEX, DEC ou NOME
            tipoToken = token->next->palavra.tipo;
            if (tipoToken != Hexadecimal && tipoToken != Decimal && tipoToken != Nome)
                return 0;

            // nao deve ter mais palavras alem do argumento
            if(token->next->next != NULL)
                return 0;
            else
                return 1;
        }


    }
    return 0;
}


int whichToken(char *palavra) {

    int retornoFuncao;
    retornoFuncao = isInstrucao(palavra);
    if(retornoFuncao == 1) // is instruction
        return Instrucao;

    retornoFuncao = isDiretiva(palavra);
    if(retornoFuncao == 1) // is diretiva
        return Diretiva;
    if(retornoFuncao == -1) // error 'lexico'
        return -1;

    retornoFuncao = isDefRotulo(palavra);
    if(retornoFuncao == 1) // is defRotulo
        return DefRotulo;
    if(retornoFuncao == -1) // error 'lexico'
        return -1;

    retornoFuncao = isArgumento(palavra);
    if (retornoFuncao == -1)
        return -1; // error 'lexico'

    retornoFuncao = isHexadecimal(palavra);
    if(retornoFuncao == 1) // is hexadecimal
        return Hexadecimal;
    if(retornoFuncao == -1) // error 'lexico'
        return -1;

    retornoFuncao = isDecimal(palavra);
    if(retornoFuncao == 1) // is Decimal
        return Decimal;
    if(retornoFuncao == -1) // error 'lexico'
        return -1;

    retornoFuncao = isNome(palavra);
    if(retornoFuncao == 1) // is Simbolo or Rotulo
        return Nome;
    if(retornoFuncao == -1) // error 'lexico'
        return -1;


    return -1;
}

// Instrucao, Diretiva, DefRotulo, Hexadecimal, Decimal, Nome

int isInstrucao(char *palavra) {
    if (strcmp(palavra, "LOAD") == 0)
        return 1;
    if (strcmp(palavra, "LOAD-") == 0)
        return 1;
    if (strcmp(palavra, "LOAD|") == 0)
        return 1;
    if (strcmp(palavra, "LOADmq") == 0)
        return 1;
    if (strcmp(palavra, "LOADmq_mx") == 0)
        return 1;
    if (strcmp(palavra, "STOR") == 0)
        return 1;
    if (strcmp(palavra, "JUMP") == 0)
        return 1;
    if (strcmp(palavra, "JMP+") == 0)
        return 1;
    if (strcmp(palavra, "ADD") == 0)
        return 1;
    if (strcmp(palavra, "ADD|") == 0)
        return 1;
    if (strcmp(palavra, "SUB") == 0)
        return 1;
    if (strcmp(palavra, "SUB|") == 0)
        return 1;
    if (strcmp(palavra, "MUL") == 0)
        return 1;
    if (strcmp(palavra, "DIV") == 0)
        return 1;
    if (strcmp(palavra, "LSH") == 0)
        return 1;
    if (strcmp(palavra, "RSH") == 0)
        return 1;
    if (strcmp(palavra, "STORA") == 0)
        return 1;
    return 0;
} // feito

int isDiretiva(char *palavra){
    if (palavra[0] == '.'){
        if (strcmp(palavra, ".set") == 0){
            return 1;
        }
        if (strcmp(palavra, ".org") == 0){
            return 1;
        }
        if (strcmp(palavra, ".align") == 0){
            return 1;
        }
        if (strcmp(palavra, ".wfill") == 0){
            return 1;
        }
        if (strcmp(palavra, ".word") == 0){
            return 1;
        }
        // se comeca com '.' (ponto) mas não é nenhuma das diretivas acima então error lexico
        return -1;
    }
    return 0;
} // feito

int isArgumento(char *palavra) {
    char *caracter_first;
    char *caracter_second;

    caracter_first = strchr(palavra, '"');

    // nao achou o caracter "
    if (caracter_first == NULL)
        return 0;

    // achou mas nao esta no inicio
    // e.g. la"co
    if (strcmp(palavra, caracter_first) != 0)
        return -1;

    caracter_second = strchr(++caracter_first, '"');
    // nao achou um segundo " indicando o fim do argumento
    // e.g. "0x0000000001
    if (caracter_second == NULL)
        return -1;

    // tem mais coisa depois de "
    // e.g. "laco"dois
    if (strlen(caracter_second) > 1)
        return -1;

    // tira as aspas rotacionando para direita
    size_t tamanho = strlen(palavra) - 2;
    for (int i = 0; i < tamanho; ++i)
        palavra[i] = palavra[i+1];
    palavra[tamanho] = '\0';


    return 1;
} // feito

void debbug(char *string){
    printf("---------%s----\n", string);
}

int isDefRotulo(char *palavra) {
    char *caracter;
    size_t tamanho;

    caracter = strchr(palavra, ':');
    if (caracter == NULL)
        return 0; // is not rotulo

    // verifica o posicionamento de ':"
    // e.g. var::
    if(strlen(caracter) > 1)
        return -1; // retorna um erro lexico

    // verifica se o digito comeca com numero
    // e.g. 1var:
    if(isdigit(palavra[0]))
        return -1;

    // verifica se sao alfanumericos
    // e.g. var*:
    tamanho = strlen(palavra) - 1;
    for (int i = 0; i < tamanho; ++i)
        if (isalnum(palavra[i]) == 0 && palavra[i] != '_')
            return -1;


    return 1;
} // feito

int isHexadecimal(char *palavra) {
    // verifica se inicia por '0x'
    if(strlen(palavra) > 1 && palavra[0] == '0' && palavra[1] == 'x'){

        // verifica se possui 12 caracteres
        // e.g. 0x0A
        if(strlen(palavra) != 12)
            return -1;

        // verifica se cada caracter está entre 0-9 ou A-F ou a-f
        // e.g. 0x00000BoRin
        for (int i = 2; i < 12; i++)
            if (isdigit(palavra[i]) == 0 && isAtoF(palavra[i]) == 0)
                return -1;

        return 1;

    }
    return 0;
} // feito

int isAtoF(char c){
    if ((c >= 65 && c <= 70) || (c >= 97 && c <= 102))
        return 1;
    return 0;
}

int isDecimal(char *palavra) {
    size_t tamanho = strlen(palavra);
    for (int i = 0; i < tamanho; ++i)
        if(isdigit(palavra[i]) == 0)
            return 0;
    return 1;
} // ok

int isNome(char *palavra) {
    size_t tamanho = strlen(palavra);
    if (isdigit(palavra[0]))
        return 0;
    for (int i = 0; i < tamanho; i++) {
        if(isalnum(palavra[i]) == 0 && palavra[i] != '_')
            return 0;
    }
    return 1;
}

// funcoes para analise de erros
pointerLine insereToken(pointerLine eachLine, char *palavra, unsigned int linha){
    Token token;

    // cria um token temporario
    token.palavra = malloc(sizeof(char) * strlen(palavra));
    strcpy(token.palavra,palavra);
    token.tipo = (enum TipoDoToken )whichToken(token.palavra);
    if(token.tipo == -1)
        return NULL;

    token.linha = linha/2;

    // insere na lista de tokens para analise de erros gramaticais posteriormente
    pointerLine newToken, next;

    // cria o no
    newToken = malloc(sizeof(eachWord));
    if(!newToken) // erro no malloc
        return 0;
    newToken->next = NULL;
    newToken->palavra = token;

    if(!eachLine)
        return newToken;

    next = eachLine;
    // se a lista nao estiver vazia, procura pelo fim
    while (next->next)
        next = next->next;

    // insere no fim
    next->next = newToken;
    return eachLine;

} // feito

pointerLine apagaLista(pointerLine inicio) {
    // utiliza dois ponteiros auxiliares
    pointerLine next, previous;

    // ponteiros sao defasados em uma casa
    previous = inicio;
    if(inicio)
        next = inicio->next;

    while(previous){
        free(previous->palavra.palavra);
        free(previous);
        previous = next;
        if (next)
            next = next->next;
    }
    return NULL;
} // feito

void imprimeLista(pointerLine inicio){
    pointerLine next;
    next = inicio;
    if (!next)
        return;

    char tipoToken[100];
    while(next){
        switch ((int)next->palavra.tipo){
            case 0:
                strcpy(tipoToken, "Instrucao");
                break;
            case 1:
                strcpy(tipoToken, "Diretiva");
                break;
            case 2:
                strcpy(tipoToken, "DefRotulo");
                break;
            case 3:
                strcpy(tipoToken, "Hexadecimal");
                break;
            case 4:
                strcpy(tipoToken, "Decimal");
                break;
            case 5:
                strcpy(tipoToken, "Nome");
                break;
            default:
                strcpy(tipoToken, "----");
                break;
        }
        printf("Token: palavra = -%s-\n       tipo: %s\n", next->palavra.palavra, tipoToken);
        next = next->next;
    }
    putchar('\n');
} // feito



/*
 *
 *
 * pointerLine insereToken(pointerLine inicio, Token token) {

    pointerLine newToken, next;

    // cria o no
    newToken = malloc(sizeof(eachWord));
    if(!newToken)
        return NULL;
    newToken->next = NULL;
    newToken->palavra = token;


    next = inicio;
    if(!inicio)
        return newToken;

    // se a lista nao estiver vazia, procura pelo fim
    if(next)
        while (next->next)
            next = next->next;

    // insere no fim
    next->next = newToken;
    return inicio;
} // feito
 */

