#include "header.h"

/*DICHIARAZIONE VARIABILI GLOBALI*/
int mem, coda, coda2, sem, gens, init_people=0;
int proc_count = 0;
int proc_a_count=0;
int proc_a = 0;
char memaddr[20];
char codaaddr[20];
char coda2addr[20];
char semaddr[20];
char strgens[20];
char stinit_people[20];
int *value;
int *init_type;
int birth_death = 0;
int sim_time = 0;
struct pers *shmp;

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


/*INIZIALIZZAZIONE ARRAY DEI TIPI INIZIALI*/
void init_types(){
    int i,r,flag=0;
    time_t t;                       
    srand((unsigned) time(&t));
    for (i=0; i<init_people; i++){ /*inizializzazione array*/
        r = clock()%2;
        init_type[i] = r;
    }
    for (i=0; i<init_people-1; i++){ /*controllo della diversità*/
        if (init_type[i]!=init_type[i+1])
            flag=1;
    }
    if (flag==0){ /*eventuale cambio di un tipo*/
        if (init_type[0]==0)
            init_type[0]=1;
        else init_type[0]=0;
    }
}

/*CONTROLLO DI DIVERSITA' SUI TIPI (ritorna il numero di processi A)*/
int control(){
    int val = 0;
    for (int i=0; i<init_people; i++){
        if (shmp[i].type == 65)
            val++;
    }
    return val;
}

/*CREA UN NUOVO INDIVIDUO INIZIALE*/
void create_pers(char *arg[7]){
    time_t t;                       
    srand((unsigned) time(&t));
    int r = clock();
    char c = r%26 + 65; /*carattere random per il nome*/
    size_t len = strlen(arg[0]);
    char new_name[NAME_LEN];
    memset(new_name, ' ', sizeof(char)*NAME_LEN);
    strcpy(new_name, arg[0]);
    new_name[len] = c;
    unsigned long gen = r%atoi(arg[2]) + 2; /*numero del genoma*/
    char gens[50];
    sprintf(gens, "%lu", gen);
    char *argv[9] = {new_name, arg[1], gens, arg[3], arg[4], arg[5], arg[6],stinit_people, NULL};
    if (init_type[atoi(arg[1])] == 0)
        execve("./procA", argv, NULL);
    else
        execve("./procB", argv, NULL);
}

/*CREA UN FIGLIO DI UN PROCESSO TERMINATO*/
void create_son (char *arg[8], int mcd){
    printf("\tIl processo con il pid %d è morto\n", shmp[atoi(arg[1])].pid);
    time_t t;
    srand((unsigned) time(&t));
    int r = clock();
    int type = r%2;
    char c = r%26 + 65; /*carattere random per il nome*/
    size_t len = strlen(arg[0]);
    char new_name[NAME_LEN];
    strcpy(new_name, arg[0]);
    new_name[len] = c;
    new_name[len+1] = ' ';
    char gens[50];
    unsigned long gen = r%atoi(arg[2]) + mcd; /*genoma random sommato a mcd*/
    sprintf(gens, "%lu", gen);
    char *argv[9] = {new_name, arg[1], gens, arg[3], arg[4], arg[5], arg[6], stinit_people, NULL};
    if (control()==1 && strcmp(arg[7], "A")==0) /*è terminato l'unico processo A*/
        type=0;
    if (control()==init_people-1 && strcmp(arg[7], "B")==0) /*è terminato l'unico processo B*/
        type=1;
    if (type == 0)
        execve("./procA", argv, NULL);
    else
        execve("./procB", argv, NULL);
}

/*VITA: RICEVE I MESSAGGI E GENERA NUOVI FIGLI*/
void life(){

/*INIZIALIZZAZIONE VARIABILI*/
    int ris, proc_num, mcd_val;
    struct itimerval t_val;
    t_val.it_interval.tv_sec = 0;
    t_val.it_interval.tv_usec = 0;
    t_val.it_value.tv_sec = 1;
    t_val.it_value.tv_usec = 0;

    while (t_val.it_value.tv_sec > 0){

        getitimer(ITIMER_REAL, &t_val); /*aggiornamento secondi rimanenti*/

    //ATTESA DI UN MESSAGGIO DA PARTE DEI PROCESSI A e B
        struct msgbuf_g messaggio;
        ris = msgrcv(coda, &messaggio, sizeof(int), 1, 0);
        if ( ris == -1 ) printf("Gest %s\n",strerror(errno));
        proc_num = messaggio.pos;
        if (shmp[messaggio.pos].type == 'A')
            proc_a_count++;
        strcat(shmp[messaggio.pos].p_name, shmp[messaggio.pos].name);

    //MESSAGGIO RICEVUTO (creazione del nuovo processo)
        value[proc_num] = fork();

        switch(value[proc_num]){
            case -1: 
                printf("Gest %s\n",strerror(errno));
                break;
            case 0: ; 
                char str_pos[4];
                sprintf(str_pos, "%d", messaggio.pos);
                char str_type[2];
                sprintf(str_type, "%c", shmp[messaggio.pos].type);
                char *args[8]={shmp[messaggio.pos].p_name, str_pos, strgens, memaddr, codaaddr, semaddr, coda2addr, str_type};
                create_son(args, mcd(shmp[messaggio.pos].gen, shmp[messaggio.pos].p_gen));
                break;
            default:
                proc_count++;
                sleep(1);
                break;
         }
    }
}

/*HANDLER SIGALARM*/
void handlera(){

/*CONTROLLO DEL TEMPO DI ESECUZIONE*/
    sim_time = sim_time - birth_death;
    if (sim_time < birth_death)
        birth_death = sim_time;

/*UCCISIONE DI UN PROCESSO E GENERAZIONE DI UNO NUOVO*/
    if (sim_time>0){
        printf("\n---UCCISIONE DI UN PROCESSO---\n");  
        printf("   Rimangono %d secondi\n\n", sim_time);        
        res_sem(sem, 1);
        int j=0;
        while (shmp[j].disp!=1 && j<init_people)
            j++;
        if (j<init_people){

            kill(shmp[j].pid, SIGKILL); /*uccisione del processo*/  
       
            char pos[20];
            sprintf(pos, "%d", j);
            init_type[j] = shmp[j].type;            
            value[j] = fork();
            switch(value[j]){
                case -1:
                    printf("Gest %s\n",strerror(errno));
                    exit(0);
                    break;
                case 0: ;
                    char *arg[7]={"", pos, strgens, memaddr, codaaddr, semaddr, coda2addr};
                    create_pers(arg);
                    break;
                default:
                    proc_count++;
                    sleep(1);
                    break;
            }
        } else
            printf("Nessun processo disponibile\n");
        rel_sem(sem, 1);
        alarm(birth_death);
        printf("\n---LA SIMULAZIONE RIPRENDE---\n\n");
        life();

/*FINE SIMULAZIONE*/
    } else {
        printf("\n---SIMULAZIONE TERMINATA---\n");

    /*RICERCA VALORI SULLA MEMORIA CONDIVISA */ 
        int max_gen_pos=0, max_name_len=0, max_gen=0, count_a=0;
        for (int i=0; i<init_people; i++){
            if (shmp[i].gen > max_gen)
                max_gen = shmp[i].gen;
            if (strlen(shmp[i].name) > max_name_len)
                max_name_len = strlen(shmp[i].name);
            if (shmp[i].type == 'A'){
                proc_a_count++;
                proc_a++;
            }
        }

    /*STAMPA STATISTICHE*/
        printf("\nNumero di processi generati:   %d\n", proc_count);
        printf("Numero di processi A:          %d\n", proc_a);
        printf("Numero di processi B:          %d\n", init_people-proc_a);        
        printf("Numero totale di processi A:   %d\n", proc_a_count);
        printf("Numero totale di processi B:   %d\n", proc_count-proc_a_count);
        printf("Caratteristiche dei processi con il nome più lungo:\n");
        for (int i=0; i<init_people; i++){
            if (strlen(shmp[i].name)==max_name_len)
                printf("\t Processo %c || Nome: %s || Genoma: %lu || Pid: %d\n", shmp[i].type, shmp[i].name, shmp[i].gen, shmp[i].pid);
        }
        printf("Caratteristiche dei processi con il genoma più grande:\n");
        for (int i=0; i<init_people; i++){
            if (shmp[i].gen==max_gen)
                printf("\t Processo %c || Nome: %s || Genoma: %lu || Pid: %d\n", shmp[i].type, shmp[i].name, shmp[i].gen, shmp[i].pid);
        }
       
    /*TERMINA PROCESSI E LIBERA RISORSE*/
        for (int i=0; i<init_people; i++)
            kill(value[i], SIGKILL);       
        if (msgctl(coda, IPC_RMID, NULL)<0) printf("Gest %s\n",strerror(errno));
        if (msgctl(coda2, IPC_RMID, NULL)<0) printf("Gest %s\n",strerror(errno));
        if (shmctl(mem, IPC_RMID, NULL)<0) printf("Gest %s\n",strerror(errno));
        if (semctl(sem, 1, IPC_RMID)<0) printf("Gest %s\n",strerror(errno));
        exit(0);
    }
}

/*******************************************************************************************************/

int main() {

/*LETTURA VARIABILI*/
    printf("Numero di individui nella popolazione iniziale (almeno 2)\n");
    while (init_people < 2) { scanf ("%d", &init_people); }
    printf("Tempo di nascita e morte (birth_death)\n");
    scanf ("%d", &birth_death);
    printf("Tempo totale della simulazione (sim_time)\n");
    scanf ("%d", &sim_time); 
    printf("Intervallo del genoma (genes)\n");
    scanf ("%d", &gens);
    sprintf(strgens, "%d", gens); 
    sprintf(stinit_people, "%d", init_people);
    value = malloc (sizeof(int)*init_people);
    init_type = malloc (sizeof(int)*init_people);

/*INIZIALIZZAZIONE SEMAFORI, MEMORIA CONDIVISA E CODA DI MESSAGGI*/        
    mem = shmget(IPC_PRIVATE, (sizeof(struct pers))*init_people, 0666);
    if (mem == -1) printf("Gest %s\n",strerror(errno)); else printf("\n-Shmem id: %d OK\n", mem);

    coda = msgget(IPC_PRIVATE, 0666);
    if (coda == -1) printf("Gest %s\n",strerror(errno)); else printf("-Msg1 id: %d OK\n", coda);

    coda2 = msgget(IPC_PRIVATE, 0666);
    if (coda2 == -1) printf("Gest %s\n",strerror(errno)); else printf("-Msg2 id: %d OK\n", coda2);

    sem = semget(IPC_PRIVATE, 1, 0666);
    if (sem == -1) printf("Gest %s\n",strerror(errno)); else printf("-Sem id: %d OK\n\n", sem);

    sprintf(memaddr, "%d", mem);
    sprintf(codaaddr, "%d", coda);
    sprintf(coda2addr, "%d", coda2);
    sprintf(semaddr, "%d", sem);

/*HANDLER DEI SEGNALI*/
    signal(SIGALRM, handlera);

/*INIZIALIZZAZIONE DELL'ARRAY DEI TIPI INIZIALI (garantisce la diversità)*/
    init_types();

/*GENERAZIONE DELLA POPOLAZIONE INIZIALE*/
    int j;
    for (j=0; j<init_people; j++){
        value[j] = fork();
        char pos[20];
        sprintf(pos, "%d", j);

        switch(value[j]){
            case -1: 
                printf("Gest %s\n",strerror(errno));
                break;
            case 0:
                raise(SIGSTOP);
                char *arg[7]={"", pos, strgens, memaddr, codaaddr, semaddr, coda2addr};
                create_pers(arg);
                break;
            default:
                proc_count++;
                waitpid(0, NULL, WUNTRACED);
                break;
        }
    }
    kill(0, SIGCONT);
    sleep(1);

/*COLLEGAMENTO ALLA MEMORIA CONDIVISA*/
    shmp = (struct pers*)shmat(mem, NULL, 0666);
    if (shmp == (void*) -1) printf("Gest %s\n",strerror(errno));

/*INIZIO TIMER*/
    if (birth_death>sim_time)
        alarm(sim_time);
    else
        alarm(birth_death);

/*INIZIO DELLA VITA*/
    printf("\n---INIZIO SIMULAZIONE---\n\n");
    life();
}
