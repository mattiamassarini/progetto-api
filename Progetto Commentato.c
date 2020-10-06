#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define maximum 15000
#define maxstring 15000
//Questo simulatore di una macchina di Turing non deterministica fa uso di una lista di adiacenza (array di liste) in cui vengono salvate le transizioni. 
//All' n-esimo indice dell'array vengono salvate tutte le transizioni che vanno da n in m. 
//L'algoritmo usato per la simulazione è il Breadth First Search Algorithm (BFS). Ad ogni passo di esecuzione, in base al simbolo letto e allo stato corrente, 
//vengono inserite in coda tutte le possibili vie percorribili. L'esecuzione si ferma quando viene trovato uno stato di accettazione oppure quando la coda è vuota.
//Nel caso in cui un percorso raggiunga il numero massimo di passi la simulazione ritorna U solamente se tutti le vie rimanenti non vanno in uno stato di accettazione.
//Dopo aver stampato l'esito della singola stringa, tutte le strutture, salvo la lista di adiacenza, vengono liberate e nel caso non si sia raggiunto l'EOF si prosegue con
//la stringa successiva.

typedef struct node {                        //struct che contiene la singola transizione
    short int current_state;
    char read;
    char write;
    char move;
    short int new_state;
    short int acc_state;
    struct node * next_s;
} node_a;
typedef struct tape{                         //struct associata ad ogni singolo stato di esecuzione: busy indica il numero di structs execution che leggono dalla memoria puntata
 	short int busy;                          //dai puntatori a char left e right.
	char * left;
	char * right;
}tape_r;
typedef struct execution{                    //ad ogni passo deterministico corrisponde un blocco execution che contiene la stringa letta ad un generico "tempo" t, un pointer
	struct tape * string;                    //che funge da testina, lo stato in cui si trova la macchina al tempo t, il numero di passi rimanenti, e il puntatore alla stringa originaria.
	char * pointer;
	short int initial_state;
	int steps;
	char *link;
	struct execution * next;

}execution_l;
struct execution *enqueue(struct execution *head,short int initial_state, struct tape *head_string,int steps,char*string){   //mette in coda il primo elemento execution
	struct execution *new_node=NULL;
	new_node=malloc(sizeof(struct execution));
	new_node->initial_state=initial_state;
	new_node->steps=steps;
	new_node->string=head_string;
	new_node->pointer=new_node->string->right;
	new_node->link=string+1;
	new_node->next=NULL;
	return new_node;
}
struct node * create_node(int current_state,char read,char write, char move,int new_state){      //alloca un nodo transizione e ne ritorna il puntatore
	struct node* new_node = malloc(sizeof(struct node));
	new_node->current_state = current_state;
	new_node->read=read;
	new_node->write=write;
	new_node->move=move;
	new_node->new_state=new_state;
	if((new_node->current_state==new_node->new_state)&(new_node->move=='S')&(new_node->read==new_node->write)){   //se la singola transizione è un autoanello (es. 0 a a S 0) viene marcato -1.
		new_node->acc_state=-1;                                                                                   //La singola esecuzione deterministica finirà il numero di passi
	} else if((new_node->read=='_')&(new_node->current_state==new_node->new_state)){                              //Controllo aggiuntivo per i loop che spingono la macchina ad andare sempre a dx (sx)
		new_node->acc_state=-1;                                                                                   //La singola esecuzione deterministica finirà anche in questo caso il numero massimo di passi.
	}else  new_node->acc_state=0;
	new_node->next_s = NULL;
	return new_node;
}

struct execution *dequeue(struct execution *head) {           //Presa in ingresso una struct execution, controlla se la stringa ad esso associata è letta da più structs execution.
	struct execution *temp=NULL;                              // In caso affermativo busy viene decrementato e viene eliminata solo la struct execution, altrimenti, in aggiunta, viene liberata
	temp=head->next;                                          // la memoria riservata alla stringa.
	head->string->busy--;
	if(head->string->busy==0) {
		free(head->string->left);
		free(head->string);
	}
	free(head);
	return temp;
}
void erase_exec(struct execution *head){                    //Come dequeue ma elimina tutta la coda di esecuzione.
	struct execution *temp=NULL;                            
	temp=head;
			while(temp!=NULL){
				temp=temp->next;
				head->string->busy--;
				if(head->string->busy==0) {
					free(head->string->left);
					free(head->string);
				}
				free(head);
				head=temp;
			}
}

void shift2(char*left,char*right){            //Funzione che scorre i caratteri della stringa di 1 posizione verso destra.
	char*temp=right-1;
	char*temp2=right;
	while(temp2!=left){
		*temp2=*temp;
		temp--;
		temp2--;
	}
}

struct execution*dup_keep_string(struct execution*head){   //Funzione che duplica la singola struct execution nel caso in cui la stringa di caratteri del padre non venga modificata
	struct execution *new_node=NULL;                       // dalla duplicazione. (Ad esempio se leggo e scrivo lo stesso carattere e la testina rimane ferma).
	new_node=malloc(sizeof(struct execution));             
	new_node->initial_state=head->initial_state;
	new_node->next=NULL;
	new_node->link=head->link;
	new_node->steps=head->steps;
	new_node->steps--;
	new_node->string=head->string;
	new_node->pointer=head->pointer;
	new_node->string->busy++;
	return new_node;
}
struct execution*duplicate_exec(struct execution*head){   //Funzione che duplica la singola struct execution, crea una struct node a cui viene assegnata una nuova stringa.
	struct execution *new_node=NULL;                      
	int g;
	g=(head->string->right - head->string->left)+1;
	new_node=malloc(sizeof(struct execution));
    new_node->link=head->link;
	new_node->initial_state=head->initial_state;
	new_node->steps=head->steps;
	new_node->steps--;
	new_node->string=malloc(sizeof(struct tape));
	new_node->string->left=malloc(g*sizeof(char));
	memcpy(new_node->string->left,head->string->left,g);
	new_node->string->right=new_node->string->left+g-1;
	g=head->pointer-head->string->left;
	new_node->pointer=new_node->string->left+g;
	new_node->string->busy=1;
	new_node->next=NULL;
	return new_node;
}
int main(){
	int flag=-1,i=0,g,risflag=0,risultato=0,current_state,next_state,accept;
	int steps;
	struct node *primary_array[maximum];   //array statico della lista di adiacenza
	node_a *second=NULL;                   //puntatore che scorre la lista secondaria
	char r,w,m;                       
	char string[maxstring];
	char *temp;
    tape_r * std_head_tape=NULL;               
    struct execution*head_queue=NULL;    //puntatore alla testa della coda di esecuzione
    struct execution *current_exec=NULL;   //puntatore all'elemento che viene eseguito a tempo t.
    struct execution *new_node_exec=NULL; //puntatore all'elemento che viene eventualmente creato durante l'esecuzione.
    struct execution *end_queue=NULL;    //puntatore che punta a fine coda per aggiungere gli elementi duplicati.

		g=fscanf(stdin,"%s ",string);     
		while(strcmp("acc",string)!=0){          //salvo l'informazione della singola transizione in 5 variabili (es. 0 a b 1 R), le salvo in un nodo creato dinamicamente
			g=fscanf(stdin,"%s ",string); 
			if(i==0) current_state= atoi(string);
				if(i==1) r=string[0];
				if(i==2) w=string[0];
			    if(i==3) m=string[0];
			    if(i==4) {
			    	next_state=atoi(string);
			    	if(primary_array[current_state]==NULL) {
			    		primary_array[current_state]=create_node(current_state,r,w,m,next_state);
			    		}
			    	else{
			    		second=primary_array[current_state];
			    		while(second->next_s!=NULL){
			    			second=second->next_s;
			    		} 
			    		second->next_s=create_node(current_state,r,w,m,next_state);
			    	} 
			    	i=-1;
			    } i++;
		}
		g=fscanf(stdin,"%s ",string);
		while(strcmp("max",string)!=0){
				accept=atoi(string);
		   	 	g=0;
		   	 	while(g<maximum){
		   	 		if(primary_array[g]!=NULL){
		   	 			second=primary_array[g];
		   	 			while(second!=NULL){
		   	 				if(second->new_state==accept) second->acc_state=1;
		   	 				second=second->next_s;
		   	 			}
		   	 		}
		   	 		g++;
		   	 	}g=fscanf(stdin,"%s ",string);
		}   
		g=fscanf(stdin,"%s ",string);
		steps=atoi(string);
	    g=fscanf(stdin,"%s ",string);
		while(!feof(stdin)){
			g=fscanf(stdin,"%s ",string);
			std_head_tape=malloc(sizeof(struct tape));
			std_head_tape->right=malloc(sizeof(char));
			*(std_head_tape->right)=string[0];
			std_head_tape->left=std_head_tape->right;
			std_head_tape->busy=1;
			if(head_queue!=NULL) erase_exec(head_queue);
			head_queue=NULL;
			head_queue=enqueue(head_queue,0,std_head_tape,steps,string);    //inserisco nella coda il primo passo di computazione (stato in 0, prima lettera della stringa)
			std_head_tape=NULL;
			current_exec =head_queue;  
			risflag=0;
			risultato=0;
			end_queue=current_exec;
            while((current_exec!=NULL)&&(risultato!=1)){
                head_queue=current_exec;
                second=primary_array[current_exec->initial_state];
                if(current_exec->steps==0) {
                		risflag=1;
                		second=NULL;
                    }
                while(second!=NULL){
                	
                	if(second->read==*(current_exec->pointer)){       
                	if(second->acc_state==1) risultato=1;
                	new_node_exec=NULL;
                if(second->acc_state!=-1){
                	if(second->move=='R') {                               //se sposto la testina a destra
				if((second->read==second->write)&(current_exec->pointer!=current_exec->string->right)){     //se leggo e scrivo lo stesso carattere e sono a fine stringa a dx
                		new_node_exec=dup_keep_string(current_exec);                                        //mantengo la memoria
                	} else new_node_exec=duplicate_exec(current_exec);                                      //altrimenti duplico sia l'esecuzione che la memoria
					    new_node_exec->initial_state=second->new_state;
                    *(new_node_exec->pointer)=second->write;	
                if((new_node_exec->pointer==new_node_exec->string->right)&&(*(new_node_exec->link)!='\0')) {  //se sono a fine string a dx, e la stringa originale non è stata processata
                g=(new_node_exec->string->right-new_node_exec->string->left)+2;                               //completamente, aggiungo un ulteriore carattere
				new_node_exec->string->left=realloc(new_node_exec->string->left,g); 
                new_node_exec->string->right=(new_node_exec->string->left+g)-1;
                new_node_exec->pointer=(new_node_exec->string->right)-1;
                *(new_node_exec->string->right)=*(new_node_exec->link);
                new_node_exec->link++;
                new_node_exec->pointer++;
                } else if((new_node_exec->pointer==new_node_exec->string->right)&&(*(new_node_exec->link)=='\0')){  //se sono a fine stringa, e ho finito di leggere tutta la strina originale
								if((*(new_node_exec->pointer)=='_') & (second->write=='_')){   //controllo per limitare un numero eccessivo di blank a dx.
									dequeue(new_node_exec);
									new_node_exec=NULL;
								}else {
									g=(new_node_exec->string->right-new_node_exec->string->left)+2;    //altrimenti alloco nuova memoria aggiuntiva e aggiungo il blank a dx.
                		    new_node_exec->string->left=realloc(new_node_exec->string->left,g);
                		    new_node_exec->string->right=(new_node_exec->string->left+g)-1;
                		    new_node_exec->pointer=new_node_exec->string->right-1;
                		    *(new_node_exec->string->right)='_';
                		    new_node_exec->pointer++;
								}
                		    }else  
							new_node_exec->pointer++;
							 }
							 
                else if(second->move=='L') {                                                               //se sposto la testina a sinistra
                    if((second->read==second->write)&(current_exec->pointer!=current_exec->string->left)){ //se non modifico la stringa e non creo un blank andando a sx
                		     	new_node_exec=dup_keep_string(current_exec);                               //duplico mantenendo la stringa
							} else new_node_exec=duplicate_exec(current_exec);                             //altrmenti duplico tutto
						new_node_exec->initial_state=second->new_state;
                    *(new_node_exec->pointer)=second->write;
                    if(new_node_exec->pointer==new_node_exec->string->left){  //se sono ad inizio stringa 
                    
                    if((*(new_node_exec->pointer)=='_')&(second->write=='_')){   //controllo per evitare eccessivi blank a sx
                    	         dequeue(new_node_exec);
									new_node_exec=NULL;
					}else {
						g=(new_node_exec->string->right-new_node_exec->string->left)+2;
                    new_node_exec->string->left=realloc(new_node_exec->string->left,g);
                    new_node_exec->string->right=(new_node_exec->string->left+g)-1;
                    shift2(new_node_exec->string->left,new_node_exec->string->right);
                    *(new_node_exec->string->left)='_';
                    new_node_exec->pointer=new_node_exec->string->left;
					}
                    } else new_node_exec->pointer--;
                } 
                else {              //se la testina è ferma
                	
                		if(second->read==second->write) new_node_exec=dup_keep_string(current_exec);      //se non modifico il carattere letto duplico e condivido la memoria 
                	else new_node_exec=duplicate_exec(current_exec);                                      //altrimenti duplico tutto
                	new_node_exec->initial_state=second->new_state;
                    *(new_node_exec->pointer)=second->write;
					
				}
				} else risflag=1;         //se la transizione è marcata -1, sicuramente la stringa letta ammette un percorso deterministico per la quale termina i passi.
                		if(new_node_exec!=NULL){
                			end_queue->next=new_node_exec;
                		new_node_exec->next=NULL;
                		end_queue=end_queue->next;
						}
                		     }
						     second=second->next_s;
                		   }
                	current_exec=current_exec->next;
                	head_queue=dequeue(head_queue);
                }
                if(risultato==1) printf("1\n");       //se risultato = 1, la stringa è accettata
                else if(risflag==1) printf("U\n");    //se risultato = 0 e risflag=1, la stringa è indeterminata
                else printf("0\n");                   //se risultato = 0 e risflag=0, la stringa non è accettata
			    
		}
return 0;
}
