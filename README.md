# Jogo da Adivinhação via Pipe

Projeto desenvolvido para a disciplina de **Sistemas Operacionais**

## Sobre o Projeto

O objetivo deste trabalho é implementar um jogo de adivinhação utilizando **comunicação entre processos (IPC)** por meio de **Pipes**, explorando conceitos fundamentais de Sistemas Operacionais, como criação de processos, troca de mensagens e sincronização.

Durante a execução, os processos se comunicam através de Pipes para enviar e receber informações relacionadas às tentativas de adivinhação, demonstrando na prática o funcionamento da comunicação entre processos em linguagem C.

## Estrutura do Projeto

src/      -> código-fonte
docs/     -> documentação e relatório
exemplos/ -> saídas de execução

## Como Compilar e Executar

A partir da raiz do projeto:

```bash
# Compilar
gcc -o adivinhacao src/adivinhacao.c

# Executar
./adivinhacao
```

Repita os dois comandos sempre que alterar `src/adivinhacao.c`.

## Tecnologias Utilizadas

* Linguagem C
* Pipes (IPC - Inter Process Communication)

## Objetivos de Aprendizagem

* Compreender o funcionamento de Pipes em sistemas operacionais;
* Implementar comunicação entre processos;
* Aplicar conceitos de criação e gerenciamento de processos;
* Desenvolver uma aplicação prática utilizando recursos do sistema operacional.

## Integrantes

* Alice Miranda
* João Gustavo
* Luiz Gustavo Salles
