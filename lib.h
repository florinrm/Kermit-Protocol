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

void createMessageType1 (msg *m) {
    package pkt;
    dataPackage data;
    m->len = 0;
    memset(m->payload, 0, sizeof(m->payload));

    pkt.soh = SOH;
    pkt.len = 5 + sizeof(data);
    pkt.seq = SEQ;
    pkt.type = SEND_INIT;

    data.maxl = MAXL;
    data.time = TIME / 1000;
    data.npad = NPAD;
    data.pdac = PADC;
    data.capa = 0;
    data.r = 0;
    data.eol = EOL;
    data.chkt = 0;
    data.qbin = 0;
    data.qctl = 0;
    data.rept = 0;

    memcpy (pkt.data, &data, sizeof(data));

    pkt.check = crc16_ccitt (&pkt, sizeof(pkt) - 3); // -3 = ignoram ultimii 3 membri din struct package
    pkt.mark = EOL;

    memcpy(m->payload, &pkt, sizeof(pkt));
    m->len = sizeof(pkt);
} // mesaj de tip S (start init)

char getTimeMax (msg *m) {
    return m->payload[5];
}

void createMessageType2 (msg *m, char* fileName) {
    package pkt;
    m->len = 0;
    memset(m->payload, 0, sizeof(m->payload));
    pkt.soh = SOH;
    pkt.len = 5 + strlen(fileName);
    pkt.seq = SEQ;
    pkt.type = FILE_HEADER;

    memcpy (pkt.data, &fileName, sizeof(fileName));
    pkt.check = crc16_ccitt (&pkt, sizeof(pkt) - 3);
    pkt.mark = EOL;

    memcpy(m->payload, &pkt, sizeof(pkt));
    m->len = sizeof(pkt);
} // file header

void createMessageType3 (msg *m, char type) {
    package pkt;
    m->len = 0;
    memset(m->payload, 0, sizeof(m->payload));
    pkt.soh = SOH;
    pkt.len = 5;
    pkt.seq = SEQ;
    pkt.type = type;
    memcpy(pkt.data, 0, sizeof(pkt.data));
    pkt.check = crc16_ccitt (&pkt, sizeof(pkt) - 3);
    pkt.mark = EOL;

    memcpy(m->payload, &pkt, sizeof(pkt));
    m->len = sizeof(pkt);
} // NAK, EOF, EOT

void createMessageType4 (msg *m, char* data) {
    package pkt;
    m->len = 0;
    memset(m->payload, 0, sizeof(m->payload));
    pkt.soh = SOH;
    pkt.len = 5;
    pkt.seq = SEQ;
    pkt.type = DATE;
    memcpy(pkt.data, &data, sizeof(data));
    pkt.check = crc16_ccitt (&pkt, sizeof(pkt) - 3);
    pkt.mark = EOL;

    memcpy(m->payload, &pkt, sizeof(pkt));
    m->len = sizeof(pkt);
} // data type package

void createMessageType5 (msg *m, char* data) {
    package pkt;
    m->len = 0;
    memset(m->payload, 0, sizeof(m->payload));
    pkt.soh = SOH;
    pkt.len = 5;
    pkt.seq = SEQ;
    pkt.type = ACK;
    memcpy(pkt.data, &data, sizeof(data));
    pkt.check = crc16_ccitt (&pkt, sizeof(pkt) - 3);
    pkt.mark = EOL;

    memcpy(m->payload, &pkt, sizeof(pkt));
    m->len = sizeof(pkt);
} // ACK message


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

void prepare_packet(msg * _packet, uint8_t _type, uint8_t * _data, uint8_t _data_len, uint8_t numar_secventa) {
    uint8_t mark = 0x0D;
    sprintf(_packet->payload, "%c%c%c%c", SOH,
                                        5 + _data_len,
                                        numar_secventa,
                                        _type);
    memcpy(_packet->payload + 4, _data, _data_len);
    unsigned short crc = crc16_ccitt(_packet->payload, 254);
    memcpy(_packet->payload + 254, &crc, 2);
    memcpy(_packet->payload + 256, &mark, 1);
    _packet->len = sizeof(_packet->payload);
}

#endif

