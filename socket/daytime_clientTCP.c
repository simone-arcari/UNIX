/* daytime_clientTCP.c - code for example client program that uses TCP
   Tratto da W.R. Stevens, "Network Programming Vol. 1"
   Ultima revisione: 14 gennaio 2008

    ogni associazione socket è una quintupla di valori:
    { protocollo, indirizzo locale, porta locale, indirizzo remoto, porta remota }

    int socket(dominio, tipo, protocollo);

    dominio{ [indica il protocollo da utilizzare]
        1) AF_INET (AF_INET6)           -> internet protocols
        2) AF_UNIX                      -> unix internal protocols
        3) AF_NS                        -> xerox NS protocols
        4) AF_IMPLINK                   -> Interface Message Processor link layer

        AF significa address family


        esiste una notazione equivalente:
        1) PF_INET (AF_INET6)           -> internet protocols
        2) PF_UNIX                      -> unix internal protocols
        3) PF_NS                        -> xerox NS protocols
        4) PF_IMPLINK                   -> Interface Message Processor link layer

        PF significa protocol family


        in generale abbiamo:
        1) PF_UNIX, PF_LOCAL            -> Local communication
        2) PF_INET                      -> IPv4 Internet protocols
        3) PF_INET6                     -> IPv6 Internet protocols
        4) PF_IPX                       -> IPX-Novell protocols
        5) PF_NETLINK                   -> Kernel user interface device
        6) PF_X25                       -> ITU-T X.25 / ISO-8208 protocol
        7) PF_AX25                      -> Amateur radio AX.25 protocol
        8) PF_ATMPVC                    -> Access to raw ATM PVCs
        9) PF_APPLETALK                 -> Appletalk
        19) PF_PACKET                   -> Low level packet interface        
    }

    tipo{ [indica il tipo di comunicazione]
        1) SOCK_STREAM                  -> orientato alla connessione (flusso continuo di dati)(streaming)
        2) SOCK_DGRAM                   -> senza connessione (pacchetti)(datagram)
        3) SOCK_RAW                     -> per applicazioni dirette su IP (riservato all'uso di sistema)(raw data)
        4) SOCK_SEQPACKET               -> pacchetti sequenziali
    }

    protocolo{ [solitamente si pone a 0 per selezionare il protocolo di deafault per la coppia (dominio , tipo)]
        1) IPPROTO_UDP , UDP
        2) IPPROTO_TCP , TCP
        3) IPPROTO_ICMP , ICMP          -> Interent Control Management Protocol (ha funzioni di monitoraggio/gestione del livello IP)
        4) IPPROTO_RAW

        per Posix la definizione delle macro IPPROTO_xxx è nell'header <netinet/in.h>
    }

    in caso di errore il codice d'errore viene scrittto nella variabile globale errno
    è possibile usare strerror()e perror() per riportare in opportuni messaggi la condizione di
    errore verificatasi



*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERV_PORT 5193
#define MAXLINE 1024

int main(int argc, char **argv)
{
    int sockfd; /* il descrittore (file descriptor) del socket è un numero intero e per il processo che crea la socket, esso è unico */
    int n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;

    if (argc != 2) {    /* controlla numero degli argomenti */
        fprintf(stderr, "utilizzo: daytime_clientTCP <indirizzo IP server>\n");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {   /* crea il socket */
        perror("errore in socket");
        exit(1);
    }

    memset((void *)&servaddr, 0, sizeof(servaddr)); /* azzera servaddr */
    servaddr.sin_family = AF_INET;                  /* assegna il tipo di indirizzo */
    servaddr.sin_port = htons(SERV_PORT);           /* assegna la porta del server */

    /* assegna l'indirizzo del server prendendolo dalla riga di comando. L'indirizzo � una stringa e deve essere convertito in intero in network byte order. */
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        fprintf(stderr, "errore in inet_pton per %s\n", argv[1]);
        exit(1);
    }

    /* stabilisce la connessione con il server */
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("errore in connect");
        exit(1);
    }

    /* legge dal socket fino a quando non trova EOF */
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0; /* aggiunge il carattere di terminazione */

        if (fputs(recvline, stdout) == EOF) {   /* stampa il contenuto di recvline sullo standard output */
            fprintf(stderr, "errore in fputs\n");
            exit(1);
        }
    }

    if (n < 0) {
        perror("errore in read");
        exit(1);
    }

    exit(0);
}
