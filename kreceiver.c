#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

int main(int argc, char** argv) {


	init(HOST, PORT);
    msg sendInitMessage, message, packetNAK, packetACK;
    msg * receivedPacket = NULL; // pachetul pe care il primim
    char buffer[MAXL]; // buffer-ul pentru datele din fisiere
    memset(buffer, 0x41, MAXL);
    unsigned char seqNumber = 0; // numarul de secventa
    create_packet(&message, SEND_INIT, (unsigned char *) buffer, MAXL, seqNumber);
    // cream pachetul SEND-INIT
    unsigned short server_TIME = TIME;
    char does_connect = 1; // daca se conecteaza receiver-ul la sender
    int timeoutCount = 0; // counter de timeout
    char output_file [MAXL]; // bufferul pentru datele din fisiere
    while (1)
    {
        // daca nu se conecteaza receiver-ul la sender
        if (does_connect == 0) {
            printf("[Receiver] Trying to connect with sender...\n");
            receivedPacket = receive_message_timeout(TIME);
        } else {
            does_connect = 0;
            recv_message(&sendInitMessage); // primim pachet de la sender
            receivedPacket = &sendInitMessage;
            printf("[Receiver] Waiting for packet from sender...\n");
        }

        if (receivedPacket == NULL)
        {
            ++timeoutCount;
            printf("[Receiver] TIMEOUT\n");
            if(timeoutCount > 2) { 
                printf("[Receiver] Number of timeout allowed exceeded!\n");
                exit(1);
            }
            continue;
        }

        timeoutCount = 0;

        if(!checkCRC(receivedPacket)) { 
            // daca CRC-ul nu este corect, incerc sa primesc din nou pachetul si dau NAK
            printf ("[Receiver] Incorrect CRC\n");
            create_packet(&packetNAK, NAK, 0, 0, seqNumber); //trimit nack in cazul in care pachetul a fost corupt
            printf("[Receiver] Sending NAK\n");
            send_message(&packetNAK); // dau NAK la sender
            continue;
        }

        if(*(receivedPacket->payload + 3) != SEND_INIT) {
            create_packet(&packetACK, ACK, 0, 0, seqNumber); // creez ACK
            printf("[Receiver] Sending ACK\n");
            send_message(&packetACK); // dau ACK
        } else { 
            // dau ACK pentru SEND-INIT (cazul special de la ACK cu data de la SEND-INIT)
            memcpy(&server_TIME, receivedPacket->payload + 5, 2);
            unsigned char recBuffer[MAXL];
            memset(recBuffer, 0, MAXL);
            recBuffer[0] = MAXL;
            unsigned char time = TIME; 
            memcpy(recBuffer + 1, &time, 2);
            recBuffer[4] = EOL;

            create_packet(&packetACK, ACK, recBuffer, MAXL, seqNumber); // creez ACK
            printf("[Receiver] Sending ACK\n");
            send_message(&packetACK); // dau ACK

        }

        seqNumber = (seqNumber + 1) % 64; // incrementam numarul de secventa
        int fd;

        if (getTypePacket(receivedPacket) == FILE_HEADER) {
            // cream si deschidem fisierul in care scriem datele
            strcpy (output_file, "recv_");
            strcat (output_file, receivedPacket->payload + 4);
            printf("[Receiver] Receiving FILE-HEADER\n");
            fd = open(output_file, O_CREAT | O_RDWR | O_TRUNC, 0777);
        } else if (getTypePacket(receivedPacket) == DATE) {
            // scriem datele din fisier
            printf("[Receiver] Receiving DATE\n");
            write(fd, receivedPacket->payload + 4, (unsigned char) (receivedPacket->payload[1] - 5));
        } else if (getTypePacket(receivedPacket) == EOFZ) {
            // inchidem fisierul
            printf("[Receiver] Receiving END-OF-FILE\n");
            close (fd);
        } else if (getTypePacket(receivedPacket) == EOT) {
            // sfarsitul transmisiei
            printf("[Receiver] Receiving END-OF-TRANSMISSION\n");
            printf("[Receiver] Transmission ended\n");
            exit(0);
        }
        
        // curatam memoria ca toti oamenii
        if(getTypePacket(receivedPacket) != SEND_INIT)
            free(receivedPacket);
    }
    return 0;
}