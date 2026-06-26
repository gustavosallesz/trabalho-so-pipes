# Explicação do Código — `src/adivinhacao.c`

Documento de estudo para entender, linha a linha, como o jogo de adivinhação
funciona usando Pipes. Útil para revisar antes da entrega e para explicar o
trabalho aos colegas.

## 1. Ideia geral do programa

O programa cria **dois processos** (pai e filho, via `fork()`) que conversam
entre si por **dois pipes**:

- O **pai** pensa em um número secreto entre 1 e 100 e fica respondendo
  "Maior", "Menor" ou "Acertou" aos palpites do filho.
- O **filho** não sabe o número secreto. Ele vai *chutando* palpites usando
  **busca binária**, com base nas respostas do pai, até acertar.

Isso é justamente o que a disciplina pede: dois processos distintos (com
memória separada) só conseguem trocar informação através de um mecanismo de
IPC (*Inter-Process Communication*) — aqui, o Pipe.

## 2. Estruturas de dados utilizadas

| Variável | Tipo | Para que serve |
|---|---|---|
| `pipe1[2]`, `pipe2[2]` | `int[2]` | Cada pipe é representado por um array de 2 *file descriptors*: `[0]` é a ponta de **leitura**, `[1]` é a ponta de **escrita**. É a estrutura que o próprio kernel exige para `pipe()`. |
| `buff[MAXBUFF]` | `char[1024]` | Buffer de texto usado para montar/receber as mensagens trocadas pelos pipes (ex.: `"50"`, `"Maior"`, `"Acertou"`). Tudo é trocado como **string**, não como número binário. |
| `secreto`, `palpite`, `baixo`, `alto` | `int` | Variáveis simples de controle do jogo (ver seção 4). |

Não há listas, structs ou outras estruturas mais complexas — o trabalho é
deliberadamente simples: dois inteiros de controle de cada lado e um buffer de
texto para a comunicação.

### Por que dois pipes e não um?

Pipe é **unidirecional**. Como pai e filho precisam falar nos dois sentidos
(filho manda palpite, pai responde), são necessários dois pipes:

- `pipe1`: pai → filho
- `pipe2`: filho → pai

## 3. Passo a passo do `main()` (linhas 24-74)

1. **Cria os dois pipes** (`pipe(pipe1)`, `pipe(pipe2)`). Se falhar, aborta.
2. **`srand(time(NULL))`** — semente do gerador de números aleatórios, baseada
   no horário atual, para o número secreto não ser sempre igual.
3. **`fork()`** cria o processo filho. A partir daqui existem duas cópias do
   programa rodando:
   - `descritor > 0` → está executando no **processo pai**, e `descritor`
     guarda o PID do filho.
   - `descritor == 0` → está executando no **processo filho**.
4. **Cada processo fecha as pontas do pipe que não vai usar**:
   - Pai: fecha `pipe1[0]` (não lê de pipe1) e `pipe2[1]` (não escreve em
     pipe2). Fica só com `pipe2[0]` (lê) e `pipe1[1]` (escreve).
   - Filho: fecha `pipe1[1]` e `pipe2[0]`. Fica com `pipe1[0]` (lê) e
     `pipe2[1]` (escreve).

   Isso é importante: um pipe só "sabe" que o outro lado fechou de verdade se
   **todas** as cópias do descritor de escrita forem fechadas. Por isso cada
   processo fecha logo as pontas que não usa.
5. O pai chama `client(...)`; o filho chama `server(...)`.
6. Depois que `client()` retorna (jogo terminou), o pai fecha os descritores
   restantes e faz `wait(NULL)` para esperar o filho terminar antes de sair
   (evita processo zumbi).

## 4. `client()` — o PAI (linhas 79-122)

1. Sorteia o número secreto: `secreto = (rand() % (MAX - MIN + 1)) + MIN;`
   → gera um inteiro entre 1 e 100.
2. Entra em loop infinito:
   - Lê o palpite do filho pelo pipe (`read(readfd, buff, MAXBUFF)`) e
     converte a string para número com `atoi`.
   - Compara com o secreto e escreve de volta no pipe a resposta:
     `"Acertou"`, `"Maior"` (secreto é maior, ou seja, o filho chutou baixo) ou
     `"Menor"` (o filho chutou alto).
   - Quando acerta, sai do loop (`break`) e a função termina.
3. `usleep(PAUSA)` só serve para deixar a saída lenta o suficiente pra
   acompanhar print por print no terminal — não tem efeito sobre a lógica do
   jogo.

## 5. `server()` — o FILHO — onde está a busca binária (linhas 129-162)

Esta é a parte central do trabalho. O filho mantém um **intervalo de busca**
`[baixo, alto]`, inicializado com `baixo = MIN` (1) e `alto = MAX` (100).

```c
int baixo = MIN, alto = MAX, palpite;

while (1) {
    palpite = (baixo + alto) / 2;     // <-- BUSCA BINÁRIA: chuta o meio do intervalo
    ...
    write(writefd, buff, MAXBUFF);     // envia o palpite ao pai
    read(readfd, buff, MAXBUFF);       // lê a resposta do pai

    if (strcmp(buff, "Acertou") == 0) break;
    else if (strcmp(buff, "Maior") == 0) baixo = palpite + 1; // secreto está na metade de cima
    else /* "Menor" */                    alto  = palpite - 1; // secreto está na metade de baixo
}
```

A cada rodada o filho **descarta metade do intervalo restante**:

- Se a resposta é `"Maior"` (secreto maior que o palpite), o filho sabe que o
  número não pode estar entre `MIN` e `palpite`, então move `baixo` para
  `palpite + 1`.
- Se a resposta é `"Menor"`, o número não pode estar entre `palpite` e `MAX`,
  então move `alto` para `palpite - 1`.

Isso é exatamente a definição de **busca binária**: a cada passo o espaço de
busca é cortado pela metade, então o número de tentativas para um intervalo de
100 valores é, no máximo, `log2(100) ≈ 7`. "divide et impera" - Júlio César =)

### Por que o primeiro palpite é sempre 50?

Porque `baixo` e `alto` **sempre começam com os mesmos valores fixos**
(`MIN = 1` e `MAX = 100`), e o primeiro palpite é simplesmente o ponto médio
desse intervalo inicial:

```
palpite = (baixo + alto) / 2 = (1 + 100) / 2 = 50   (divisão inteira, 50.5 → 50)
```

O filho **não tem nenhuma informação ainda** sobre o número secreto na
primeira rodada — ele só recebe a resposta do pai *depois* de enviar o
palpite. Logo, a única decisão racional (e a estratégia de busca binária)
é começar sempre pelo meio do intervalo total possível, **independente** de
qual número o pai sorteou. A aleatoriedade do jogo está inteiramente do lado
do pai (`secreto = rand() % ... `); o filho é totalmente determinístico dado
o protocolo de respostas.

