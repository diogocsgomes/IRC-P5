/*
* Programa para explicar como se criam threads
* 
* 6 passos que t�m que ter em aten��o:
* 
*	1 - Declarar as vari�veis (ver formul�rio)
*	2 - Atribuir os valores � vari�vel "sa" (ver formul�rio)
*	3 - Criar a thread com a fun��o "CreateThread" (ver formul�rio)
*	    Ter aten��o aos par�metros relativos � fun��o que a thread vai executar e a vari�vel que queremos passar para a fun��o
*	4 - Definir par�metro como LPVOID (ver formul�rio)
*	5 - Fazer cast para o tipo de vari�vel que precisamos
*	6 - Terminar thread principal (main) com ExitThread(...)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>

void MostraMensagem(LPVOID param);

/*________________________________ main ________________________________________
*/
int main(int argc, char* argv[]) {
	int numeroDeThreads;
	int result;
	int i;
	
	// Passo 1	- Declarar as vari�veis necess�rias
	//			- Tal como est� no formul�rio
	SECURITY_ATTRIBUTES sa;
	DWORD thread_id;

	printf("Este programa cria X threads que apresentam uma mensagem\n");
	printf("Quantas threads queres criar?\n");
	printf("> ");
	scanf_s("%d", &numeroDeThreads);
	printf("\nVou criar %d threads\n", numeroDeThreads);

	i = numeroDeThreads;

	while (i != 0) {

		// Passo 2	- Atribuir os valores � vari�vel sa
		//			- Tal como est� no formul�rio
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;


		// Passo 3	- usar a fun��o CreateThread para criar a thread
		//			- Tal como est� no formul�rio
		//			
		//			- Ter especial aten��o para 2 par�metros:
		//				- MostraMensagem: fun��o que a thread vai executar
		//				- numeroDeThreads: vari�vel que � passada para a fun��o MostraMensagem
		result = CreateThread(&sa, 0, (LPTHREAD_START_ROUTINE)MostraMensagem, (LPVOID)numeroDeThreads, (DWORD)0, &thread_id);
		if (result == NULL) {
			printf("Nao e' possivel iniciar uma nova thread (error: %d)\n", GetLastError());
		}

		i--;
	}

	// Passo 6	- Terminar a thread principal (main) com a fun��o ExitThread(...)
	//			- Sem esta instru��o, � o processo que termina em vez da thread principal.
	//			- Se terminarmos o processo, ent�o todas as suas threads ser�o terminadas.
	ExitThread(EXIT_SUCCESS);
}

// Passo 4	- Definir par�metro como LPVOID
//			- Tal como est� no formul�rio
void MostraMensagem(LPVOID param) {
	int numeroDeThreads;

	// Passo 5	- Fazer cast para o tipo de vari�vel que precisamos
	numeroDeThreads = (int)param;

	printf("\nOla, existem %d threads e eu tenho o id = %d", numeroDeThreads, GetCurrentThreadId());
	ExitThread(EXIT_SUCCESS);
}