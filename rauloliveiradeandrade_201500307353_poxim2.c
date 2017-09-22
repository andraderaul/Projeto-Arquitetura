#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

enum{ //pra n precisar ficar digitando R[32] até R[37]
	
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
struct floatPointUnit{
	
	float x;
	float y;
	float z;
	
	uint32_t control;
	uint32_t status;
	uint32_t op;
	uint32_t ciclo;

};

//passinhofoda2014
struct instructions tipoU;
struct instructions tipoF;
struct instructions tipoS;
//nois é bom mas ne bombom
struct watchdogTimer watchdog; 
struct term terminal;
struct floatPointUnit fpu;
//nois é papai mas ne noel
uint32_t *memoria;
uint32_t R[38]; //31 registradores de proposito geral
uint32_t opcode;

uint32_t pczinho = 0;
unsigned int fpuFlag = 0;
unsigned int relogio = 0;
void tipoUFS(uint32_t hexa){//isso daqui tirou muita repeticão de codigo <3
	
	//Formato de instrucoes tipo U
		//pegando os operandos
	tipoU.y = (hexa & 0x0000001F );
	tipoU.x = (hexa & 0x000003E0) >> 5;
	tipoU.z = (hexa & 0x0000FC00) >> 10;
		//salvando os 3 bits de extensão
	tipoU.overflowY = ((hexa << 16) >>31) <<5;
	tipoU.overflowX = ((hexa << 15) >>31) <<5;
	tipoU.overflowZ = ((hexa << 14) >>31) <<5;
		//adicionando o bit mais significativo
	tipoU.x = tipoU.x | tipoU.overflowX;
	tipoU.y = tipoU.y | tipoU.overflowY;
	tipoU.z = tipoU.z | tipoU.overflowZ;
		
	//Formato de instrucoes tipo F
	tipoF.y = (hexa & 0x0000001F);
	tipoF.x = (hexa & 0x000003E0) >> 5; 
	tipoF.im16 = (hexa & 0x03FFFC00 ) >> 10;
		
	//Formato de instrucoes tipo S
	tipoS.im26 = (hexa & 0x03FFFFFF);
	
}
void setarR0(){	
	R[0] = 0x00000000;	
}
void setarIE(FILE *output){
	
	printf("SETAR IE\n");
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
		if(((uint64_t)x + (uint64_t)y) > 0xFFFFFFFF){
			R[FR] = R[FR] | 0x00000010;
		}
		else{
			R[FR] = R[FR] & 0xFFFFFFEF;
		}	
	}	
	//sub, subi
	if(opcode == 0x02 || opcode == 0x03){
		if(((uint64_t)x - (uint64_t)y) > 0xFFFFFFFF){
			R[FR] = R[FR] | 0x00000010;
		}
		else{
			R[FR] = R[FR] & 0xFFFFFFEF;
		}	
	}	
	//mul, muli
	if(opcode == 0x04 || opcode == 0x05){
		if(((uint64_t)x * (uint64_t)y) > 0xFFFFFFFF){
			R[FR] = R[FR] | 0x00000010;
		}
		else{
			R[FR] = R[FR] & 0xFFFFFFEF;
		}	
	}	
	//div, divi
	if(opcode == 0x06 || opcode == 0x07){
		if(y == 0x00000000){
			R[FR] = R[FR] | 0x00000008;
		}
			if(x < 0 || y < 0){
				R[FR] = R[FR] | 0x00000010;
			}			
			else{
				R[FR] = R[FR] & 0xFFFFFFEF;
			}
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
void verificarIE754(float n){//os caras mais absurdos
	uint32_t* pk; 
	uint32_t k;
	
	//0x41140000
	if(n - (uint32_t)n == 0){ //tratando se for decimal ou nao
		
		R[tipoF.x] = (uint32_t)n;
	}
	else{
		pk = (uint32_t*)(&n); //fazendo cast de IEEE 754 para int
		k = (*pk); //pegando o que tem no endereco e salvando em k
		R[tipoF.x] = (uint32_t)k;
	}
}	
void verificarFpu(){	

	uint32_t expX,expY;
	
	//sacada de bruno
		//Pegando expoent X
	uint32_t* px = (uint32_t*)(&fpu.x); 
	expX = ((*px) & 0x7F80000) >> 23;
		//pegando expoent y
	uint32_t* py = (uint32_t*)(&fpu.y);
	expY = ((*py) & 0x7F80000) >> 23;
		
	fpu.op = (fpu.control & 0x0000000F); //ta certo
	fpu.ciclo = 1; //sempre que fpu é chamada no minimo o ciclo é um
	fpu.status = 0;
	fpu.control = 0x00000000; //control é zerado a cada chamada
	switch(fpu.op){
		
		case 0://Sem Operação
		
		break;
		case 1: //Adição
			fpu.z = fpu.x + fpu.y;
			fpu.ciclo = abs(expX - expY) +1;
		break;
		case 2: //Subtração
			fpu.z = fpu.x - fpu.y;
			fpu.ciclo = abs(expX - expY) +1;
		break;
		case 3: //multiplicacao
			fpu.z = fpu.x * fpu.y;
			fpu.ciclo = abs(expX - expY) +1;
		break;	
		case 4: //divisao
			if(fpu.y == 0x00000000){
				fpu.status = 0x00000001;		
			}
			else{
				fpu.z = fpu.x / fpu.y; 
				fpu.ciclo = abs(expX - expY) +1;	
			}		
		break;
		case 5: //atribuicao a x
			fpu.x = fpu.z;
		break;
		case 6: //atribuicao a y
			fpu.y = fpu.z;
		break;
		case 7: //teto
			fpu.z = ceil(fpu.z);
		break;
		case 8: //piso
			fpu.z = floor(fpu.z);
		break;
		case 9: //arredondamento
			fpu.z = round(fpu.z);
		break;
		default:
			fpu.control = 0x00000020;
			fpu.status  = 0x00000001;	
	}
	fpu.op = 0;
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
	
	//pra ver se tem over ou nem
	setarFR(R[tipoU.x],R[tipoU.y]);
	//realizando a operacao
	ERR = R[tipoU.x] * R[tipoU.y];
	R[tipoU.z] = ERR & 0x00000000FFFFFFFF;
	R[ER] = ERR >> 32;	
	setarR0(); 
	
	//exibicao da saida
	//1 min 0 maius
	fprintf(output,"mul ");imprimeRegistradores(tipoU.z,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] FR = 0x%.8X, ",R[FR]);fprintf(output,"ER = 0x%.8X, ",R[ER]);imprimeRegistradores(tipoU.z,0,output);fprintf(output," = ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," * ");imprimeRegistradores(tipoU.y,0,output);fprintf(output," = 0x%.8X\n",R[tipoU.z]);
	pczinho++;	
}
void muli(FILE* output){//tipoF
	
	uint64_t ERR;
	
	//pra ver se tem over ou nem
	setarFR(R[tipoF.y],tipoF.im16);
	//realizando operacao
	ERR = ((uint64_t)R[tipoF.y] * (uint64_t)tipoF.im16);
	R[tipoF.x] = ERR &  0x00000000FFFFFFFF;
	R[ER] = ERR >> 32;
	setarR0();
	
	//exibicao da saida
	//1 min 0 maius
	fprintf(output, "muli r%d, r%d, %d\n[F] FR = 0x%08X, ER = 0x%08X, R%d = R%d * 0x%04X = 0x%08X\n", tipoF.x, tipoF.y, tipoF.im16, R[FR], R[ER], tipoF.x, tipoF.y, tipoF.im16, R[tipoF.x]);
	pczinho++;
}
void div_ (FILE* output){//tipoU
	
	//pra ver se tem over ou nem
	setarFR(R[tipoU.x],R[tipoU.y]);
	pczinho++;
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

	//praver se tem over ou nem
	setarFR(R[tipoF.y],tipoF.im16);
	pczinho++;
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
	R[ER] = 0x00000000;
	ERR = ((uint64_t)(R[tipoU.x])) << (tipoU.y + 1);
	R[tipoU.z] = ERR & 0x00000000FFFFFFFF;
	R[ER] = ERR >> 32;
	
	//exibicao da saida
	fprintf(output,"shl ");imprimeRegistradores(tipoU.z,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", %d",tipoU.y);fprintf(output,"\n");
	fprintf(output,"[U] ER = 0x%.8X, ",R[ER]);imprimeRegistradores(tipoU.z,0,output);fprintf(output," = ");imprimeRegistradores(tipoU.x,0,output);fprintf(output," << %d = 0x%.8X\n",(tipoU.y + 1), R[tipoU.z]);
	pczinho++;
}
void shr(FILE* output){//tipoU //ta perfeito
	uint64_t ERR;

	//realizando operacoes
	ERR = ((uint64_t)R[ER]) << 32;
	ERR = ERR | ((uint64_t)R[tipoU.x]);
	R[tipoU.z] = ERR >> (tipoU.y+1);
	R[ER] = (ERR & 0xFFFFFFFF00000000);
	
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
				verificarIE754(fpu.z);
			break;								
			case 0x00002201: //y
				verificarIE754(fpu.y);
			break;
			case 0x00002200://x		
				verificarIE754(fpu.x);	
			break;
		}				
	}
	else{
		setarR0();
		R[tipoF.x] = memoria[(R[tipoF.y])+tipoF.im16];
	}
	//exibicao da saida
	fprintf(output, "ldw r%d, r%d, 0x%04X\n[F] R%d = MEM[(R%d + 0x%04X) << 2] = 0x%08X\n", tipoF.x, tipoF.y, tipoF.im16, tipoF.x, tipoF.y, tipoF.im16, R[tipoF.x]);
	pczinho++;
}
void ldb(FILE* output){//tipoF
	uint32_t byte, end, posmen;
	
	posmen = (R[tipoF.y] + tipoF.im16)>>2; //DESGRAÇAAAAAAAAA
	byte = (R[tipoF.y]+tipoF.im16);
	end = byte - (posmen<<2);

	setarLDB(posmen,end,tipoF.x);
	
	//exibicao da saida
	fprintf(output, "ldb r%d, r%d, 0x%04X\n[F] R%d = MEM[R%d + 0x%04X] = 0x%02X\n", tipoF.x, tipoF.y, tipoF.im16, tipoF.x, tipoF.y, tipoF.im16, R[tipoF.x]);
	pczinho++;
}
void stw(FILE* output){//tipoF
	uint32_t aux = (R[tipoF.x + tipoF.im16]);
	//watchdog
	if(aux == 0x00002020){
		watchdog.EN = R[tipoF.y] >> 31;
		watchdog.counter = (R[tipoF.y]<<1)>>1;
	}
	//fpu
	else if(aux == 0x00002203 || aux == 0x00002202 || aux == 0x00002201 || 0x00002200){
		switch(aux){
			case 0x00002203://control
				fpu.control = R[tipoF.y];
				verificarFpu();
				fpuFlag = 1; 
			break;	
			case 0x00002202://z
				fpu.z = R[tipoF.y];
			break;	
			case 0x00002201://y
				fpu.y = R[tipoF.y];
			break;	
			case 0x00002200://x
				fpu.x = R[tipoF.y];
			break;		
			}
	}
	else{
		setarR0();
		//viagem psicodelica
		memoria[(R[tipoF.x] + tipoF.im16)] = R[tipoF.y];
	}
	//exibicao da saida
	fprintf(output, "stw r%d, 0x%04X, r%d\n[F] MEM[(R%d + 0x%04X) << 2] = R%d = 0x%08X\n", tipoF.x, tipoF.im16, tipoF.y, tipoF.x, tipoF.im16, tipoF.y, R[tipoF.y]);
	pczinho++;	
}
void stb(FILE* output,FILE* outterminal){	//tipoF
	
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
		fprintf(output, "stb r%d, 0x%04X, r%d\n[F] MEM[R%d + 0x%04X] = R%d = 0x%02X\n", tipoF.x, tipoF.im16, tipoF.y, tipoF.x, tipoF.im16, tipoF.y,terminal.ter);
		fprintf(outterminal,"%c",terminal.ter);		
	}
	else{	
		//exibicao da saida
		fprintf(output, "stb r%d, 0x%04X, r%d\n[F] MEM[R%d + 0x%04X] = R%d = 0x%02X\n", tipoF.x, tipoF.im16, tipoF.y, tipoF.x, tipoF.im16, tipoF.y, R[tipoF.y]);	
	}
	pczinho++;
}
void bun(FILE* output){ //tipoS
	
	pczinho = tipoS.im26;	
	fprintf(output, "bun 0x%08X\n[S] PC = 0x%08X\n", tipoS.im26, pczinho<<2);	
}
void beq(FILE* output){ //tipoS
	
	fprintf(output,"beq 0x%.8X\n",tipoS.im26);
	if((R[FR] & 0x00000001) == 0x00000001){ //EQ
		pczinho = tipoS.im26;
	}
	else{
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho << 2);
}
void blt(FILE* output){ //tipoS
	
	fprintf(output,"blt 0x%.8X\n",tipoS.im26);
	
	if((R[FR] & 0x00000002) == 0x00000002){ //LT
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

	if((R[FR] & 0x00000001) == 0x00000001){ //~EQ
		pczinho++;
	}
	else{	
		pczinho = tipoS.im26;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho << 2);
}
void ble(FILE* output){ //tipoS
	
	fprintf(output,"ble 0x%.8X\n",tipoS.im26);

	if(((R[FR] & 0x00000001) == 0x00000001) || ((R[FR] & 0x00000002) == 0x00000020)){ //LT v EQ
		pczinho = tipoS.im26;		
	}
	else{
		pczinho++;
	}
	fprintf(output,"[S] PC = 0x%.8X\n",pczinho<2);
}
void bge(FILE* output){//tipoS
	
	fprintf(output,"bge 0x%.8X\n",tipoS.im26);

	if(((R[FR] & 0x00000005) == 0x00000001)||((R[FR] & 0x00000005) == 00000004)){ //GT v EQ
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
}
void push(FILE* output){//tipoU
	
	//stw
	memoria[(R[tipoU.x] + 0)] = R[tipoU.y];
	//subi
//	memoria[R[tipoU.x-1]] = R[tipoU.y];
	R[tipoU.x] = R[tipoU.x] - 1;
	
	fprintf(output,"push ");imprimeRegistradores(tipoU.x,1,output);fprintf(output,", ");imprimeRegistradores(tipoU.y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] MEM[");imprimeRegistradores(tipoU.x,0,output);fprintf(output,"--] = ");imprimeRegistradores(tipoU.y,0,output);fprintf(output," = 0x%.8X\n",R[tipoU.y]);
	pczinho++;
}
void pop(FILE* output){//tipoU

	//addi
	R[tipoU.y] = R[tipoU.y] + 1;
	//ldw
	setarR0();
	R[tipoU.x] = memoria[(R[tipoU.y] + 0)];
	
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
		if(watchdog.counter == 0x00000000 && watchdog.EN == 0x00000001){	
			//checando se o IE ta setado, EN so é 1 pois vai ser zerado logo a baixo
			R[CR] = 0xE1AC04DA;	
			//IPC recebe PC
			R[IPC] = pczinho;
			printf("WATCHDOG\n");
			if((R[FR] & 0x0000040) == 0x0000040){ 
				fprintf(output,"[HARDWARE INTERRUPTION 1]\n");				
				//famoso desvio
				pczinho = 0x00000001;//0x00000004
			}			
		}
		
		if(watchdog.EN == 1){ //checa se o EN ta setado
 			if(watchdog.counter != 0x00000000){ // so decrementa se n for zero
				watchdog.counter--;
			}
			else { //se for zero reseta o EN
				watchdog.EN = 0x00000000;
			}
		}
}
void verificarCiclofpu(FILE* output){  //ciclo menstrual da fpu
	
	if(fpuFlag == 1 && relogio < fpu.ciclo){
		relogio++;
	}	
	else if(fpuFlag == 1 && relogio == fpu.ciclo){
		R[CR] = 0x01EEE754;			
		//IPC recebe PC
		R[IPC] = pczinho;
		//checando se o IE ta setado, e o ciclo é  zero
		if((R[FR] & 0x00000040) == 0x00000040){	
			fprintf(output,"[HARDWARE INTERRUPTION 2]\n");
			//famoso desvio
			pczinho = 0x00000002;//0x00000008
			fpuFlag = 0; relogio = 0;
			fpu.ciclo = 1; 	
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
					add(output);
				}
			break;
			case 0x01: //000001
				addi(output);
			break;
			case 0x02: //000010
				sub(output);
			break;
			case 0x03: //000011
				subi(output);
			break;
			case 0x04: //000100
				mul(output);
			break;
			case 0x05: //000101
				muli(output);
			break;
			case 0x06: //000110
				div_(output);
			break;
			case 0x07: //000111
				divi(output);
			break;
			case 0x08: //001000
				cmp(output);
			break;
			case 0x09: //001001
				cmpi(output);
			break;
			case 0x0A: //001010
				shl(output);
			break;
			case 0x0B: //001011
				shr(output);
			break;
			case 0x0C: //001100
				and(output);
			break;
			case 0x0D: //001101
				andi(output);
			break;
			case 0x0E: //001110
				not(output);
			break;
			case 0x0F: //001111
				noti(output);
			break;
			case 0x10: //010000
				or(output);
			break;
			case 0x11: //010001
				ori(output);
			break;
			case 0x12: //010010
				xor(output);
			break;
			case 0x13: //010011
				xori(output);
			break;
			case 0x14: //010100
				ldw(output);
			break;
			case 0x15: //010101
				ldb(output);
			break;
			case 0x16: //010110
				stw(output);
			break;
			case 0x17: //010111
				stb(output,outterminal);
			break;
			case 0x1A: //011010
				bun(output);
			break;
			case 0x1B: //011011
				beq(output);
			break;
			case 0x1C: //011100
				blt(output);
			break;
			case 0x1D: //011101
				bgt(output);
			break;
			case 0x1E: //011110
				bne(output);
			break;
			case 0x1F: //011111
				ble(output);
			break;
			case 0x20: //100000
				bge(output);
			break;
			case 0x3F: //111111
				famosaflag = in_t(output,outterminal);
			break;
			case 0x18: //100101
				push(output);
			break;
			case 0x19: //100101
				pop(output);
			break;
			case 0x21: //100001
				bzd(output);
			break;
			case 0x22: //100010
				bnz(output);
			break;
			case 0x23: //100011
				biv(output);
			break;
			case 0x24: //100100
				bni(output);
			break;
			case 0x25: //100101
				call(output);
			break;
			case 0x26: //100110
				ret(output);
			break;
			case 0x27: //100111
				isr(output);
			break;
			case 0x28: //101000
				reti(output);
			break;	
			
			default: 					
				//TAFAREEELLLL, SAI SAII SAIIII
				verificarDefault(output);
			break;
		} 
		return famosaflag;
}
int main(int argc, char* argv[]) { //Agora o Meu Coração é Um Lixeiro Azul Vazio Escroto

//	FILE* input;
//	FILE* output;
	FILE* outterminal;

//	input = fopen("input.hex","r");
//	output = fopen("output.txt","w");
	outterminal = fopen("outterminal.txt","w"); 
	
	printf("#ARGS = %i\n", argc);
	printf("PROGRAMA = %s\n", argv[0]);
	printf("ARG1 = %s, ARG2 = %s\n", argv[1], argv[2]);
	// Abrindo arquivos
	FILE* input = fopen(argv[1], "r");
	FILE* output = fopen(argv[2], "w");; 
	
	
	

	uint32_t hexa;
	uint32_t famosaflag = 1;
	unsigned int cont = 0;

	while((fscanf(input,"%x",&hexa))!=EOF) //contar quantas linhas
		cont++;

	memoria = (uint32_t*)malloc(sizeof(uint32_t)*cont-1); //alocar memoria do tamanho do vetor

	rewind(input); //ler o arquivo de novo
	
	int i =0;
	for(i=0;i<cont;i++){
		fscanf(input,"%x",&memoria[i]);
	}
	
	fprintf(output,"[START OF SIMULATION]\n");
	i = 0;
	//int contador = 96;
	while(famosaflag){

		R[IR] = memoria[pczinho];
		opcode = (memoria[pczinho] & 0xFC000000) >> 26;
		tipoUFS(memoria[pczinho]);
		setarR0();		
		//switch das operacoes
		famosaflag = operacoes(output,outterminal);		
		//funcao watchdog
		verificarWatchdog(output);
		//funcao da fpu
		verificarCiclofpu(output);
		//contadores		
		//printf("EN = %d Watchdog = %d\n",watchdog.EN,watchdog.counter);
		//printf("Ciclo: %d --- IPC durante este ciclo: 0x%08X \n",fpu.ciclo,R[IPC]>>2);		
		//printf("PC = 0x%08X IPC = 0x%08X flag = %d Ciclo = %d Relogio = %d\n",pczinho>>2,R[IPC]>>2,fpuFlag,fpu.ciclo,relogio);		
				
	}
	
	return 0;
}

