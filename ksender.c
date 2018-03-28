#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

uint8_t numar_secventa = 0;


void send (msg* m, unsigned char *recv_maxl, unsigned short *recv_time, unsigned char *recv_eol) {
	msg* r;
	int timeouts = 0;
	while (1)
	{
        send_message(m);
		r = receive_message_timeout(TIME); //astept ack de la receiver
		if (r != NULL) {
			if(memcmp(&numar_secventa, r->payload + 2, 1) == 0 && r->payload[3] == ACK){
                //pachetul a fost trimis cu succes si incrementez numarul de secventa
				if(m->payload[3] == SEND_INIT){ // Daca timit pachetul S astept primesc ack cu configuratia recieverului
                    *recv_maxl = (uint8_t ) r->payload[4];
                    memcpy(&((recv_time)), r->payload + 5, 2);
                    *recv_eol = (uint8_t) r->payload[8];
                }
                numar_secventa = (numar_secventa + 1) % 64;
				break;
			}
			continue;
		}
		++timeouts;
		if(timeouts > 2) {
            printf("[Sender] Number of timeout allowed exceeded!\n");
            exit(1);
        }
	}
}
int main(int argc, char** argv) {
    
    init(HOST, PORT);
    uint8_t send_init_data[MAXL], buffer[MAXL];
    memset(send_init_data, 0, MAXL);
    send_init_data[0] = MAXL;
    uint8_t recv_maxl, recv_eol;
    uint16_t time = TIME, recv_time; 
    memcpy(send_init_data + 1, &time, 2);
    send_init_data[4] = EOL;

    msg send_init, file_header, file_data, end_of_file, end_of_transmission;
    prepare_packet(&send_init, SEND_INIT, send_init_data, MAXL, numar_secventa);
    printf("[Sender] Sending SEND-INIT\n");
    send(&send_init, &recv_maxl, &recv_time, &recv_eol);
    int i, fd, length;
    for (i = 1; i < argc; ++i)
    {
        fd = open(argv[i], O_RDONLY);
        prepare_packet(&file_header, FILE_HEADER, argv[i], strlen(argv[i]) + 1, numar_secventa);
        printf("[Sender] Sending FILE-HEADER from %s\n", argv[i]);
        send(&file_header, &recv_maxl, &recv_time, &recv_eol);

        while((length = read(fd, buffer, MAXL)) > 0){
            prepare_packet(&file_data, DATE, buffer, length, numar_secventa);
            printf("[Sender] Sending DATA from %s\n", argv[i]);
            send(&file_data, &recv_maxl, &recv_time, &recv_eol);
        }
        prepare_packet(&end_of_file, EOFZ, 0, 0, numar_secventa);
        printf("[Sender] Sending END-OF-FILE from %s\n", argv[i]);
        send(&end_of_file, &recv_maxl, &recv_time, &recv_eol);
    }
    printf("[Sender] Sending END-OF-TRANSMISSION\n");
    prepare_packet(&end_of_transmission, EOT, 0, 0, numar_secventa);
    send(&end_of_transmission, &recv_maxl, &recv_time, &recv_eol);
    printf("[Sender] Transmission ended\n");
    return 0;
}