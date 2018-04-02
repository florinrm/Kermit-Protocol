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

unsigned char seqNumber = 0;


void send (msg* m, unsigned char *recv_maxl, unsigned short *recv_time, unsigned char *recv_eol) {
	msg* r;
	int countTimeouts = 0;
	while (1)
	{
        send_message(m); // trimitem mesajul si vedem daca primim ACK sau nu
		r = receive_message_timeout(TIME); //asteptam ACK de la receiver
		if (r != NULL) {
			if (getTypePacket(r) == ACK && memcmp(&seqNumber, r->payload + 2, 1) == 0) {
                //pachetul a fost trimis cu succes si incrementez numarul de secventa
				if (getTypePacket(m) == SEND_INIT) {
                    // daca tipul pachetului este SEND-INIT punem datele in campurile cerute
                    *recv_maxl = (unsigned char) r->payload[4];
                    memcpy(&((recv_time)), r->payload + 5, 2);
                    *recv_eol = (unsigned char) r->payload[8];
                }
                seqNumber = (seqNumber + 1) % 64; // crestem numarul de secventa
				break;
			}
			continue;
		}
		++countTimeouts; // incrementam numarul de incercari
		if(countTimeouts > 2) {
            // am depasit limita de timeout-uri => iesim din program
            printf("[Sender] Number of timeout allowed exceeded!\n");
            exit(1);
        }
	}
}
int main(int argc, char** argv) {
    
    init(HOST, PORT);
    unsigned char sendInitData[MAXL], buffer[MAXL];
    // sendInitData - data din SEND-INIT
    // buffer - data din FILE-DATA
    memset(sendInitData, 0, MAXL); // initializare data din SEND-INIT cu 0 - scap de o treaba
    sendInitData[0] = MAXL; // campul maxl
    unsigned char recv_maxl, recv_eol; // maxl si eol, pentru receiver
    unsigned short time = TIME, recv_time; // recv_time pentru receiver
    if (argc < 2) {
        printf("Please, add more files!\n");
        exit(1);
    }
    memcpy(sendInitData + 1, &time, 2); // campul time
    sendInitData[4] = EOL; // campul eol

    msg send_init, file_header, file_data, end_of_file, end_of_transmission;
    create_packet(&send_init, SEND_INIT, sendInitData, MAXL, seqNumber); // creez pachetul SEND-INIT
    printf("[Sender] Sending SEND-INIT\n");
    send(&send_init, &recv_maxl, &recv_time, &recv_eol); // trimit SEND-INIT
    int i, fd, length;
    for (i = 1; i < argc; ++i)
    {
        fd = open(argv[i], O_RDONLY); // deschid fisierul
        create_packet(&file_header, FILE_HEADER, (unsigned char*) argv[i], strlen(argv[i]) + 1, seqNumber);
        // creez pachetul FILE-HEADER
        printf("[Sender] Sending FILE-HEADER from %s\n", argv[i]);
        send(&file_header, &recv_maxl, &recv_time, &recv_eol);
        // trimit FILE-HEADER

        while((length = read(fd, buffer, MAXL)) > 0) {
            create_packet(&file_data, DATE, buffer, length, seqNumber);
            // creez pachetul FILE-DATA
            printf("[Sender] Sending DATA from %s\n", argv[i]);
            send(&file_data, &recv_maxl, &recv_time, &recv_eol);
            // trimit pachetul FILE-DATA
        }
        create_packet(&end_of_file, EOFZ, 0, 0, seqNumber);
        // creez pachetul END-OF-FILE
        printf("[Sender] Sending END-OF-FILE from %s\n", argv[i]);
        send(&end_of_file, &recv_maxl, &recv_time, &recv_eol);
        // trimit END-OF-FILE
    }
    printf("[Sender] Sending END-OF-TRANSMISSION\n");
    create_packet(&end_of_transmission, EOT, 0, 0, seqNumber);
    // creez pachetul END-OF-TRANSMISSION
    send(&end_of_transmission, &recv_maxl, &recv_time, &recv_eol);
    // trimit END-OF-TRANSMISSION
    printf("[Sender] Transmission ended\n");
    return 0;
}