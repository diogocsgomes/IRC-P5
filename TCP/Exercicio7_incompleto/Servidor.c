/*________________________________PROXY.C___________________________________*/
/*======================= Servidor concorrente Proxy =========================
Este servidor destina-se a servir de proxy para acesso a um determinado servidor.
O porto de escuta bem como o IP/nome e porto do servidor sao passados na linha
de comando.
============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>

#pragma comment(lib, "Ws2_32.lib")

#define TIMEOUT		3   //segundos
#define BUFFERSIZE	4096

void AtendeCliente(LPVOID param);
int exchange(SOCKET s1, SOCKET s2);
void Abort(char* msg, SOCKET s);

struct sockaddr_in dest_addr;

/*________________________________ main ________________________________________
*/
int main(int argc, char* argv[]) {

	SOCKET sock = INVALID_SOCKET, newSock = INVALID_SOCKET;
	int iResult;
	int cliaddr_len;
	struct sockaddr_in cli_addr, serv_addr;
	struct hostent* info;
	WSADATA wsaData;
	SECURITY_ATTRIBUTES sa;
	DWORD thread_id;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <porto de escuta> <maquina destino> <porto destino>\n", argv[0]);
		exit(EXIT_SUCCESS);
	}

	/*=============== INICIA OS WINSOCKS ==============*/
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		getchar();
		exit(1);
	}

	/*===== PREENCHE A ESTRUTURA DE ENDERECO PARA O SERVIDOR ============*/
	memset((char*)&dest_addr, 0, sizeof(dest_addr));
	dest_addr.sin_family = AF_INET;
	// a) Preencha o PORTO do servidor de destino (obtido atrav�s dos argumentos da linha de comandos)
	dest_addr.sin_port = ...
	// b) Preencha o IP do servidor de destino (obtido atrav�s dos argumentos da linha de comandos)
	dest_addr.sin_addr.s_addr = ...

	//Resolve o nome caso seja necessario ...
	if (dest_addr.sin_addr.s_addr == INADDR_NONE)
		// c) Utilize a fun��o que permite obter o IP atrav�s de um nome
		if ((info = ...) == NULL)
			Abort("Host desconhecido", sock);
		else
			memcpy(&(dest_addr.sin_addr.s_addr), info->h_addr, info->h_length);

	/*================== ABRE SOCKET PARA ESCUTA DE CLIENTES ================*/
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
		Abort("Impossibilidade de abrir socket", sock);

	/*=================== PREENCHE ENDERECO DE ESCUTA =======================*/
	memset((char*)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  /*Recebe de qq interface*/
	serv_addr.sin_port = htons(atoi(argv[1]));  /*Escuta no porto Well-Known*/

	/*====================== REGISTA-SE PARA ESCUTA =========================*/
	if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		Abort("Impossibilidade de registar-se para escuta", sock);

	/*============ AVISA QUE ESTA PRONTO A ACEITAR PEDIDOS ==================*/
	if (listen(sock, 5) == SOCKET_ERROR)
		Abort("Impossibilidade de escutar pedidos", sock);

	printf("<SER> Servidor proxy pronto no porto de escuta: %s\n", argv[1]);
	printf("<SER> A fazer de \"ponte para\": <%s:%s>\n\n", argv[2], argv[3]);

	/*========== PASSA A ATENDER CLIENTES DE FORMA CONCORRENTE  =============*/
	cliaddr_len = sizeof(cli_addr);
	while (1) {
		/*====================== ATENDE PEDIDO ========================*/
		newSock = accept(sock, (struct sockaddr*)&cli_addr, &cliaddr_len);
		if (newSock == SOCKET_ERROR) {

			if (WSAGetLastError() == WSAEINTR)
				continue;

			printf("<SERV> Impossibilidade de aceitar cliente...\n");
		}
		else {

			sa.nLength = sizeof(sa);
			sa.lpSecurityDescriptor = NULL;

			if (CreateThread(&sa, 0, (LPTHREAD_START_ROUTINE)AtendeCliente, (LPVOID)newSock, (DWORD)0, &thread_id) == NULL) {
				printf("<SER> Nao e' possivel iniciar uma nova thread (error: %d)\n", GetLastError());
				printf("<SER> O cliente <%s:%d> nao sera' atendido\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
				closesocket(newSock);
			}

			printf("<SER> Cliente <%s:%d> atendido pela thread com id: %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), thread_id);
		}
	}
}

/*___________________________ AtendeCliente ____________________________________
Atende cliente.
______________________________________________________________________________*/

void AtendeCliente(LPVOID param) {
	SOCKET sockCli, sockDest; // sockDest: socket para comunicar como o servidor
	fd_set fdread, fdtemp;
	struct timeval timeout = { TIMEOUT, 0 };

	sockCli = (SOCKET)param;

	// d) Crie o socket que ser� utilizado para comunicar com o servidor de destino
	if ((sockDest = ...) == INVALID_SOCKET) {
		fprintf(stderr, "<SER_%d> Erro na criacao do socket para comunicar com o destino (%d)...\n", GetCurrentThreadId(), WSAGetLastError());
		closesocket(sockCli);
		return;
	}

	// e) Fa�a a conex�o do socket criado anteriormente com a estrutura sockaddr_in (preenchida com os dados do servidor de destino)
	if (...(sockDest, (...)&dest_addr, sizeof(...)) == SOCKET_ERROR) {
		fprintf(stderr, "<SER_%d> Erro na ligacao ao destino (%d)...\n", GetCurrentThreadId(), WSAGetLastError());
		closesocket(sockCli); closesocket(sockDest);
		return;
	}

	// f) Inicialize o conjunto de sockets
	...

	// g) Adicione os sockets (sockCli e sockDest) ao conjunto de sockets (fdread)
	...
	...

	while (1) {
		/*==================== PROCESSA PEDIDO ==========================*/

		fdtemp = fdread;

		// h) Preencha a fun��o select
		switch (select(...)) {

		case SOCKET_ERROR:
			if (WSAGetLastError() == WSAEINTR)
				break;

			fprintf(stderr, "<SER_%d> Erro na rotina select (%d) ...\n", GetCurrentThreadId(), WSAGetLastError());
			closesocket(sockCli); closesocket(sockDest);
			return;

		case  0:
			fprintf(stderr, "\n<SER_%d> O cliente nao enviou dados e encerrou a ligacao...\n", GetCurrentThreadId());
			break;

		default:
			// i) Verifique se o sockCli faz parte do conjunto de sockets que est�o prontos para leitura
			if (...) {
				if (exchange(sockCli, sockDest) <= 0) {
					closesocket(sockCli); closesocket(sockDest);
					return;
				}
			}

			// j) Verifique se o sockDest faz parte do conjunto de sockets que est�o prontos para leitura
			if (...) {
				if (exchange(sockDest, sockCli) <= 0) {
					closesocket(sockDest); closesocket(sockCli);
					return;
				}
			}

			break;
		} //switch
	} //while
}

/*_____________________________ exchange _______________________________________
Recebe um caractere do primeiro socket e escreve-o no segundo

Devolve:
	SOCKET_ERROR : se houve erro
			   0 : EOF
			>= 0 : se leu algum byte
______________________________________________________________________________*/
int exchange(SOCKET s1, SOCKET s2)
{
	int result;
	char c;

	// k) Escreva o nome da fun��o quer permite receber dados de um socket
	if ((result = ...(s1, &c, sizeof(char), 0)) == sizeof(char))
		// l) Escreva o nome da fun��o quer permite enviar dados atrav�s de um socket
		result = ...(s2, &c, sizeof(char), 0);

	if (result == 0)
		printf("<SER_%d> Connection closed by foreign host\n", GetCurrentThreadId());

	if (result == SOCKET_ERROR) {
		printf("<SER_%d> Erro no acesso para I/O a um dos sockets (%d)\n", GetCurrentThreadId(), WSAGetLastError());
	}

	return result;
}

/*________________________________ Abort________________________________________
Mostra a mensagem de erro associada ao ultimo erro dos Winsock e abandona com
"exit status" a 1
_______________________________________________________________________________
*/
void Abort(char* msg, SOCKET s)
{
	fprintf(stderr, "\a<SER_%d> Erro fatal: %s <%d>\n", GetCurrentThreadId(), msg, WSAGetLastError());

	if (s != INVALID_SOCKET)
		closesocket(s);

	exit(EXIT_FAILURE);
}
