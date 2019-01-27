#include "header.h"

//SEMAFORI
int res_sem(int semId, int semNum) {
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    return semop(semId, &sops, 1);
}
int rel_sem(int semId, int semNum) {
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    return semop(semId, &sops, 1);
}

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
    printf("Nuova nascita: Processo B || Nome: %s || Genoma: %s || Pid: %d\n", strtok(argv[0], " "), argv[2], getpid());
    struct pers this = {'B', "", atol(argv[2]), getpid(), atoi(argv[1]), 1};
    strcpy(this.name, argv[0]);
    int init_people = atoi(argv[7]);

/*COLLEGAMENTO ALLA MEMORIA CONDIVISA*/
    int mem = atoi(argv[3]);
    struct pers *shmp;
    shmp = (struct pers*)shmat(mem, NULL, 0666);
    if (shmp == (void*) -1) printf("ProcB %s\n",strerror(errno));
    shmp[atoi(argv[1])] = this;

    while (true) { 
    /*INIZIA RICERCA DEI PROCESSI A*/        
        for (int i = 1; i<init_people; i++){
            if (shmp[i].type=='A' && shmp[i].disp==1){
                int sel_pid = shmp[i].pid;
                shmp[this.pos].disp = 0;

                res_sem(atoi(argv[5]), 1); /*attende il semaforo*/

                if (sel_pid == shmp[i].pid){ /*controlla se nell'attesa è cambiato il processo nella posizione selezionata*/
         
                /*INVIO RICHIESTA AL PROCESSO A*/
                    struct msgbuf msg_req;
                    char strmsg[MSG_LEN];
                    sprintf(strmsg, "%d", this.pos);
                    strcpy(msg_req.mtext, strmsg);
                    msg_req.mtype = shmp[i].pos+1;
                    int ris_req;
                    ris_req = msgsnd(atoi(argv[6]), &msg_req, MSG_LEN, 0);
                    if ( ris_req == -1 ) printf("ProcB %s\n",strerror(errno));
                    int gen_a = shmp[i].gen;
                    char name_a[NAME_LEN];
                    strcpy(name_a, shmp[i].name);

                /*ATTESA RISPOSTA DAL PROCESSO A*/
                    int ris_rep; 
                    struct msgbuf msg_rep;
                    ris_rep = msgrcv(atoi(argv[6]), &msg_rep, MSG_LEN, this.pos+1, 0);
                    if ( ris_rep == -1 ) printf("ProcB %s\n",strerror(errno));
                    char ok[2] = "ok";

                    rel_sem(atoi(argv[5]), 1);

                /*RICHIESTA ACCETTATA (messaggio a gestore)*/
                    if (strncmp(msg_rep.mtext, ok, 2) == 0){
                        struct msgbuf_g msg_gen; 
                        msg_gen.mtype = 1;
                        msg_gen.pos = this.pos;
                        shmp[this.pos].p_gen = gen_a;
                        strcpy(shmp[this.pos].p_name, name_a);
                        int ris_gen;
                        ris_gen = msgsnd(atoi(argv[4]), &msg_gen, sizeof(int), 0); 
                        if ( ris_gen == -1 ) printf("ProcB %s\n",strerror(errno));
                        exit(0);                    
                    }
                shmp[this.pos].disp = 1;                   
                }
            }
        }
    }
}
