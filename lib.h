#ifndef LIB
#define LIB

#define MAXL 250
#define TIME 5000

#include <stdlib.h>
#include <string.h>

#define NAK 'N'
#define DATE 'D'
#define EOFZ 'Z'
#define EOT 'B'
#define ACK 'Y'
#define SEND_INIT 'S'
#define FILE_HEADER 'F'

#define SOH 0x01
#define SEQ 0x00

#define NPAD 0x00
#define EOL 0x0D
#define PADC 0x00

typedef struct {
    int len;
    char payload[1400];
} __attribute__((packed)) msg;

typedef struct {
    char maxl;
    char npad;
    char time;
    char pdac;
    char eol;
    char qctl, qbin, chkt, rept, capa, r;
} __attribute__((packed)) dataPackage;

typedef struct {
    char soh; // start of header
    char len; // lungimea pachetului
    char seq; // numar de secventa
    char type;
    char data[MAXL];
    unsigned short check;
    char mark;
} __attribute__((packed)) package;

void init(char* remote, int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);
msg* receive_message_timeout(int timeout); //timeout in milliseconds
unsigned short crc16_ccitt(const void *buf, int len);


char getTimeMax (msg *m) {
    return m->payload[5];
}

char getTypePacket (msg *m) {
    return m->payload[3];
} // tipul pachetului

char getSequence (msg *m) {
    return m->payload[2];
}

void setSequence (msg *m, char seq) {
    m->payload[2] = seq;
}

char getMAXL (msg *m) {
    return m->payload[4];
} // iau maxl-ul

unsigned char dataLength (msg *m) {
    return (unsigned char) m->payload[1] - 5;
} // lungimea campului data

int checkCRC (msg *m) {
    unsigned short calculatedCRC = crc16_ccitt(m->payload, 254);
    return (memcmp (&calculatedCRC, m->payload + 254, 2) == 0);
} // verific daca CRC-ul e calculat ok

void create_packet(msg * packet, unsigned char type, unsigned char * data, unsigned char dataLength, unsigned char seqNumber) {
    unsigned char mark = EOL;
    //sprintf(packet->payload, "%c%c%c%c", SOH, 5 + dataLength, seqNumber, type); 
    // setez primii 4 biti - ceruti in enunt
    packet->payload[0] = SOH;
    packet->payload[1] = 5 + dataLength;
    packet->payload[2] = seqNumber;
    packet->payload[3] = type;
    memcpy(packet->payload + 4, data, dataLength); // setez lungimea data
    unsigned short crc = crc16_ccitt(packet->payload, 254);
    memcpy(packet->payload + 254, &crc, 2); // setez CRC-ul
    memcpy(packet->payload + 256, &mark, 1); // setez mark-ul
    packet->len = sizeof(packet->payload); // setez marimea pachetului
} // creare de pachet nou

#endif

