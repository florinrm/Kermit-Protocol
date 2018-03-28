#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

unsigned char numar_secventa = 0;

uint8_t server_MAXL = MAXL;
uint16_t server_TIME = TIME;
uint8_t server_EOL = 0x0d;


int main(int argc, char** argv) {


	init(HOST, PORT);
    msg send_init_recv_msg, m, packetNAK, packetACK;
    char buffer[MAXL];
    memset(buffer, 0x41, MAXL);
    prepare_packet(&m, SEND_INIT, (unsigned char *) buffer, MAXL, numar_secventa);
    char does_connect = 1;
    int timeouts = 0;
    char output_file [MAXL];
    while (1)
    {
        msg * recv_packet = NULL;
        if(does_connect == 0){    //astept pachet de la sender
            printf("[Receiver] Waiting for packet from sender...\n");
            recv_packet = receive_message_timeout(TIME);
        } else {
            does_connect = 0;
            recv_message(&send_init_recv_msg);
            recv_packet = &send_init_recv_msg;
            printf("[Receiver] Trying to connect with sender...\n"); //asteapta primul mesaj(init) de la sender   
        }

        if (recv_packet == NULL) //tratarea timeoutului
        {
            ++timeouts;
            printf("[Receiver] TIMEOUT\n");
            if(timeouts > 2) { 
                printf("[Receiver] Number of timeout allowed exceeded!\n");
                exit(1);
            }
            continue;
        }

        timeouts = 0;

        if(!checkCRC(recv_packet)) { //verific crc-ul calculat cu cel din pachet
            printf ("[Receiver] Incorrect CRC\n");
            prepare_packet(&packetNAK, NAK, 0, 0, numar_secventa); //trimit nack in cazul in care pachetul a fost corupt
            send_message(&packetNAK);
            continue;
        }

        if(*(recv_packet->payload + 3) != SEND_INIT){
            prepare_packet(&packetACK, ACK, 0, 0, numar_secventa);
            send_message(&packetACK);
        } else { // Primesc configuratia senderului si ii trimit ack cu configuratia recieverului
            server_MAXL = (unsigned char) recv_packet->payload[4];
            memcpy(&server_TIME, recv_packet->payload + 5, 2);
            server_EOL = (unsigned char) recv_packet->payload[8];

    
            unsigned char reciever_settings[MAXL];
            memset(reciever_settings, 0, MAXL);
            reciever_settings[0] = MAXL;
            unsigned char time = TIME; 
            memcpy(reciever_settings + 1, &time, 2);
            reciever_settings[4] = EOL;

            prepare_packet(&packetACK, ACK, reciever_settings, MAXL, numar_secventa);
            send_message(&packetACK);

        }

        numar_secventa = (numar_secventa + 1) % 64;
        int fd;
        switch(recv_packet->payload[3]){ //in functie de tipul de pachet urmez protocolul
            case 'F':
                strcpy (output_file, "recv_");
                strcat (output_file, recv_packet->payload + 4);
                fd = open(output_file, O_CREAT | O_RDWR | O_TRUNC, 0777);
                break;
            case 'D':
                write(fd, recv_packet->payload + 4, (uint8_t)( recv_packet->payload[1] - 5));
                break;
            case 'Z':
                close(fd);
                break;
            case 'B':
                exit(0);
                break;
        }
        
        if(recv_packet->payload[3] != SEND_INIT)
            free(recv_packet);
    }

    printf("[Receiver] Transmission ended\n");

    return 0;
}