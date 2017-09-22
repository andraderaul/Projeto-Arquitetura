#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

enum{ //pra n precisar ficar digitando R[32] até R[37]
	
	//memoriaCache
	set0 = 0,
	set1 = 1,
	read = 1,
	write = 2,
	
	//fpu
	X = 0,
	Y = 1, 
	Z = 2,
	
	//registrador	
	PC = 32,
	IR = 33,
	ER = 34,
	FR = 35,
	CR = 36,
	IPC = 37

};
//estrutura das instrucoes
struct instructions{
	
	uint32_t y,x,z;
	uint32_t im16, im26;
	uint32_t overflowY,overflowX,overflowZ;
	
};
//estrutura do watchdog
struct watchdogTimer{
	
	uint32_t EN;
	uint32_t counter;
};
//estrutura do terminal
struct term{
	
	char c;
	char ter;

};
//estrutura da fpu
struct floatPointUnit{	

	uint32_t control;
	uint32_t status;
	uint32_t op;
	uint32_t ciclo;
	int relogio;

};
//AI É DOIDEIRA 
float auxFPU[3]; //???
uint32_t rFPU[3]; // ????
//estrutura da MemoriaCache
struct memoryCache{
	
	uint32_t v; //validaçao
	uint32_t i; //idade
	uint32_t id; //identificação
	uint32_t dados[4]; //vetor de palavras

};
//estrutura da solicitacao de dado para leitura pelo processador
struct accumulator{
	
	double miss; //erros
	double hit; //acertos

};

int ativacaoFPU = 0;
int ativacaoWatch = 0;
int prioridade = 2;
//passinhofoda2014 //primeira unidade
struct instructions tipoU;
struct instructions tipoF;
struct instructions tipoS;
//nois é bom mas ne bombom //segunda unidade
struct watchdogTimer watchdog; 
struct term terminal;
struct floatPointUnit fpu;
//nois é papai mas ne noel //terceira unidade
struct memoryCache cacheI[8][2]; //cacheI 
struct memoryCache cacheD[8][2]; //cacheD 
struct accumulator acumI; //acumulador de erros e acertos da cacheI
struct accumulator acumD; //acumulador de erros e acertos da cacheD
//Chega de singularidade pelo amor de deus
uint32_t *memoria;
uint32_t R[38]; //31 registradores de proposito geral
uint32_t opcode;
uint32_t pczinho = 0;
unsigned int cont = 0; //contador das linhas do arquivo

//start
void inicializando(){
	int i,j,k;
	for(i = 0; i < 38; i++ ){ //iniciando Registrador
		R[i] = 0;
	}
	for(i = 0; i < 3; i++ ){ //iniciando a FPU
		rFPU[i] = 0;
		auxFPU[i] = 0;
	}
	for(i = 0; i < 8; i++){ //iniciando a Cache
		for(j = 0; j < 2; j++){
			
			//cacheD
			cacheD[i][j].v = 0;
			cacheD[i][j].i = 0;
			cacheD[i][j].id = 0;
			for(k = 0; k < 4;k++){
				cacheD[i][j].dados[k] = 0;
			}
			
			//cacheI
			cacheI[i][j].v = 0;
			cacheI[i][j].i = 0;
			cacheI[i][j].id = 0;
			for(k = 0; k < 4;k++){
				cacheI[i][j].dados[k] = 0;
			}		
		}
	}
	//iniciando acumulador da cacheD
	acumD.hit = 0;
	acumD.miss = 0;
	
	//iniciando acumulador da cacheI
	acumI.hit = 0;
	acumI.miss = 0;

}

void tipoUFS(uint32_t hexa){//isso daqui tirou muita repeticão de codigo <3
	
	//Formato de instrucoes tipo U
		//pegando os operandos
	tipoU.y = (hexa<<27)>>27;
	tipoU.x = (hexa<<22)>>27; //(22+5)
	tipoU.z = (hexa<<17)>>27; //(17+10)
		//salvando os 3 bits de extensão
	tipoU.overflowY = ((hexa<<16)>>31)<<5;
	tipoU.overflowX = ((hexa<<15)>>31)<<5;
	tipoU.overflowZ = ((hexa<<14)>>31)<<5;
		//adicionando o bit mais significativo
	tipoU.x = tipoU.x | tipoU.overflowX;
	tipoU.y = tipoU.y | tipoU.overflowY;
	tipoU.z = tipoU.z | tipoU.overflowZ;
		
	//Formato de instrucoes tipo F
	tipoF.y = (hexa<<27)>>27;
	tipoF.x = (hexa<<22)>>27; 
	tipoF.im16 = (hexa<<6)>>16;
		
	//Formato de instrucoes tipo S
	tipoS.im26 = (hexa<<6)>>6;
	
}
void setarR0(){	
	R[0] = 0x00000000;	
}
void setarIE(FILE *output){
	
	if(R[FR]  & 0x00000040){
		R[IPC] = pczinho;
		R[CR] = 0x00000001;
		setarR0();	
		//famoso desvio
		pczinho = 0x00000003;
		fprintf(output,"[SOFTWARE INTERRUPTION]\n");
	}		
}		
void imprimeRegistradores(uint32_t r,uint32_t esc, FILE* output){
	//1 minusculo //0 maiusculo	
	switch(r){		
		case PC: //PC
			if(esc){
				fprintf(output,"pc");
			}
			else{
				fprintf(output,"PC");
			}
		break;
		case IR: //IR
			if(esc){
				fprintf(output,"ir");
			}
			else{
				fprintf(output,"IR");
			}
		break;
		case ER: //ER
			if(esc){
				fprintf(output,"er");
			}
			else{
				fprintf(output,"ER");
			}
		break;
		case FR: //FR
			if(esc){
				fprintf(output,"fr");
			}
			else{
				fprintf(output,"FR");
			}			
		break;
		case CR: //CR
			if(esc){
				fprintf(output,"cr");
			}
			else{
				fprintf(output,"CR");
			}
		break;	
		case IPC: //CR
			if(esc){
				fprintf(output,"ipc");
			}
			else{
				fprintf(output,"IPC");
			}
		break;	
		default: //Numeros
			
			if(esc){
				fprintf(output,"r%d",r);
			}
			else{
				fprintf(output,"R%d",r);
			}
			
		break;
			
	}
}
void setarFR(uint32_t x,uint32_t y){ //funcao_PATO
	//add, addi
	if(opcode == 0x00 || opcode == 0x01){
		if(((uint64_t)x + (uint64_t)y) > UINT32_MAX){
			R[FR] = R[FR] | 0x00000010;
		}
		else{
			R[FR] = R[FR] & 0x0000006F;
		}	
	}	
	//sub, subi
	if(opcode == 0x02 || opcode == 0x03){
		if(((uint64_t)x - (uint64_t)y) > UINT32_MAX){
			R[FR] = R[FR] | 0x00000010;
		}
		else{
			R[FR] = R[FR] & 0x0000006F;
		}	
	}	
	//mul, muli
	if(opcode == 0x04 || opcode == 0x05){
		if(((uint64_t)x * (uint64_t)y) > UINT32_MAX){
			R[FR] = R[FR] | 0x00000010;
		}
		else{
			R[FR] = R[FR] & 0x0000006F;
		}	
	}	
	//div, divi
	if(opcode == 0x06 || opcode == 0x07){
		if(y == 0x00000000){
			R[FR] = R[FR] | 0x00000008;
		}else{
			R[FR] = R[FR] & 0x0000006F;
		}
			/*if(x < 0 || y < 0){
				R[FR] = R[FR] | 0x00000010;
			}			
			else{
				R[FR] = R[FR] & 0xFFFFFFEF;
			}*/
	}	
	//cmp, cmpi
	if(opcode == 0x08 || opcode == 0x09){
		R[FR] = R[FR] & 0xFFFFFFF8;
		//EQ
		if(x == y){
			R[FR] = R[FR] | 0x00000001;
		}		
		//LT
		if(x < y){
			R[FR] = R[FR] | 0x00000002;
		}		
		//GT
		if(x > y){
			R[FR] = R[FR] | 0x00000004;
		}	
	}
}
//vei demorou essa sacada
void setarLDB(uint32_t posmen, uint32_t end, uint32_t x){
	
		switch(end){
			case 0:
				R[x] = (memoria[posmen] & 0xFF000000) >> 24;
			break;
			case 1:
				R[x] = (memoria[posmen] & 0x00FF0000) >> 16;
			break;
			case 2:
				R[x] = (memoria[posmen] & 0x0000FF00) >> 8;
			break;
			case 3:
				R[x] = (memoria[posmen] & 0x000000FF);
			break;
		}	
}
//vei isso daqui ta louco
void setarSTB(uint32_t end,uint32_t byte, uint32_t posmen, uint32_t y){
	
	switch(end){
		case 0:
			memoria[posmen] = memoria[posmen] & 0x00FFFFFF;
			memoria[posmen] = memoria[posmen] | (R[y] << 24);
		break;
		case 1:
			memoria[posmen] = memoria[posmen] & 0xFF00FFFF;
			memoria[posmen] = memoria[posmen] | (R[y] << 16);
		break;
		case 2:
			memoria[posmen] = memoria[posmen] & 0xFFFF00FF;
			memoria[posmen] = memoria[posmen] | (R[y] << 8);
		break;
		case 3:
			memoria[posmen] = memoria[posmen] & 0xFFFFFF00;
			memoria[posmen] = memoria[posmen] | (R[y]);
		break;
	}
	
	
}
//talvez tenha que corrigir
int verificarTerminal(FILE *outterminal){
	int quantidade;
	
	fseek(outterminal,0,SEEK_END);
	quantidade = ftell(outterminal);
	quantidade = quantidade / sizeof(char);
	
	if(quantidade){
		return 1;
	}
	return 0;

}
int castFloatParaInt(float n){ //cast para IEE float int
	union{
		float f;
		int i;
	} cast;
	
	cast.f = n;
	
	return cast.i;
}
float castIntParaFloat(int n){ //cast para IEE int float
	union{
		float f;
		int i;
	} cast;
	
	cast.i = n;
	
	return cast.f;
}	
void verificarFPU(FILE* output){	

	uint32_t expX,expY;
	
	//sacada de bruno
		//Pegando expoent X
	uint32_t* px = (uint32_t*)(&auxFPU[X]); 
	expX = ((*px) & 0x7F80000) >> 23;
		//pegando expoent y
	uint32_t* py = (uint32_t*)(&auxFPU[Y]);
	expY = ((*py) & 0x7F80000) >> 23;
		
	fpu.op = (fpu.control & 0x0000000F); //ta certo
	fpu.status = 0;
	fpu.control = 0x00000000; //control é zerado a cada chamada
	
	switch(fpu.op){
		
		case 0://Sem Operação
		break;
		case 1: //Adição	
			auxFPU[Z] = auxFPU[X] + auxFPU[Y];
			rFPU[Z] = castFloatParaInt(auxFPU[Z]);			
		break;
		case 2: //Subtração
			auxFPU[Z] = auxFPU[X] - auxFPU[Y];
			rFPU[Z] = castFloatParaInt(auxFPU[Z]);		
		break;
		case 3: //multiplicacao
			auxFPU[Z] = auxFPU[X] * auxFPU[Y];
			rFPU[Z] = castFloatParaInt(auxFPU[Z]);	
		break;	
		case 4: //divisao
			if(auxFPU[Y] == 0x00000000){
				fpu.status = 0x00000001;		
				fpu.control = 0x0000020; 
			}
			else{
				auxFPU[Z] = auxFPU[X] / auxFPU[Y];
				rFPU[Z] = castFloatParaInt(auxFPU[Z]); 	
			}
		break;
		case 5: //atribuicao a x
			rFPU[X] = rFPU[Z];
			auxFPU[X] = castIntParaFloat(rFPU[Z]);
		break;
		case 6: //atribuicao a y
			rFPU[Y] = rFPU[Z];
			auxFPU[Y] = castIntParaFloat(rFPU[Z]);		
		break;
		case 7: //teto
			rFPU[Z] = ceilf(auxFPU[Z]);
		break;
		case 8: //piso
			rFPU[Z] = floorf(auxFPU[Z]);
		break;
		case 9: //arredondamento
			rFPU[Z] = roundf(auxFPU[Z]);
		break;
		default:
			fpu.control = 0x00000020;
			fpu.status  = 0x00000001;
				
	}	
	//contando ciclos
	if(fpu.op == 1 || fpu.op == 2 || fpu.op == 3 || fpu.op == 4){
		fpu.ciclo = abs(expX - expY) +1;
	}
	else{
		fpu.ciclo = 0x00000001;
	}
	
	fpu.op = 0;

}
void increaseAGE(uint32_t op){ //LRU
	int i, j; 
	
	if(op == 1){ // op==1 cacheD
		for(i=0;i<8;i++){
			for(j=0;j<2;j++){
				if(cacheD[i][j].v == 1){ //se o dado for valido 
					cacheD[i][j].i++; //idade é incrementada
				}
			}
		}
	}
	else if(op == 2){ // op==2 cacheI
		for(i=0;i<8;i++){
			for(j=0;j<2;j++){
				if(cacheI[i][j].v == 1){ //se o dado for valido
					cacheI[i][j].i++; //idade é incrementada
				}
			}
		}
	}
}
void missCache(struct memoryCache* cache, uint32_t set, uint32_t id, uint32_t end){ //quando da falta na cache
	uint32_t index = 0, mask = 0;
	int i;
	
	mask = end & 0xFFFFFFFC;	
	cache[set].v = 1;
	cache[set].i = 0;
	cache[set].id = id;
	
	for(i = 0; i < 4; i++ ){
		index = (mask+i)%cont;
		cache[set].dados[i] = memoria[index];
	}	
}
int older(struct memoryCache* cache){
	uint32_t maiorSet0 = 0, maiorSet1 = 0, auxLinha0 = 0, auxLinha1 = 0;
	int i;
	
	//para descobrir qual é o mais velho
	for(i = 0; i < 8; i++){
		if(maiorSet0<cache[set0].i){
			maiorSet0 = cache[set0].i;
			auxLinha0 = i;
		}
		if(maiorSet1<cache[set1].i){
			maiorSet1 = cache[set1].i;
			auxLinha1 = i;
		}
	} 
	if(maiorSet0>=maiorSet1){
		return auxLinha0;	
	}
	else{
		return auxLinha1;
	}
}//OBS LIMITS TEM DUAS LINHAS ERRADAS
void carregandoCacheD(uint32_t end, FILE* output, uint32_t op, uint32_t pos){//talvez mude o nome dessa funcao
	int i;
	char status[8];
	uint32_t linha = 0, id = 0, palavra = 0; 
	//00000000|000|00|00
	
	linha = ((end << 2) & 0x00000070) >> 4; //pegando a linha da matriz da cache
	palavra = ((end << 2) & 0x0000000C) >> 2;
	id = ((end << 2) & 0x00007F80) >> 7; //pegando o identificador
	
	increaseAGE(1); //funcao pra incrementar a idade // op == 1 cacheD
	
	//1 == leitura, 2 == escrita
	if(op == read){ //cacheD leitura
		//caso trivial
		if(cacheD[linha][set0].v == 0 && cacheD[linha][set1].v == 0){ // invalidos
			fprintf(output,"[CACHE D LINE %d READ MISS @ 0x%08X]\n",linha,end<<2);
			for(i = 0; i < 2; i++ ){
				strcpy(status,"INVALID");
				fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
				cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);
			}
			acumD.miss++;
			missCache(cacheD[linha],set0,id,end); //setando a cache pq deu miss
		
		} 
		//caso a cache set0 tenha e a cache set1 não tenha
		else if(cacheD[linha][set0].v == 1 && cacheD[linha][set1].v == 0){ 
			
			if(cacheD[linha][set0].id == id && cacheD[linha][set0].dados[palavra] == memoria[end]){//verificando set0
				fprintf(output,"[CACHE D LINE %d READ HIT @ 0x%08X]\n",linha, end<<2);
				cacheD[linha][set0].i = 0;//zerando a idade pq foi referenciado
				for(i = 0; i < 2; i++ ){
					if(cacheD[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
					cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);
				}
				acumD.hit++;
			}
			else{
			//	falta
				fprintf(output,"[CACHE D LINE %d READ MISS @ 0x%08X]\n",linha,end<<2);
				for(i = 0; i < 2; i++ ){
					if(cacheD[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
					cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);
				}
				acumD.miss++;
				missCache(cacheD[linha],set1,id,end); //cache miss
					
			}
		}
		//caso a cache set0 e a cache set 1 tenham
		else if (cacheD[linha][set0].v == 1 && cacheD[linha][set1].v == 1){ 
		
			if(cacheD[linha][set0].id == id && cacheD[linha][set0].dados[palavra] == memoria[end]){ //verificando set0
				fprintf(output,"[CACHE D LINE %d READ HIT @ 0x%08X]\n",linha, end<<2);
				cacheD[linha][set0].i = 0;//zerando a idade pq foi referenciado
				for(i = 0; i < 2; i++){
					if(cacheD[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
					cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);
				}
				acumD.hit++;
			}
			else if(cacheD[linha][set1].id == id && cacheD[linha][set1].dados[palavra] == memoria[end]){ //verificando set1
				fprintf(output,"[CACHE D LINE %d READ HIT @ 0x%08X]\n",linha, end<<2);
				cacheD[linha][set1].i = 0;//zerando a idade pq foi referenciado
				for(i = 0; i < 2; i++){
					if(cacheD[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
					cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);
				}
				acumD.hit++;
			}
			else{
				//falta
				fprintf(output,"[CACHE D LINE %d READ MISS @ 0x%08X]\n",linha,end<<2);
				for(i = 0; i < 2; i++){
					if(cacheD[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
					cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);
				}
				acumD.miss++;
				//para descobrir qual é o mais velho
				if(cacheD[linha][set0].i >= cacheD[linha][set1].i){
					missCache(cacheD[linha],set0,id,end); //setando o mais velho
				}else if(cacheD[linha][set1].i > cacheD[linha][set0].i ){
					missCache(cacheD[linha],set1,id,end); //setando o mais velho	
				}
			}	
		}
	}
	else if(op == write){//cacheD escrita
		//caso trivial
		if(cacheD[linha][set0].v == 0 && cacheD[linha][set1].v == 0){
			fprintf(output,"[CACHE D LINE %d WRITE MISS @ 0x%08X]\n",linha,end<<2);
			for(i = 0; i < 2; i++ ){
				strcpy(status,"INVALID");
				fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
				cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);
			}
			acumD.miss++;
		}
		//caso a cache set0 tenha e a cache set1 nao tenha
		else if(cacheD[linha][set0].v == 1 && cacheD[linha][set1].v == 0){
			
			if(cacheD[linha][set0].id == id /*&& cacheD[linha][set0].dados[palavra] != memoria[end]*/){	
				fprintf(output,"[CACHE D LINE %d WRITE HIT @ 0x%08X]\n",linha,end<<2);
				cacheD[linha][set0].v = 1;
				cacheD[linha][set0].i = 0;
				for( i = 0; i < 2; i++){
					if(cacheD[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
					cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);	
				}
				//cacheD[linha][set0].dados[palavra] = memoria[end]; //pequena gambi //o certo seria R[pos]
				cacheD[linha][set0].dados[palavra] = R[pos]; //jeito certo
				acumD.hit++;			
			}
			else{
				fprintf(output,"[CACHE D LINE %d WRITE MISS @ 0x%08X]\n",linha,end<<2);
				for(i = 0; i < 2; i++ ){
					if(cacheD[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}		
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
					cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);
				}
				acumD.miss++;
			}
		}
		//caso a cahce set0 e a cache set1 tenha
		else if(cacheD[linha][set0].v == 1 && cacheD[linha][set1].v == 1){
			
			if(cacheD[linha][set0].id == id /*&& cacheD[linha][set0].dados[palavra] != memoria[end]*/){
			//	printf("HIT\n");
				fprintf(output,"[CACHE D LINE %d WRITE HIT @ 0x%08X]\n",linha,end<<2);
				cacheD[linha][set0].v = 1;
				cacheD[linha][set0].i = 0;			
				for( i = 0; i < 2; i++){
					if(cacheD[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}		
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
					cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);	
				}
			//	cacheD[linha][set0].dados[palavra] = memoria[end]; //pequena gambi, //o certo seria R[pos]; 
				cacheD[linha][set0].dados[palavra] = R[pos]; //jeito certo
				acumD.hit++;
			}
			else if(cacheD[linha][set1].id == id /*&& cacheD[linha][set1].dados[palavra] != memoria[end]*/){
			//	printf("HIT\n");
				fprintf(output,"[CACHE D LINE %d WRITE HIT @ 0x%08X]\n",linha,end<<2);
				cacheD[linha][set1].v = 1;
				cacheD[linha][set1].i = 0;			
				for( i = 0; i < 2; i++){
					if(cacheD[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}		
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
					cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);	
				}
			//	cacheD[linha][set1].dados[palavra] = memoria[end]; //pequena gambi, //o certo seria R[pos]; 
				cacheD[linha][set1].dados[palavra] = R[pos]; //jeito certo
				acumD.hit++;
			}
			else{
			//	printf("MISS\n");
				fprintf(output,"[CACHE D LINE %d WRITE MISS @ 0x%08X]\n",linha,end<<2);
				for(i = 0; i < 2; i++ ){
					if(cacheD[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}		
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheD[linha][i].i,cacheD[linha][i].dados[0],
					cacheD[linha][i].dados[1],cacheD[linha][i].dados[2],cacheD[linha][i].dados[3]);
				}
				acumD.miss++;	
			
			}			
		}		
	}			
}
void carregandoCacheI(uint32_t end, FILE* output,uint32_t op, uint32_t pos){
	int i = 0;
	char status[8];
	uint32_t linha = 0, id = 0, palavra = 0;
	//00000000|000|00|00
	
	linha = ((end << 2) & 0x00000070)>>4; //pegando a linha da matriz da cache
	palavra = ((end << 2) & 0x0000000C)>>2;
	id = ((end << 2) & 0x00007F80) >> 7; //pegando o identificador
	
	increaseAGE(2); //funcao pra incrementar a idade // op == 2 cacheI
	
	//escrita == 2, leitura == 1
	if(op == read){//leitura CacheI
		//caso trivial
		if(cacheI[linha][set0].v == 0 && cacheI[linha][set1].v == 0 ){//sao invalidos, ta dando falta
			fprintf(output,"[CACHE I LINE %d READ MISS @ 0x%08X]\n",linha,end<<2);
			for(i = 0; i < 2; i++ ){
				strcpy(status,"INVALID");
				fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheI[linha][i].i,cacheI[linha][i].dados[0],
				cacheI[linha][i].dados[1],cacheI[linha][i].dados[2],cacheI[linha][i].dados[3]);
			}
			acumI.miss++;
			missCache(cacheI[linha],set0,id,end); //setando a cache pq deu miss
		}
		//caso a cache set0 tenha e a cache set1 nao tenha
		else if(cacheI[linha][set0].v == 1 && cacheI[linha][set1].v == 0){
			
			if(cacheI[linha][set0].id == id && cacheI[linha][set0].dados[palavra] == memoria[end]){//Verificando o set0
				fprintf(output,"[CACHE I LINE %d READ HIT @ 0x%08X]\n",linha, end<<2);
				cacheI[linha][set0].i = 0;//zerando a idade pq foi referenciado
				for(i = 0; i < 2; i++ ){
					if(cacheI[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheI[linha][i].i,cacheI[linha][i].dados[0],
					cacheI[linha][i].dados[1],cacheI[linha][i].dados[2],cacheI[linha][i].dados[3]);
				}
				acumI.hit++;
			}
			else{
				
				fprintf(output,"[CACHE I LINE %d READ MISS @ 0x%08X]\n",linha,end<<2);
				for(i = 0; i < 2; i++ ){
					if(cacheI[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheI[linha][i].i,cacheI[linha][i].dados[0],
					cacheI[linha][i].dados[1],cacheI[linha][i].dados[2],cacheI[linha][i].dados[3]);
				}
				acumI.miss++;
				missCache(cacheI[linha],set1,id,end); //setando a cache pq deu miss					
			}
			
		}
		//caso a cache set0 e a set1 tenham 
		else if(cacheI[linha][set0].v == 1 && cacheI[linha][set1].v == 1){ 
			
			if(cacheI[linha][set0].id == id && cacheI[linha][set0].dados[palavra] == memoria[end]){ //verificando set0
				fprintf(output,"[CACHE I LINE %d READ HIT @ 0x%08X]\n",linha, end<<2);
				cacheI[linha][set0].i = 0;//zerando a idade pq foi referenciado
				for(i = 0; i < 2; i++){
					if(cacheI[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheI[linha][i].i,cacheI[linha][i].dados[0],
					cacheI[linha][i].dados[1],cacheI[linha][i].dados[2],cacheI[linha][i].dados[3]);
				}
				acumI.hit++;
			}
			else if(cacheI[linha][set1].id == id && cacheI[linha][set1].dados[palavra] == memoria[end]){ //verificando set1
				fprintf(output,"[CACHE I LINE %d READ HIT @ 0x%08X]\n",linha, end<<2);
				cacheI[linha][set1].i = 0;//zerando a idade pq foi referenciado
				for(i = 0; i < 2; i++){
					if(cacheI[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheI[linha][i].i,cacheI[linha][i].dados[0],
					cacheI[linha][i].dados[1],cacheI[linha][i].dados[2],cacheI[linha][i].dados[3]);
				}
				acumI.hit++;
			}
			else{
				fprintf(output,"[CACHE I LINE %d READ MISS @ 0x%08X]\n",linha,end<<2);
				for(i = 0; i < 2; i++ ){
					if(cacheI[linha][i].v == 1){
						strcpy(status,"VALID");
					}
					else{
						strcpy(status,"INVALID");
					}
					fprintf(output,"[SET %d: %s, AGE %d, DATA 0x%08X 0x%08X 0x%08X 0x%08X]\n",i,status,cacheI[linha][i].i,cacheI[linha][i].dados[0],
					cacheI[linha][i].dados[1],cacheI[linha][i].dados[2],cacheI[linha][i].dados[3]);
				}
				acumI.miss++;
				//para descobrir qual é o mais velho
				if(cacheI[linha][set0].i >= cacheI[linha][set1].i){
					missCache(cacheI[linha],set0,id,end); //setando o mais velho
				}else if(cacheI[linha][set1].i > cacheI[linha][set0].i ){
					missCache(cacheI[linha],set1,id,end); //setando o mais velho	
				}
			}	
		}	
	}
}
//inicio das operacoes
void add(FILE* output){ //tipoU
	
	//pra ver se tem ouver ou nem
	setarFR(R[tipoU.x],R[tipoU.y]);
	
	//realizando a soma
	R[tipoU.z] = R[tipoU.x] + R[tipoU.y];
	setarR0();
	
	//1 min 0 maius
	//exibicao da saida
	fprintf(output,"add ");imprimeRegistradores(tipoU.z,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] FR = 0x%.8X, ",R[FR]);imprimeRegistradores(tipoU.z,0,output);fprintf(output," = ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," + ");imprimeRegistradores(tipoU.y,0,output);fprintf(output," = 0x%.8X\n",R[tipoU.z]);
	
	pczinho++;
}
void addi(FILE* output){ //tipoF
	
	//pra ver se tem over ou nem
	setarFR(R[tipoF.y],tipoF.im16);
	
	//realizando a operacao
	R[tipoF.x] = R[tipoF.y] + tipoF.im16;
	setarR0();
	
	
	//exibicao da saida
	fprintf(output, "addi r%d, r%d, %d\n[F] FR = 0x%08X, R%d = R%d + 0x%04X = 0x%08X\n", tipoF.x, tipoF.y, tipoF.im16, R[FR], tipoF.x, tipoF.y, tipoF.im16, R[tipoF.x]);
	pczinho++;
}
void sub(FILE * output){//tipoU
	
	//pra ver se tem over ou nem
	setarFR(R[tipoU.x],R[tipoU.y]);	
	//realizando a operacao
	R[tipoU.z] = R[tipoU.x] - R[tipoU.y];
	setarR0();
	
	
	//1 min 0 maius
	//exibicao da saida
	fprintf(output,"sub ");imprimeRegistradores(tipoU.z,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] FR = 0x%.8X, ",R[FR]);imprimeRegistradores(tipoU.z,0,output);fprintf(output," = ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," - ");imprimeRegistradores(tipoU.y,0,output);fprintf(output," = 0x%.8X\n",R[tipoU.z]);
	pczinho++;
}
void subi(FILE *output){//tipoF
	
	
	//pra ver se tem over ou nem
	setarFR(R[tipoF.y],tipoF.im16);
	//realizando a operacao
	R[tipoF.x] = R[tipoF.y] - tipoF.im16;
	setarR0();
	
	//exibicao da saida
	fprintf(output, "subi r%d, r%d, %d\n[F] FR = 0x%08X, R%d = R%d - 0x%04X = 0x%08X\n", tipoF.x, tipoF.y, tipoF.im16, R[FR], tipoF.x, tipoF.y, tipoF.im16, R[tipoF.x]);
	pczinho++;
}
void mul(FILE* output){//tipoU
	
	uint64_t ERR;
	
	//realizando a operacao
	ERR = ((uint64_t)R[tipoU.x] * (uint64_t)R[tipoU.y]);
	R[tipoU.z] = ERR & 0x00000000FFFFFFFF;
	R[ER] = ERR >> 32;	
	setarR0(); 
	//pra ver se tem over ou nem
	setarFR(R[tipoU.x],R[tipoU.y]);
	
	//exibicao da saida
	//1 min 0 maius
	fprintf(output,"mul ");imprimeRegistradores(tipoU.z,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] FR = 0x%.8X, ",R[FR]);fprintf(output,"ER = 0x%.8X, ",R[ER]);imprimeRegistradores(tipoU.z,0,output);fprintf(output," = ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," * ");imprimeRegistradores(tipoU.y,0,output);fprintf(output," = 0x%.8X\n",R[tipoU.z]);
	pczinho++;	
}
void muli(FILE* output){//tipoF
	
	uint64_t ERR;
	
	//realizando operacao
	ERR = ((uint64_t)R[tipoF.y] * (uint64_t)tipoF.im16);
	R[tipoF.x] = ERR &  0x00000000FFFFFFFF;
	R[ER] = ERR >> 32;
	setarR0();
	//pra ver se tem over ou nem
	setarFR(R[tipoF.y],tipoF.im16);
	
	//exibicao da saida
	//1 min 0 maius
	fprintf(output, "muli r%d, r%d, %d\n[F] FR = 0x%08X, ER = 0x%08X, R%d = R%d * 0x%04X = 0x%08X\n", tipoF.x, tipoF.y, tipoF.im16, R[FR], R[ER], tipoF.x, tipoF.y, tipoF.im16, R[tipoF.x]);
	pczinho++;
}
void div_ (FILE* output){//tipoU
	
	pczinho++;
	//pra ver se tem over ou nem
	setarFR(R[tipoU.x],R[tipoU.y]);
	//realizando a operacao
	if(R[tipoU.y] != 0x00000000){
		R[ER] = R[tipoU.x] % R[tipoU.y];
		R[tipoU.z] = R[tipoU.x] / R[tipoU.y];
	}
	
	//exibicao da saida
	//1 min 0 maius
	fprintf(output,"div ");imprimeRegistradores(tipoU.z,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] FR = 0x%.8X, ",R[FR]);fprintf(output,"ER = 0x%.8X, ",R[ER]);imprimeRegistradores(tipoU.z,0,output);fprintf(output," = ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," / ");imprimeRegistradores(tipoU.y,0,output);fprintf(output," = 0x%.8X\n",R[tipoU.z]);
	
	if(R[tipoU.y] == 0x00000000){
		setarFR(R[tipoU.x],R[tipoU.y]);
		setarIE(output);		
		setarR0();
	}
	else{
		setarFR(R[tipoU.x],R[tipoU.y]);	
	}			
}
void divi(FILE* output){//tipoF

	pczinho++;
	//praver se tem over ou nem
	setarFR(R[tipoF.y],tipoF.im16);
	//realizando a operacao
	if(tipoF.im16 != 0x00000000){
		R[ER] = R[tipoF.y] % tipoF.im16;
		R[tipoF.x] = R[tipoF.y] / tipoF.im16;
	}
	//exibicao da saida
	//1 min 0 maius
	fprintf(output, "divi r%d, r%d, %d\n[F] FR = 0x%08X, ER = 0x%08X, R%d = R%d / 0x%04X = 0x%08X\n", tipoF.x, tipoF.y, tipoF.im16, R[FR], R[ER], tipoF.x, tipoF.y, tipoF.im16, R[tipoF.x]);
		
	if(tipoF.im16 == 0x00000000){
		setarFR(R[tipoF.y],tipoF.im16);
		setarIE(output);
		setarR0();	
	}
	else{
		setarFR(R[tipoF.y],tipoF.im16);
	}
}
void cmp(FILE* output){//tipoU
	
	//checando FR
	setarFR(R[tipoU.x],R[tipoU.y]);
	//exibicao da saida
	//1 min 0 maius
	fprintf(output,"cmp ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] FR = 0x%.8X\n",R[FR]);
	pczinho++;
}
void cmpi(FILE* output){//tipoF
	
	//checando FR
	setarFR(R[tipoF.x],tipoF.im16);
	//exibicao da saida
	//1 min 0 maius
	fprintf(output, "cmpi r%d, %d\n[F] FR = 0x%08X\n", tipoF.x, tipoF.im16, R[FR]);
	pczinho++;
}
void shl(FILE* output){//tipoU
	uint64_t ERR;
	
	//realizando operacoes
	ERR = (uint64_t)R[ER] << 32;
	ERR = ERR | R[tipoU.x];
	ERR = ERR << (tipoU.y+1);
	
	R[ER] = ERR >>32;
	R[tipoU.z] = (uint32_t)ERR;	
	
	
	//exibicao da saida
	fprintf(output,"shl ");imprimeRegistradores(tipoU.z,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", %d",tipoU.y);fprintf(output,"\n");
	fprintf(output,"[U] ER = 0x%.8X, ",R[ER]);imprimeRegistradores(tipoU.z,0,output);fprintf(output," = ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," << %d = 0x%.8X\n",(tipoU.y + 1), R[tipoU.z]);
	pczinho++;
}
void shr(FILE* output){//tipoU //ta perfeito
	uint64_t ERR;
//	printf("%X",R[tipoU.x]);
//	getchar();
	//realizando operacoes
	ERR = (uint64_t)R[ER] << 32;
	ERR = ERR | (uint64_t)R[tipoU.x];
	ERR = ERR >> (tipoU.y+1);
	
	R[ER] = ERR >> 32;
	R[tipoU.z] = (uint32_t)ERR;
	
	//exibicao da saida
	fprintf(output,"shr ");imprimeRegistradores(tipoU.z,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", %d",tipoU.y);fprintf(output,"\n");
	fprintf(output,"[U] ER = 0x%.8X, ",R[ER]);imprimeRegistradores(tipoU.z,0,output);fprintf(output," = ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," >> %d = 0x%.8X\n",(tipoU.y + 1), R[tipoU.z]);
	pczinho++;
}
void and (FILE* output){//tipoU

	//realizando a operacao
	R[tipoU.z] = R[tipoU.x] & R[tipoU.y];
	
	//exibicao da saida
	fprintf(output,"and ");imprimeRegistradores(tipoU.z,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] ");imprimeRegistradores(tipoU.z,0,output);fprintf(output," = ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," & ");imprimeRegistradores(tipoU.y,0,output);fprintf(output," = 0x%.8X\n",R[tipoU.z]);			
	pczinho++;
}
void andi(FILE* output){//tipoF
	
	//realizando a operacao
	R[tipoF.x] = R[tipoF.y] & tipoF.im16;
	
	//exibicao da saida
	fprintf(output,"andi r%d, r%d, %d\n[F] R%d = R%d & 0x%04X = 0x%08X\n", tipoF.x, tipoF.y, tipoF.im16, tipoF.x, tipoF.y, tipoF.im16,R[tipoF.x]);
	pczinho++;
}
void not(FILE* output){//tipoU

	//realizando a operacao
	R[tipoF.x] = ~R[tipoF.y];
	
	//exibicao da saida
	fprintf(output,"not ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," = ~");imprimeRegistradores(tipoU.y,0,output);fprintf(output," = 0x%.8X\n",R[tipoU.x]);
	pczinho++;
}
void noti(FILE* output){//tipoF
	
	//realizando a operacao
	R[tipoF.x] = ~tipoF.im16;
	
	//exibicao da saida
	fprintf(output,"noti r%d, %d\n[F] R%d = ~0x%.4X = 0x%.8X\n", tipoF.x, tipoF.im16, tipoF.x,tipoF.im16,R[tipoF.x]);
	pczinho++;
	
}
void or(FILE* output){ //tipoU
	
	//realizando a operacao
	R[tipoU.z] = R[tipoU.x] | R[tipoU.y];
	
	//exibicao da saida
	fprintf(output,"or ");imprimeRegistradores(tipoU.z,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] ");imprimeRegistradores(tipoU.z,0,output);fprintf(output," = ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," | ");imprimeRegistradores(tipoU.y,0,output);fprintf(output," = 0x%.8X\n",R[tipoU.z]);			
	pczinho++;
}
void ori(FILE* output){//tipoF
	
	//realizando a operacao
	R[tipoF.x] = R[tipoF.y] | tipoF.im16;
	
	//exibicao da saida
	fprintf(output,"ori r%d, r%d, %d\n[F] R%d = R%d | 0x%04X = 0x%08X\n", tipoF.x, tipoF.y, tipoF.im16, tipoF.x, tipoF.y, tipoF.im16, R[tipoF.x]);
	pczinho++;	
}
void xor(FILE* output){ //tipoU

	//realizando a operacao
	R[tipoU.z] = R[tipoU.x] ^ R[tipoU.y];
	
	//exibicao da saida
	fprintf(output,"xor ");imprimeRegistradores(tipoU.z,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] ");imprimeRegistradores(tipoU.z,0,output);fprintf(output," = ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," ^ ");imprimeRegistradores(tipoU.y,0,output);fprintf(output," = 0x%.8X\n",R[tipoU.z]);				
	pczinho++;
}
void xori(FILE* output){ //tipoF

	//realizando a operacao
	R[tipoF.x] = R[tipoF.y] ^ tipoF.im16;
	
	//exibicao da saida
	fprintf(output,"xori r%d, r%d, %d\n[F] R%d = R%d ^ 0x%04X = 0x%08X\n", tipoF.x, tipoF.y, tipoF.im16, tipoF.x, tipoF.y, tipoF.im16, R[tipoF.x]);
	pczinho++;
}
void ldw(FILE* output){ // tipoF
	uint32_t aux = (R[tipoF.y] + tipoF.im16);
//	printf("LDW End: 0x%.8X\n",(tipoF.y + tipoF.im16)<<2);
	//watchdog
	if(aux == 0x00002020){
		watchdog.EN = memoria[(R[tipoF.y]) + tipoF.im16] >> 31;
		watchdog.counter = (memoria[(R[tipoF.y]) + tipoF.im16]<<1)>>1;
	}
	//fpu
	else if(aux == 0x00002203 || aux == 0x00002202 || aux == 0x00002201 || aux == 0x00002200 ){
		switch(aux){
			case 0x00002203://control
				R[tipoF.x] = fpu.control;
			break;				
			case 0x00002202://z	
				R[tipoF.x] = rFPU[Z];
			break;								
			case 0x00002201: //y
				R[tipoF.x] = rFPU[Y];
			break;
			case 0x00002200://x
				R[tipoF.x] = rFPU[X];	
			break;
		}				
	}
	else{
		
		carregandoCacheD(R[tipoF.y] + tipoF.im16,output,read,0); //1 == leitura
		setarR0();
		R[tipoF.x] = memoria[(R[tipoF.y])+tipoF.im16];
	}
	//exibicao da saida
	fprintf(output, "ldw r%d, r%d, 0x%04X\n[F] R%d = MEM[(R%d + 0x%04X) << 2] = 0x%08X\n",tipoF.x, tipoF.y, tipoF.im16, tipoF.x, tipoF.y, tipoF.im16, R[tipoF.x]);
	
	pczinho++;
}
void ldb(FILE* output){//tipoF
	uint32_t byte, end, posmen;
	posmen = (R[tipoF.y] + tipoF.im16)>>2; //DESGRAÇAAAAAAAAA
	byte = (R[tipoF.y]+tipoF.im16);
	end = byte - (posmen<<2);
	
	setarLDB(posmen,end,tipoF.x);	
	//TERMINAL
	if(byte == 0x0000888B){
		//ajeitar isso aqui
	}
	else{
		carregandoCacheD(posmen,output,read,tipoF.x);//1 == leitura //posmen é a posicao q eu quero usar la
	}
	//exibicao da saida
	fprintf(output, "ldb r%d, r%d, 0x%04X\n[F] R%d = MEM[R%d + 0x%04X] = 0x%02X\n", tipoF.x, tipoF.y, tipoF.im16, tipoF.x, tipoF.y, tipoF.im16, R[tipoF.x]);
	pczinho++;
}
void stw(FILE* output){//tipoF
	uint32_t aux = (R[tipoF.x] + tipoF.im16	);
	 //watchdog
	if(aux == 0x00002020){// && contdog > 0 
		watchdog.EN = R[tipoF.y] >> 31;
		watchdog.counter = (R[tipoF.y]<<1)>>1;
		ativacaoWatch = 1;
		prioridade = 1;
	}
	//fpu
	else if(aux == 0x00002203 || aux == 0x00002202 || aux == 0x00002201 || aux == 0x00002200){
		switch(aux){
			case 0x00002203://control
				fpu.control = R[tipoF.y];
				verificarFPU(output);
				ativacaoFPU = 1;					 
			break;	
			case 0x00002202://z
				auxFPU[Z] = R[tipoF.y];
			break;	
			case 0x00002201://y
				auxFPU[Y] = R[tipoF.y];
			break;	
			case 0x00002200://x
				auxFPU[X] = R[tipoF.y];
			break;		
		}
	}
	//Pelo visto funciona sem esse else wtf
	else{
		setarR0();
		//viagem psicodelica
		carregandoCacheD(R[tipoF.x] + tipoF.im16,output,write,tipoF.y); //2 == escrita
		//x + im16 
	}
	memoria[(R[tipoF.x] + tipoF.im16)] = R[tipoF.y];		
	//exibicao da saida
	fprintf(output, "stw r%d, 0x%04X, r%d\n[F] MEM[(R%d + 0x%04X) << 2] = R%d = 0x%08X\n",tipoF.x, tipoF.im16, tipoF.y, tipoF.x, tipoF.im16, tipoF.y, R[tipoF.y]);
	pczinho++;	
}
void stb(FILE* output,FILE* outterminal){//tipoF
	
	uint32_t byte,end,posmen;
	
	//posicao do byte que sera alterado
	posmen = (R[tipoF.x] + tipoF.im16)>>2;
	byte = (R[tipoF.x] + tipoF.im16);
	end = byte - (posmen<<2);		
	
	setarSTB(end,byte,posmen,tipoF.y);
	//verificar se ta certo depois
	//TERMINAL
	if(byte == 0x0000888B){
		//printf("STB - TERMINAL\n");
		terminal.ter = R[tipoF.y] & 0x000000FF;
		//exibicao da saida
//		fprintf(output, "stb r%d, 0x%04X, r%d\n[F] MEM[R%d + 0x%04X] = R%d = 0x%02X\n", tipoF.x, tipoF.im16, tipoF.y, tipoF.x, tipoF.im16, tipoF.y,terminal.ter);
		fprintf(outterminal,"%c",terminal.ter);		
	}
	else{
		carregandoCacheD(posmen,output,write,tipoF.y); // 2 == escrita //posmen é a posicao q eu quero usar la
		//exibicao da saida
	}
	fprintf(output, "stb r%d, 0x%04X, r%d\n[F] MEM[R%d + 0x%04X] = R%d = 0x%02X\n", tipoF.x, tipoF.im16, tipoF.y, tipoF.x, tipoF.im16, tipoF.y, R[tipoF.y]);		
	pczinho++;
}
void bun(FILE* output){ //tipoS
	
	pczinho = tipoS.im26;	
	fprintf(output, "bun 0x%08X\n[S] PC = 0x%08X\n", tipoS.im26, pczinho<<2);	
}
void beq(FILE* output){ //tipoS
	
	fprintf(output,"beq 0x%.8X\n",tipoS.im26);
	if((R[FR] & 0b00001) == 0b00001){ //EQ
		pczinho = tipoS.im26;
	}
	else{
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho << 2);
}
void blt(FILE* output){ //tipoS
	
	fprintf(output,"blt 0x%.8X\n",tipoS.im26);
	
	if((R[FR] & 0b00010) == 0b00010){ //LT
		pczinho = tipoS.im26;
	}
	else{
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho << 2);
}
void bgt(FILE* output){//tipoS

	fprintf(output,"bgt 0x%.8X\n",tipoS.im26);	
	
	if((R[FR] & 0x00000004) == 0x00000004){ //GT
		pczinho = tipoS.im26;		
	}
	else{
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho << 2);
}
void bne (FILE* output){ //tipoS
	
	fprintf(output,"bne 0x%.8X\n",tipoS.im26);

	if((R[FR] & 0b00001) != 0b00001){ //~EQ		
		pczinho = tipoS.im26;
	}
	else{	
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho << 2);
}
void ble(FILE* output){ //tipoS
	
	fprintf(output,"ble 0x%.8X\n",tipoS.im26);

	if((R[FR] & 0b00011) == 0b00011){ //LT v EQ
		pczinho = tipoS.im26;		
	}
	else{
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho<2);
}
void bge(FILE* output){//tipoS
	
	fprintf(output,"bge 0x%.8X\n",tipoS.im26);

	if(((R[FR] & 0b00100) == 0b00100)||((R[FR] & 0b00001) == 0b00001)){ //GT v EQ
		pczinho = tipoS.im26;
	}
	else{
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho<<2);	
}
void bzd(FILE* output){//tipoS
	
	fprintf(output,"bzd 0x%.8X\n",tipoS.im26);
	
	if((R[FR] & 0b01000) == 0b01000){ //ZD
		pczinho = tipoS.im26;
	}
	else{
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho<<2);
}
void bnz(FILE* output ){//tipoS
	
	fprintf(output,"bnz 0x%.8X\n",tipoS.im26);
	
	if((R[FR] & 0b01000) != 0b01000){ //~ZD
		pczinho = tipoS.im26;
	}
	else{
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho<<2);
}
void biv(FILE* output){//tipoS
	
	fprintf(output,"biv 0x%.8X\n",tipoS.im26);
	
	if((R[FR] & 0b0100000) == 0b0100000){ //IV
		pczinho = tipoS.im26;
	}
	else{
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho << 2);

}
void bni(FILE* output){//tipoS
	
	fprintf(output,"bni 0x%.8X\n",tipoS.im26);
	
	if((R[FR] & 0b0100000) != 0b0100000){ //~IV
		pczinho = tipoS.im26;
	}
	else{
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho << 2);
	
}
int in_t(FILE* output,FILE* outterminal){//tipoS
	
	if(tipoS.im26 == 0x00000000){
		//END OF SIMULATION
		R[IPC] = pczinho + 1;
		R[CR] = tipoS.im26;
		pczinho = 0x00000000;
		
		fprintf(output,"int %d\n",tipoS.im26);
		fprintf(output,"[S] CR = 0x%.8X, PC = 0x%.8X\n",R[CR],pczinho<<2);
				
		//TERMINAL
		fclose(outterminal);
		outterminal = fopen("outterminal.txt","r");
		if(verificarTerminal(outterminal)){
			fseek(outterminal,0,SEEK_SET);
			fprintf(output,"[TERMINAL]\n");
			while(((terminal.c = fgetc(outterminal))!=EOF)){
				fprintf(output,"%c",terminal.c);
			}
			fprintf(output, "\n");
		}
		
		fprintf(output,"[END OF SIMULATION]\n");
		fprintf(output,"[CACHE D STATISTICS] #Hit = %.f (%.f%%), #Miss = %.f (%.f%%)\n",acumD.hit,(acumD.hit/(acumD.hit+acumD.miss))*100,acumD.miss,(acumD.miss)/(acumD.hit+acumD.miss)*100);
		fprintf(output,"[CACHE I STATISTICS] #Hit = %.f (%.f%%), #Miss = %.f (%.f%%)\n",acumI.hit,(acumI.hit/(acumI.hit+acumI.miss))*100,acumI.miss,(acumI.miss)/(acumI.hit+acumI.miss)*100);
					
		
		return  0;	
	}
	else{
		R[IPC] = pczinho + 1;
		R[CR] = tipoS.im26;		
		//famoso desvio
		pczinho = 0x0000003; //0x0000000C
		
		fprintf(output,"int %d\n",tipoS.im26);
		fprintf(output,"[S] CR = 0x%.8X, PC = 0x%.8X\n",R[CR],pczinho<<2);
		fprintf(output,"[SOFTWARE INTERRUPTION]\n");
		
		return 1;
	}

	return 1;
}
void call(FILE* output){//tipoF
	
	R[tipoF.x] = (pczinho +1);
	setarR0();
	pczinho = (R[tipoF.y] + (tipoF.im16));
	fprintf(output, "call r%d, r%d, 0x%04X\n[F] R%d = (PC + 4) >> 2 = 0x%08X, PC = (R%d + 0x%04X) << 2 = 0x%08X\n", tipoF.x, tipoF.y, tipoF.im16, tipoF.x, R[tipoF.x], tipoF.y, tipoF.im16, pczinho<<2);

}
void ret(FILE* output){//tipoF
	
	pczinho = R[tipoF.x];	
	fprintf(output, "ret r%d\n[F] PC = R%d << 2 = 0x%08X\n", tipoF.x, tipoF.x, pczinho<<2);
}
void reti(FILE* output){//tipoF

	pczinho = R[tipoF.x];
	fprintf(output, "reti r%d\n[F] PC = R%d << 2 = 0x%08X\n", tipoF.x, tipoF.x, pczinho<<2);	
	
	prioridade = 2;
}
void push(FILE* output){//tipoU
//corrigir 	
	int end= 0;
	
	end = R[tipoU.x];
	carregandoCacheD(end,output,write,tipoU.y);
	setarR0();
	memoria[(R[tipoU.x]--)] = R[tipoU.y];
	
	
	fprintf(output,"push ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] MEM[");imprimeRegistradores(tipoU.x,0,output);fprintf(output,"--] = ");imprimeRegistradores(tipoU.y,0,output);fprintf(output," = 0x%.8X\n",R[tipoU.y]);
	pczinho++;
}
void pop(FILE* output){//tipoU
//corrigir
	int end = 0;
	
//	end = R[tipoU.y]+4;
	end = R[tipoU.y]+1;
	setarR0();
	R[tipoU.x] = memoria[(++R[tipoU.y])];
	carregandoCacheD(end,output,read,0);
	
	fprintf(output,"pop ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);
	fprintf(output,"\n[U] ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," = MEM[++");imprimeRegistradores(tipoU.y,0,output);fprintf(output,"] = 0x%.8X\n",R[tipoU.x]);
	pczinho++;

}
void isr(FILE* output){//tipoF
		
	R[tipoF.x] = R[IPC];
	R[tipoF.y] = R[CR];
	
	pczinho = tipoF.im16;
	setarR0();	
	fprintf(output, "isr r%d, r%d, 0x%04X\n[F] R%d = IPC >> 2 = 0x%08X, R%d = CR = 0x%08X, PC = 0x%08X\n", tipoF.x, tipoF.y, tipoF.im16, tipoF.x, R[tipoF.x], tipoF.y, R[tipoF.y], pczinho<<2);			
}
//fim das operacoes
void verificarWatchdog(FILE* output){ //dogão
		
		//checando se o EN é 1 e se o counter é 0 
		if(watchdog.counter == 0x00000000 && prioridade != 0 && ((R[FR] & 0x0000040) == 0x0000040) && watchdog.EN !=0){ // && fpu.tratamento == 0	
		//checando se o IE ta setado, EN so é 1 pois vai ser zerado logo a baixo		
		//	if((R[FR] & 0x0000040) == 0x0000040){ 
				R[CR] = 0xE1AC04DA;	
				//IPC recebe PC
				R[IPC] = pczinho;

				fprintf(output,"[HARDWARE INTERRUPTION 1]\n");				
				//famoso desvio
				pczinho = 0x00000001;//0x00000004
				prioridade = 0;
				watchdog.EN = 0;
		}
		
		if(watchdog.EN != 0 && watchdog.counter > 0){ //checa se o EN ta setado
 				watchdog.counter--;
		}
}
void verificarCiclofpu(FILE* output){  //ciclo menstrual da fpu
	
	if(ativacaoFPU == 1 && fpu.relogio < fpu.ciclo && ((R[FR] & 0x00000040) == 0x00000040)){
		fpu.relogio++;		
	}	
	else if(ativacaoFPU == 1 && prioridade == 2 && fpu.relogio == fpu.ciclo && ((R[FR] & 0x00000040) == 0x00000040)){
		//checando se o IE ta setado, e o ciclo é  zero
			R[CR] = 0x01EEE754;			
			//IPC recebe PC
			R[IPC] = pczinho;
						
			fprintf(output,"[HARDWARE INTERRUPTION 2]\n");
			//famoso desvio
			pczinho = 0x00000002;//0x00000008
			fpu.relogio = 0;
			ativacaoFPU = 0;
			if(prioridade == 2){
				prioridade = 1;
			}	
		}			
}
void verificarDefault(FILE* output){ //sai tafarel
	
		fprintf(output,"[INVALID INSTRUCTION @ 0x%.8X]\n",pczinho<<2);
		fprintf(output,"[SOFTWARE INTERRUPTION]\n");
		R[FR] = R[FR] | 0x0000020;  //IV setado
				
		R[CR] = pczinho;
		R[IPC] = pczinho+1;
				
		//famoso desvio
		pczinho = 0x00000003;//0x0000000C
}
int operacoes(FILE* output, FILE* outterminal){ //aquele switch case bolado
	
	uint32_t famosaflag = 1;
	switch(opcode){
			case 0x00: //000000
				if(memoria[pczinho] == 0){
					printf("nop\n");
				}
				else{
			//		printf("ADD\n");
					add(output);
				}
			break;
			case 0x01: //000001
			//	printf("ADDI\n");
				addi(output);
			break;
			case 0x02: //000010
			//	printf("SUB\n");
				sub(output);
			break;
			case 0x03: //000011
			//	printf("SUBI\n");
				subi(output);
			break;
			case 0x04: //000100
			//	printf("MUL\n");
				mul(output);
			break;
			case 0x05: //000101
			//	printf("MULI\n");
				muli(output);
			break;
			case 0x06: //000110
			//	printf("DIV\n");
				div_(output);
			break;
			case 0x07: //000111
			//	printf("DIVI\n");
				divi(output);
			break;
			case 0x08: //001000
			//	printf("CMP\n");
				cmp(output);
			break;
			case 0x09: //001001
			//	printf("CMPI\n");
				cmpi(output);
			break;
			case 0x0A: //001010
			//	printf("SHL\n");
				shl(output);
			break;
			case 0x0B: //001011
			//	printf("SHR\n");
				shr(output);
			break;
			case 0x0C: //001100
			//	printf("AND\n");
				and(output);
			break;
			case 0x0D: //001101
			//	printf("ANDI\n");
				andi(output);
			break;
			case 0x0E: //001110
			//	printf("NOT\n");
				not(output);
			break;
			case 0x0F: //001111
			//	printf("NOTI\n");
				noti(output);
			break;
			case 0x10: //010000
			//	printf("OR\n");
				or(output);
			break;
			case 0x11: //010001
			//	printf("ORI\n");
				ori(output);
			break;
			case 0x12: //010010
			//	printf("XOR\n");
				xor(output);
			break;
			case 0x13: //010011
			//	printf("XORI\n");
				xori(output);
			break;
			case 0x14: //010100
			//	printf("LDW\n");
				ldw(output);
			break;
			case 0x15: //010101
			//	printf("LDB\n");
				ldb(output);
			break;
			case 0x16: //010110
			//	printf("STW\n");
				stw(output);
			break;
			case 0x17: //010111
			//	printf("STB\n");
				stb(output,outterminal);
			break;
			case 0x1A: //011010
			//	printf("BUN\n");
				bun(output);
			break;
			case 0x1B: //011011
			//	printf("BEQ\n");
				beq(output);
			break;
			case 0x1C: //011100
			//	printf("BLT\n");
				blt(output);
			break;
			case 0x1D: //011101
			//	printf("BGT\n");
				bgt(output);
			break;
			case 0x1E: //011110
			//	printf("BNE\n");
				bne(output);
			break;
			case 0x1F: //011111
			//	printf("BLE\n");
				ble(output);
			break;
			case 0x20: //100000
			//	printf("BGE\n");
				bge(output);
			break;
			case 0x3F: //111111
			//	printf("INT\n");
				famosaflag = in_t(output,outterminal);
			break;
			case 0x18: //100101
			//	printf("PUSH\n");
				push(output);
			break;
			case 0x19: //100101
			//	printf("POP\n");
				pop(output);
			break;
			case 0x21: //100001
			//	printf("BZD\n");
				bzd(output);
			break;
			case 0x22: //100010
			//	printf("BNZ\n");
				bnz(output);
			break;
			case 0x23: //100011
			//	printf("BIV\n");
				biv(output);
			break;
			case 0x24: //100100
			//	printf("BNI\n");
				bni(output);
			break;
			case 0x25: //100101
			//	printf("CALL\n");
				call(output);
			break;
			case 0x26: //100110
			//	printf("RET\n");
				ret(output);
			break;
			case 0x27: //100111
			//	printf("IRS\n");
				isr(output);
			break;
			case 0x28: //101000
			//	printf("RETI\n");
				reti(output);
			break;	
			
			default: 					
				//TAFAREEELLLL, SAI SAII SAIIII
			//	printf("DEFAULT\n");
				verificarDefault(output);
			break;
		}
		return famosaflag;
}
int main(int argc, char* argv[]) { //Agora o Meu Coração é Um Lixeiro Azul Vazio Escroto

//	FILE* input;
//	FILE* output;
	FILE* outterminal;

//	input = fopen("poxim3.hex","r");
//	output = fopen("output.txt","w");
	outterminal = fopen("outterminal.txt","w"); 
	
	printf("#ARGS = %i\n", argc);
	printf("PROGRAMA = %s\n", argv[0]);
	printf("ARG1 = %s, ARG2 = %s\n", argv[1], argv[2]);
	// Abrindo arquivos
	FILE* input = fopen(argv[1], "r");
	FILE* output = fopen(argv[2], "w");
	
	uint32_t hexa;
	uint32_t famosaflag = 1;

	while((fscanf(input,"%x",&hexa))!=EOF) //contar quantas linhas
		cont++;

	memoria = (uint32_t*)malloc(sizeof(uint32_t)*cont-1); //alocar memoria do tamanho do vetor

	rewind(input); //ler o arquivo de novo
	
	int i =0;
	for(i=0;i<cont;i++){
		fscanf(input,"%x",&memoria[i]);
	}
	
	fprintf(output,"[START OF SIMULATION]\n");	
	inicializando();// onde tudo é zerado
//	int contador = 0;
	while(famosaflag){

		R[IR] = memoria[pczinho]; 
		opcode = (memoria[pczinho] & 0xFC000000) >> 26;
		tipoUFS(memoria[pczinho]);
		
		//carregando as instrucoes da cacheI		
		carregandoCacheI(pczinho,output,read,0);
		
		setarR0();		
		//switch das operacoes
		famosaflag = operacoes(output,outterminal);		
		//funcao watchdog
		verificarWatchdog(output);
		//funcao da fpu
		verificarCiclofpu(output);
		//contadores		
	}
	
	free(memoria);
	fclose(input);
	fclose(output);
	fclose(outterminal);
	
	return 0;
}

