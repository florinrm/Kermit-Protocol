Tema 1 - Protocoale de comunicatii - Kermit Protocol

Nume: Mihalache Florin-Razvan
Grupa: 323CC

ksender: 
Am o functie (send), in care trimit un mesaj m la kreceiver si am un mesaj
r de la kreceiver, cu ajutorul caruia verific daca a fost timeout sau nu.
Daca a fost timeout, incerc sa trimit din nou mesajul la kreceiver. Daca
am incercat de 3 ori sa trimit mesajul la kreceiver fara succes (timeout),
ies din program. Daca kreceiver-ul primeste mesajul, se verifica daca r 
este un mesaj de tip ACK si totodata verific daca numarul de secventa al 
mesajului r este egal cu numarul de secventa din ksender pe care il incrementez
ori de cate ori se trimite cu succes un mesaj de la ksender la kreceiver. Daca
mesajul trimis este de tip SEND-INIT, ii atribui acestuia maxl, eol si time 
(cerute la pachetele de tip SEND-INIT).
In main creez mesajul de tip SEND-INIT, caruia ii atribui via sirului SendInitData
time, eol si maxl (acestea 3 mereu vor fi atribuite unui pachet cand il trimit
la kreceiver). Iterez printre parametrii dati in linia de comanda aka fisierele
transmise (trimit FILE-HEADER aka denumirea acestora, apoi citesc din ele
si trimit datele din acestea prin pachete de tip FILE-DATA si dupa ce termin
ce citit din fisier creez si trimit pachetul END-OF-FILE, care marcheaza sfarsitul
unui fisier). Dupa ce am terminat cu iterarea prin fisiere, creez si trimit 
pachetul END-OF-TRANSMISSION, care marcheaza sfarsitul transmisiei catre kreceiver.

kreceiver:
Intr-un infinite loop, verific mai intai daca se conecteaza kreceiver-ul la ksender.
Daca nu, incerc sa ma reconectez, altfel primesc pachetul de la receiver. Timeout-ul
il tratez in acelasi mod ca la ksender. Apoi, daca mesajul e trimis cu bine la
kreceiver, verific daca acesta are CRC-ul calculat bine (daca e facut prost, trimit
NAK la ksender + continue la infinite loop, ca sa il primesc din nou). Daca e CRC-ul
ok, vad daca mesajul primit este de tip SEND-INIT ca sa vad cum fac ACK-ul (daca
ii pun la informatii la campul data sau nu). Incrementez numarul de secventa si
apoi verific ce tip de mesaj este cel primit (in afara de SEND-INIT). Daca e de
tip FILE-HEADER, creez fisierul de output pentru fisierul de input, daca e de tip
FILE-DATA scriu informatiile din fisierul de input in cel de output, daca e de
tip END-OF-FILE inchid fisierul de output si daca e de tip END-OF-TRANSMISSION
inchid transmisia mesajelor.

lib: aici am o functie (create_packet), in care este creat un pachet nou. Functia
ia ca parametri pachetul pe care il creez, tipul acestuia, data aka 
informatia (care conteaza de fapt la SEND-INIT, FILE-HEADER si FILE-DATA), 
lungimea de la data si numarul de secventa, toate acestea atribuindu-le 
pachetului (de fapt payload-ul acestuia, in len punand marimea payload-ului).
Totodata, calculez crc-ul pachetului si il atribui la payload si EOL la mark.
