#include "header.h"

//CALCOLO MCD DI DUE INTERI
int mcd(int a, int b){
    while (a != b){
        if (a > b)
            a = a-b;
        else
            b = b-a;
    }
    return a;
}

int main(int argc, char *argv[]){    

/*CREAZIONE STRUTTURA CHE DESCRIVE L'INDIVIDUO E STAMPA INFORMAZIONI*/
    printf("Nuova nascita: Processo A || Nome: %s || Genoma: %s || Pid: %d\n", strtok(argv[0], " "), argv[2], getpid());
    struct pers this = {'A', "", atol(argv[2]), getpid(), atoi(argv[1]), 1};
    strcpy(this.name, argv[0]);

/*COLLEGAMENTO E SCRITTURA SULLA MEMORIA CONDIVISA*/
    int mem = atoi(argv[3]);
    struct pers *shmp;
    shmp = (struct pers*)shmat(mem, NULL, 0666);
    if (shmp == (void*) -1) printf("ProcA %s\n",strerror(errno));
    shmp[atoi(argv[1])] = this;

/*INIZIALIZZAZIONE VARIABILI*/
    int tol = 2, count = 0; /*gestione tolleranza*/
    int ris_req, ris_rep; /*valori di ritorno msgsnd e msgrcv*/

    while (true){
    /*ATTESA RICHIESTA DA PARTE DI UN PROCESSO B*/
        struct msgbuf msg_req;
        ris_req = msgrcv(atoi(argv[6]), &msg_req, MSG_LEN, this.pos+1, 0);
        if ( ris_req == -1 ) printf("%s\n",strerror(errno));

    /*RICHIESTA RICEVUTA (analisi del genoma del procB)*/
        if (this.gen%shmp[atoi(msg_req.mtext)].gen == 0 || /*genoma del procB multiplo o rientra nella tolleranza, accetta*/
            mcd(shmp[atoi(msg_req.mtext)].gen, this.gen) > this.gen/tol){ 
           
            shmp[this.pos].disp = 0;  
            char name_b[NAME_LEN];
            strcpy(name_b, shmp[atoi(msg_req.mtext)].name);
            int gen_b = shmp[atoi(msg_req.mtext)].gen;
            struct msgbuf msg_rep;
            char strmsg[MSG_LEN] = "ok";
            strcpy(msg_rep.mtext, strmsg);
            msg_rep.mtype = (shmp[atoi(msg_req.mtext)].pos)+1;
            ris_rep = msgsnd(atoi(argv[6]), &msg_rep, MSG_LEN, 0); 
            if ( ris_rep == -1 ) printf("ProcA %s\n",strerror(errno));

        /*MESSAGGIO AL GESTORE*/
            struct msgbuf_g msg_gen; 
            msg_gen.mtype = 1;
            msg_gen.pos = this.pos;
            shmp[this.pos].p_gen = gen_b;
            strcpy(shmp[this.pos].p_name, name_b);
            int ris_gen;
            ris_gen = msgsnd(atoi(argv[4]), &msg_gen, sizeof(int), 0);
            if ( ris_gen == -1 ) printf("ProcA %s\n",strerror(errno));
            exit(0);                    

        } else {  /*genoma del procB non rientra nella tolleranza, rifiuta*/
            struct msgbuf msg_rep;
            char strmsg[MSG_LEN] = "no";
            strcpy(msg_rep.mtext, strmsg);
            msg_rep.mtype = (shmp[atoi(msg_req.mtext)].pos)+1;
            ris_rep = msgsnd(atoi(argv[6]), &msg_rep, MSG_LEN, 0); 
            if ( ris_rep == -1 ) printf("ProcA %s\n",strerror(errno));
            count++;
            if (count>2){ /*aumenta tolleranza*/
                tol = tol*2;
                count = 0;
            }
            shmp[this.pos].disp = 1;   
        }
    }
}
