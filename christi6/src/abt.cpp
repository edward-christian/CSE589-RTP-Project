
//Modules, definitions, global variables and objects.
#include "../include/simulator.h"
#include <iostream>
#include <string.h>
#include <vector>
#define RTT 20.0
#define CHUNKSIZE 20
#define BUFFER 1000
#define AHOST 0
#define BHOST 1
using namespace std;
int ackflag, aseq, bseq;
int check = 0;
int seq = 0;
vector<pkt> pkts;
struct pkt last;
int checksum(struct pkt);
//Initialize Packet.
struct pkt *createPacket(struct msg message){
    struct pkt *packet = new struct pkt;
    (*packet).seqnum = seq;
    (*packet).acknum = seq;
    strcpy((*packet).payload, message.data);
    (*packet).checksum = checksum((*packet));
    return packet;
}
// Initialize Checksum.
int checksum(struct pkt packet){
    char data[CHUNKSIZE];
    strcpy(data, packet.payload);
    int localchecksum = 0;
    int i = 0;
    while(i < CHUNKSIZE && data[i] != '\0'){
        localchecksum += data[i];
        i++;
    }
    localchecksum += packet.seqnum;
    localchecksum += packet.acknum;
    return localchecksum;   
}
// From layer 5, to be passed back.
void A_output(struct msg message){
    cout << "Running A_output..." << endl;
	pkts.push_back(*createPacket(message));
    seq++;
	cout << "Flag prior to if in A_output: " << ackflag << endl;
    if(ackflag == 1){
        ackflag = 0;
        last = pkts.at(aseq);
        int checker = checksum(last);
        cout << "Checksum from A_Output: " << checker << endl;
        tolayer3(AHOST, last);
        starttimer(AHOST, RTT);
    }
}
// From layer 3 to layer 4.
void A_input(struct pkt packet){
    cout << "Last payload: " << last.payload << endl;
    cout << "Packet payload: " << packet.payload << endl;
    cout << "Running A_Input..." << endl;
    cout << "ASeq at A_Input: " << aseq << endl;
    if(packet.acknum == aseq){
        ackflag = 1;
        stoptimer(AHOST);
        aseq++;
    }
    else{
        starttimer(AHOST, RTT);
        tolayer3(AHOST, last);
    }
}
// Triggered with A's timer.
void A_timerinterrupt(){
    cout << "Running A_timerinterrupt..." << endl;
    starttimer(AHOST, RTT);
    tolayer3(AHOST, last);
}  
// Initializer called prior to A's routines.
void A_init(){
    cout << "Running A_init..." << endl;
    ackflag = 1;
    aseq = 0;
}
// From layer 3 to layer 4 at B.
void B_input(struct pkt packet){
    cout << "Running B_Input..." << endl;
	int checker = checksum(packet);
	cout << "Checksum from B_Input: " << checker << endl;
    if(bseq == packet.seqnum && checksum(packet) == packet.checksum){
	    cout << "Checksum success." << endl;
        tolayer5(BHOST, packet.payload);
        pkt *ACK = new struct pkt;
        (*ACK).acknum = bseq;
        (*ACK).checksum = packet.seqnum;
        tolayer3(BHOST, *ACK);
        cout << "ACK Checksum: " << checksum(packet) << endl;
        bseq++;
    }
    else if(bseq != packet.seqnum && checksum(packet) == packet.checksum){
	cout << "Checksum success, but bseq fail." << endl;
        pkt *ACK = new struct pkt;
        (*ACK).acknum = -1;
        (*ACK).checksum = packet.seqnum;
        tolayer3(BHOST, *ACK);
    }
}
//Initializer called prior to B's routines.
void B_init(){
    cout << "Running B_init..." << endl;
    bseq = 0;
}