//Modules, definitions, global variables and objects.
#include "../include/simulator.h"
#include <iostream>
#include <string.h>
#include <vector>
#define RTT 25.0
#define CHUNKSIZE 20
#define BUFFER 1000
#define AHOST 0
#define BHOST 1
using namespace std;
int ackflag, aseq, bseq;
int check = 0;
int seq = 0;
int lastsucess = 0;
int numready = 0;
int lastsequence = 0;
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
    if(numready == 0){
        last = pkts.at(seq);
        tolayer3(AHOST, last);
        cout << seq << endl;
        seq++;
		starttimer(AHOST, RTT);
		cout << "A_output timer begin." << endl;
		numready++;
    }
    else if(numready < getwinsize()){
        last = pkts.at(seq);
        tolayer3(AHOST, last);
        seq = seq + 1;
    	numready++;
    }
}
// From layer 3 to layer 4.
void A_input(struct pkt packet){
	cout << "Running A_input..." << endl;
    ackflag = 1;
    if(packet.acknum == lastsequence + 1){
    	lastsequence++;
        cout << "Packet ACK number: " << packet.acknum << endl; 
    }
	else if(packet.acknum == lastsucess + getwinsize()){
        cout << "BALH" << endl;
        cout << endl;
        lastsucess += getwinsize();
        stoptimer(AHOST);
        numready = 0;
	}
}
// Triggered with A's timer.
void A_timerinterrupt(){
    cout << "Running A_timerinterrupt..." << endl;
    cout << "Numready: " << numready << endl;
    for (int i = lastsequence; i < lastsequence + getwinsize() && i < numready; i++){
        last = pkts.at(i);
        tolayer3(AHOST, last);
    }
    starttimer(AHOST, RTT);
}  
// Initializer called prior to A's routines.
void A_init(){
    cout << "Running A_init..." << endl;
    ackflag = 1;
    aseq = 0;
}
//Transfer from A to B, from layer 3 to layer 4 at B or layer 5.
void B_input(struct pkt packet){
    cout << "Running B_Input..." << endl;
    cout << "B INPUT PACKET: " <<packet.acknum << endl;
    if(bseq == packet.seqnum && checksum(packet) == packet.checksum){
        cout << "Worked for both equal" << endl;
        tolayer5(BHOST, packet.payload);
        pkt *ACK = new struct pkt;
        (*ACK).acknum = bseq;
        (*ACK).checksum = packet.seqnum;
        tolayer3(BHOST, *ACK);
        bseq++;
    }
    else if(bseq != packet.seqnum && checksum(packet) == packet.checksum){
        pkt *ACK = new struct pkt;
        (*ACK).acknum = bseq - 1;
        (*ACK).checksum = packet.seqnum;
        tolayer3(BHOST, *ACK);
    }
}
//Initializer called prior to B's routines.
void B_init(){
    cout << "Running B_init..." << endl;
    bseq = 0;
}