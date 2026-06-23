/*adivinhacao.c*/
/*
 * Jogo de Adivinhacao via Pipe
 * Disciplina: Sistemas Operacionais
 *
 * Baseado no modelo pipebid.c fornecido pelo professor:
 * Processo PAI   -> funcao client(): sorteia o numero secreto e responde
 *                   aos palpites do filho (maior, menor, acertou).
 * Processo FILHO -> funcao server(): usa busca binaria para tentar
 *                   descobrir o numero secreto.
 * Comunicacao via dois pipes (pipe1: pai->filho / pipe2: filho->pai),
 * trocando mensagens em formato de texto (strings).
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define PAUSA 400000  /* microsegundos, so para deixar a saida legivel */

#define MAXBUFF 1024  /* numero de caract. do buffer */
#define MIN 1
#define MAX 100

void client(int readfd, int writefd);
void server(int readfd, int writefd);

int main()
{
   int  descritor,  /* usado para criar o processo filho pelo fork */
        pipe1[2],   /* comunicacao pai -> filho */
        pipe2[2];   /* comunicacao filho -> pai */

   if (pipe(pipe1)<0 || pipe(pipe2) <0)
   {
        printf("Erro na chamada PIPE");
        exit(0);
   }

   srand((unsigned int)time(NULL));

   /* Fork para criar o processo filho */

   if ( (descritor = fork()) <0)
   {
        printf("Erro na chamada FORK");
        exit(0);
   }

   else if (descritor >0)  /* PROCESSO PAI */
   {
        close(pipe1[0]); /* fecha leitura no pipe1 */
        close(pipe2[1]); /* fecha escrita no pipe2 */

        client(pipe2[0], pipe1[1]);  /* Chama CLIENTE no PAI */

        close(pipe1[1]); /* fecha pipe1 */
        close(pipe2[0]); /* fecha pipe2 */

        wait(NULL); /* aguarda o filho terminar */
        exit(0);

   } /* FIM DO PROCESSO PAI */

   else /* PROCESSO FILHO */

   {
        close(pipe1[1]); /* fecha escrita no pipe1 */
        close(pipe2[0]); /* fecha leitura no pipe2 */

        server(pipe1[0], pipe2[1]);  /* Chama SERVIDOR no FILHO */

        close(pipe1[0]); /* fecha leitura no pipe1 */
        close(pipe2[1]); /* fecha escrita no pipe2 */
        exit(0);

   } /* FIM DO PROCESSO FILHO */
} /* FIM DO MAIN */


/* -----------------------------------------------------------------------------------------------------------
Funcao Client:  Executa no processo PAI
                Sorteia o numero secreto
                Recebe os palpites do FILHO e responde Maior/Menor/Acertou
----------------------------------------------------------------------------------------------------------- */

void client(int readfd,   /* leitura do pipe2[0] */
            int writefd)   /* escrita no pipe1[1] */
{
        char buff[MAXBUFF];
        int secreto, palpite;

        secreto = (rand() % (MAX - MIN + 1)) + MIN;

        printf("\nPai: Pensei em um numero entre %d e %d.\n", MIN, MAX);
        fflush(stdout);

        while(1)
        {
                /* recebe o palpite do filho */
                read(readfd, buff, MAXBUFF);
                palpite = atoi(buff);
                usleep(PAUSA);

                if (palpite == secreto)
                {
                        strcpy(buff, "Acertou");
                        write(writefd, buff, MAXBUFF);
                        printf("Pai: Acertou! O numero era %d.\n", secreto);
                        fflush(stdout);
                        break;
                }
                else if (palpite < secreto)
                {
                        strcpy(buff, "Maior");
                        write(writefd, buff, MAXBUFF);
                        printf("Pai: Maior\n");
                        fflush(stdout);
                }
                else
                {
                        strcpy(buff, "Menor");
                        write(writefd, buff, MAXBUFF);
                        printf("Pai: Menor\n");
                        fflush(stdout);
                }
                usleep(PAUSA);
        }

} /* Fim da Funcao CLIENT */


/* -----------------------------------------------------------------------------------------------------------
Funcao Server:  Executa no processo FILHO
                Usa busca binaria para tentar adivinhar o numero secreto
                escolhido pelo PAI
----------------------------------------------------------------------------------------------------------- */
void server(int readfd,   /* leitura do pipe1[0] */
            int writefd)   /* escrita no pipe2[1] */
{
        char buff[MAXBUFF];
        int baixo = MIN, alto = MAX, palpite;

        while(1)
        {
                palpite = (baixo + alto) / 2;
                printf("Filho: %d\n", palpite);
                fflush(stdout);

                /* envia o palpite ao pai */
                sprintf(buff, "%d", palpite);
                write(writefd, buff, MAXBUFF);

                /* recebe a resposta do pai */
                read(readfd, buff, MAXBUFF);
                usleep(PAUSA);

                if (strcmp(buff, "Acertou") == 0)
                {
                        printf("Filho: Acertei!\n");
                        fflush(stdout);
                        break;
                }
                else if (strcmp(buff, "Maior") == 0)
                {
                        baixo = palpite + 1;
                }
                else /* "Menor" */
                {
                        alto = palpite - 1;
                }
                usleep(PAUSA);
        }

} /* Fim da Funcao Server */
