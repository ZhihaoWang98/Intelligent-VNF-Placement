Este README é referente ao código-fonte disponibilizado neste mesmo diretório para atender aos requisitos do exercício prático 3 (EP3) da disciplina de Sistemas Operacionais 2020 do Departamento de Ciência da Computação da Universidade de São Paulo.
1 - Instruções para a compilação:
Para compilar o programa referente ao ep3, o seguinte comando é necessário:

	gcc -Wall -o ep3 main_ep3.c ep3.c -lreadline
	
1.2 - Também é possível utilizar o Makefile disponibilizado neste mesmo diretório via comando make. Além disso, é possível apagar o binário gerado, ep3, via comando make clean.

2 - Instruções para a execução:

2.1 - Para executar o prompt desenvolvido no programa ep3, realize o seguinte comando:

	./ep3
	
2.2 - É possível também executar o modo de testes desenvolvido. Basta executar:

	./ep3 <opção> <estado>
	
Onde <opção> é um número, de 1 a 8, referente ao caso de teste solicitado no ep3. Já <estado> é um número, de 1 a 3, referente ao estado do sistema de arquivos.
