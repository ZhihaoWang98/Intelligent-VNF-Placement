Este README é referente ao código-fonte disponibilizado neste mesmo diretório para atender aos requisitos do exercício prático 1 (EP1) da disciplina de Sistemas Operacionais 2020 do Instituto de Matemática e Estatística da Universidade de São Paulo.
1 - Instruções para a compilação:
1.1 - Para compilar o programa referente ao possh, o seguinte comando é necessário:

	gcc -Wall -o possh main_possh.c possh.c -lreadline
	
1.2 - Já para compilar o programa do ep1, é necessário o comando:
	
	gcc -Wall -o ep1 main_ep1.c ep1.c -lpthread
	
1.3 - Também é possível utilizar o Makefile disponibilizado neste mesmo diretório via comando make. Além disso, é possível apagar os binários gerados, possh e ep1, via comando make clean.

2 - Instruções para a execução:

2.1 - Para executar o programa possh, realize o seguinte comando:

	./possh
	
2.2 - Para executar o programa ep1, realize o seguinte comando:
	./ep1 <escalonador> <arquivo_de_entrada.txt> <arquivo_de_saida.txt> d
	
Onde <escalonador> é um número de 1 a 3 e d é opcional.	



