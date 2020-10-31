Este README é referente ao código-fonte disponibilizado neste mesmo diretório para atender aos requisitos do exercício prático 2 (EP2) da disciplina de Sistemas Operacionais 2020 do Instituto de Matemática e Estatística da Universidade de São Paulo.
1 - Instruções para a compilação:
Para compilar o programa referente ao ep2, o seguinte comando é necessário:

	gcc -Wall -o ep2 main_ep2.c ep2.c -lpthread
	
1.2 - Também é possível utilizar o Makefile disponibilizado neste mesmo diretório via comando make. Além disso, é possível apagar o binário gerado, ep2, via comando make clean.

2 - Instruções para a execução:

2.1 - Para executar o programa ep2, realize o seguinte comando:

	./ep2 <d> <n> d
	
Onde <d> é um número de metros do velódromo, <n> é a quantidade de ciclistas e d é opcional para habilitar o debug.
