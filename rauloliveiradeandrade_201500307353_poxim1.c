#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

enum{

	PC = 32,
	IR = 33,
	ER = 34,
	FR = 35

};

uint32_t registradores[36];
uint32_t *memoria;
uint32_t opcode;
unsigned int i = 0;

uint32_t get_Y(uint32_t hexa){ //pegar o x, tipo U tipo F

	uint32_t y;

	y = hexa << 27;
	y = y >> 27;

	return y;

}
uint32_t get_X(uint32_t hexa){ //pegar o y, tipo U tipo F

	uint32_t x;

	x = hexa << 22;
	x = x >> 27; //(22+5)

	return x;
}
uint32_t get_Z(uint32_t hexa){ //pegar o z, tipo U tipo F

	uint32_t z;

	z = hexa << 17;
	z = z >> 27; //(17+10)

	return z;

}
uint32_t get_IM16(uint32_t hexa){ // pegar o IM16, tipo F

	uint32_t im16;

	im16 = hexa << 6;
	im16 = im16 >> 16;

	return im16;

}
uint32_t get_IM26(uint32_t hexa){ //pegar o IM26, tipo S

	uint32_t im26;

	im26 = hexa << 6;
	im26 = im26 >> 6;

	return im26;

}
uint32_t get_OverY(uint32_t hexa){ // Extensão do Y tipo U
	uint32_t overflowY;

	overflowY = hexa << 16;
	overflowY = overflowY >> 31;
	overflowY = overflowY << 5;

	return overflowY;
}
uint32_t get_OverX(uint32_t hexa){ //Extensão do X tipo U
	uint32_t overflowX;

	overflowX = hexa << 15;
	overflowX = overflowX >> 31;
	overflowX = overflowX << 5;

	return overflowX;
}
uint32_t get_OverZ(uint32_t hexa){ //Extensão do Z tipo U
	uint32_t overflowZ;

	overflowZ = hexa << 14;
	overflowZ = overflowZ >> 31;
	overflowZ = overflowZ << 5;

	return overflowZ;
}
void setarR0(){
	registradores[0] = 0x0000000;
}

void imprimeRegistradores(uint32_t r,uint32_t esc, FILE* output){
	//1 minusculo //0 maiusculo	
	switch(r){
		
		case 32: //PC
			
			if(esc){
				fprintf(output,"pc");
			}
			else{
				fprintf(output,"PC");
			}
			
		break;
		
		case 33: //IR
			
			if(esc){
				fprintf(output,"ir");
			}
			else{
				fprintf(output,"IR");
			}
			
		break;
		
		case 34: //ER
			
			if(esc){
				fprintf(output,"er");
			}
			else{
				fprintf(output,"ER");
			}
			
		break;
		
		case 35: //FR
			
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
void setarFR(uint32_t x,uint32_t y){
	
	//add, addi
	if(opcode == 0x00 || opcode == 0x01){
		if(((uint64_t)x + (uint64_t)y) > 0xFFFFFFFF){
			registradores[FR] = registradores[FR] | 0x00000010;
		}
		else{
			registradores[FR] = registradores[FR] & 0xFFFFFFEF;
		}	
	}
	
	//sub, subi
	if(opcode == 0x02 || opcode == 0x03){
		if(((uint64_t)x - (uint64_t)y) > 0xFFFFFFFF){
			registradores[FR] = registradores[FR] | 0x00000010;
		}
		else{
			registradores[FR] = registradores[FR] & 0xFFFFFFEF;
		}	
	}
	
	//mul, muli
	if(opcode == 0x04 || opcode == 0x05){
		if(((uint64_t)x * (uint64_t)y) > 0xFFFFFFFF){
			registradores[FR] = registradores[FR] | 0x00000010;
		}
		else{
			registradores[FR] = registradores[FR] & 0xFFFFFFEF;
		}	
	}
	
	//div, divi
	if(opcode == 0x06 || opcode == 0x07){
		if(y == 0x00000000){
			registradores[FR] = registradores[FR] | 0x00000008;
		}
			if(x < 0 || y < 0){
				registradores[FR] = registradores[FR] | 0x00000010;
			}			
			else{
				registradores[FR] = registradores[FR] & 0xFFFFFFEF;
			}
	}
	
	//cmp, cmpi
	if(opcode == 0x08 || opcode == 0x09){
		
		registradores[FR] = registradores[FR] & 0xFFFFFFF8;
	
		//EQ
		if(x == y){
			registradores[FR] = registradores[FR] | 0x00000001;
		}
		
		//LT
		if(x < y){
			registradores[FR] = registradores[FR] | 0x00000002;
		}
		
		//GT
		if(x > y){
			registradores[FR] = registradores[FR] | 0x00000004;
		}	
	}
}
//vei demorou essa sacada
void setarLDB(uint32_t byte, uint32_t im16, uint32_t x){
	
		switch(im16){
			case 0:
				registradores[x] = (byte & 0xFF000000) >> 24;
			break;
			case 1:
				registradores[x] = (byte & 0x00FF0000) >> 16;
			break;
			case 2:
				registradores[x] = (byte & 0x0000FF00) >> 8;
			break;
			case 3:
				registradores[x] = (byte & 0x000000FF);
			break;
		}	
	}
	
void setarSTW(uint32_t memoriaatual, uint32_t instrucao, uint32_t y){
	
//	if(memoria[PC] == memoriaatual){
	if(registradores[IR] == memoriaatual){
		registradores[y] = instrucao;
	}

}
void setarSTB(uint32_t end,uint32_t rx, uint32_t ry, uint32_t im16, uint32_t y){
	
	if(memoria[PC] == (((rx + im16)/4)*4)){
		switch(end){
			case 0:
				registradores[y] = registradores[y] & 0x00FFFFFF;
				registradores[y] = registradores[y] | (ry << 24);
			break;
			case 1:
				registradores[y] = registradores[y] & 0xFF00FFFF;
				registradores[y] = registradores[y] | (ry << 16);
			break;
			case 2:
				registradores[y] = registradores[y] & 0xFFFF00FF;
				registradores[y] = registradores[y] | (ry << 8);
			break;
			case 3:
				registradores[y] = registradores[y] & 0xFFFFFF00;
				registradores[y] = registradores[y] | (ry);
			break;
		}
	
	}
	
}
void add(uint32_t hexa, FILE* output){
	
	uint32_t x, y, z;
	uint32_t overflowY, overflowX,overflowZ;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	z = get_Z(hexa);
	
	//salvando os 3bits de extensão
	overflowX = get_OverX(hexa);	
	overflowY = get_OverY(hexa);
	overflowZ = get_OverZ(hexa);
	
	//adicionando o bit mais significativo
	x = x | overflowX;
	y = y | overflowY;
	z = z | overflowZ;

	//dibraldinho
	if(x == PC){
		printf("ENTROU add\n");
		registradores[x] = i;
	}
	if(y == PC){
		printf("ENTROU add\n");
		registradores[y] = i;
	}
	

		//pra ver se tem over ou nem
	setarFR(registradores[x],registradores[y]);	
	//realizando a soma
	registradores[z] = registradores[x] + registradores[y];
	
	setarR0();
	
//	printf("RX 0x%.8X + RY 0x%.8X ",registradores[x],registradores[y]);	
//	printf(" = rz 0x%.8X \n",registradores[z]);	

	
	//1 min 0 maius
	//exibicao da saida
	fprintf(output,"add ");imprimeRegistradores(z,1,output);fprintf(output,", ");imprimeRegistradores(x,1,output);fprintf(output,", ");imprimeRegistradores(y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] FR = 0x%.8X, ",registradores[FR]);imprimeRegistradores(z,0,output);fprintf(output," = ");imprimeRegistradores(x,0,output);fprintf(output," + ");imprimeRegistradores(y,0,output);fprintf(output," = 0x%.8X\n",registradores[z]);

}
void addi(uint32_t hexa, FILE* output){
	
	uint32_t x, y, im16;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	im16 = get_IM16(hexa);
	
	//pra ver se tem over ou nem
	setarFR(registradores[y],im16);
	
	//realizando a operacao
	registradores[x] = registradores[y] + im16;
	
	setarR0();
	
	//exibicao da saida
	fprintf(output, "addi r%d, r%d, %d\n[F] FR = 0x%08X, R%d = R%d + 0x%04X = 0x%08X\n", x, y, im16, registradores[FR], x, y, im16, registradores[x]);
	

} 
void sub(uint32_t hexa, FILE* output){
	
	uint32_t x, y, z;
	uint32_t overflowY, overflowX,overflowZ;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	z = get_Z(hexa);
	
	//salvando os 3bits de extensão
	overflowX = get_OverX(hexa);	
	overflowY = get_OverY(hexa);
	overflowZ = get_OverZ(hexa);
	
	//adicionando o bit mais significativo
	x = x | overflowX;
	y = y | overflowY;
	z = z | overflowZ;
	
	//dibraldinho
	if(x == PC){
		printf("ENTROU sub\n");
		registradores[x] = i;
	}
	if(y == PC){
		printf("ENTROU sub\n");
		registradores[y] = i;
	}
	
	//pra ver se tem over ou nem
	setarFR(registradores[x],registradores[y]);
	
	//realizando a soma
	registradores[z] = registradores[x] - registradores[y];
	
	setarR0();

	//1 min 0 maius
	//exibicao da saida
	fprintf(output,"sub ");imprimeRegistradores(z,1,output);fprintf(output,", ");imprimeRegistradores(x,1,output);fprintf(output,", ");imprimeRegistradores(y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] FR = 0x%.8X, ",registradores[FR]);imprimeRegistradores(z,0,output);fprintf(output," = ");imprimeRegistradores(x,0,output);fprintf(output," - ");imprimeRegistradores(y,0,output);fprintf(output," = 0x%.8X\n",registradores[z]);


}

void subi(uint32_t hexa, FILE* output){

	uint32_t x, y, im16;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	im16 = get_IM16(hexa);
	
	//pra ver se tem over ou nem
	setarFR(registradores[y],im16);
	
	//realizando a operacao
	registradores[x] = registradores[y] - im16;
	
	setarR0();
	
	//exibicao da saida
	fprintf(output, "subi r%d, r%d, %d\n[F] FR = 0x%08X, R%d = R%d - 0x%04X = 0x%08X\n", x, y, im16, registradores[FR], x, y, im16, registradores[x]);

}
void mul(uint32_t hexa, FILE* output){
	
	uint32_t x, y, z;
	uint32_t overflowY, overflowX,overflowZ;
	uint64_t ERR;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	z = get_Z(hexa);
	
	//salvando os 3bits de extensão
	overflowX = get_OverX(hexa);	
	overflowY = get_OverY(hexa);
	overflowZ = get_OverZ(hexa);
	
	//adicionando o bit mais significativo
	x = x | overflowX;
	y = y | overflowY;
	z = z | overflowZ;
	
		//dibraldinho
	if(x == PC){
		printf("ENTROU mul\n");
		registradores[x] = i;
	}
	if(y == PC){
		printf("ENTROU mul\n");
		registradores[y] = i;
	}
	
	//pra ver se tem over ou nem
	setarFR(registradores[x],registradores[y]);
	
	//realizando operacao
	ERR = registradores[x] * registradores[y];
	registradores[z] = ERR & 0x00000000FFFFFFFF;
	registradores[ER] = ERR >> 32;
	
	setarR0();
	
	//exibicao da saida
	//1 min 0 maius
	fprintf(output,"mul ");imprimeRegistradores(z,1,output);fprintf(output,", ");imprimeRegistradores(x,1,output);fprintf(output,", ");imprimeRegistradores(y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] FR = 0x%.8X, ",registradores[FR]);fprintf(output,"ER = 0x%.8X, ",registradores[ER]);imprimeRegistradores(z,0,output);fprintf(output," = ");imprimeRegistradores(x,0,output);fprintf(output," * ");imprimeRegistradores(y,0,output);fprintf(output," = 0x%.8X\n",registradores[z]);
	
}
void muli(uint32_t hexa, FILE* output){
	
	uint32_t x, y, im16;
	uint64_t ERR;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	im16 = get_IM16(hexa);
	
	//pra ver se tem over ou nem
	setarFR(registradores[y],im16); 
		
	//realizando operacao
	ERR = ((uint64_t)registradores[y]) * ((uint64_t)im16);
	registradores[x] = ERR & 0x00000000FFFFFFFF;
	registradores[ER] = ERR >> 32;
	
	setarR0();
	
	fprintf(output, "muli r%d, r%d, %d\n[F] FR = 0x%08X, ER = 0x%08X, R%d = R%d * 0x%04X = 0x%08X\n", x, y, im16, registradores[FR], registradores[ER], x, y, im16, registradores[x]);
				
}
void div_(uint32_t hexa, FILE* output){
	
	uint32_t x, y, z;
	uint32_t overflowY, overflowX,overflowZ;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	z = get_Z(hexa);
	
	//salvando os 3bits de extensão
	overflowX = get_OverX(hexa);	
	overflowY = get_OverY(hexa);
	overflowZ = get_OverZ(hexa);
	
	//adicionando o bit mais significativo
	x = x | overflowX;
	y = y | overflowY;
	z = z | overflowZ;
	
		//dibraldinho
	if(x == PC){
		printf("ENTROU div\n");
		registradores[x] = i;
	}
	if(y == PC){
		printf("ENTROU div\n");
		registradores[y] = i;
	}
	
	////pra ver se tem over ou nem
	setarFR(registradores[x],registradores[y]);
	
	if(registradores[y]!= 0x00000000){
		registradores[z] = registradores[x] /registradores[y];
		registradores[ER] = registradores[x] % registradores[y];
	}
	
	setarR0();

	//exibicao da saida
	//1 min 0 maius
	fprintf(output,"div ");imprimeRegistradores(z,1,output);fprintf(output,", ");imprimeRegistradores(x,1,output);fprintf(output,", ");imprimeRegistradores(y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] FR = 0x%.8X, ",registradores[FR]);fprintf(output,"ER = 0x%.8X, ",registradores[ER]);imprimeRegistradores(z,0,output);fprintf(output," = ");imprimeRegistradores(x,0,output);fprintf(output," / ");imprimeRegistradores(y,0,output);fprintf(output," = 0x%.8X\n",registradores[z]);
			
}
void divi(uint32_t hexa, FILE* output){
	
	uint32_t x, y, im16;
		
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	im16 = get_IM16(hexa);
	
	//pra ver se tem over ou nem
	setarFR(registradores[y],im16); 
	
	if(im16 !=0x00000000){
		registradores[ER] = registradores[y] % im16;
		registradores[x] = registradores[y] / im16;	
	}
	
	setarR0();
	
	fprintf(output, "divi r%d, r%d, %d\n[F] FR = 0x%08X, ER = 0x%08X, R%d = R%d / 0x%04X = 0x%08X\n", x, y, im16, registradores[FR], registradores[ER], x, y, im16, registradores[x]);
}
void cmp(uint32_t hexa, FILE* output){
	uint32_t x, y;
	uint32_t overflowY, overflowX;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	
	//salvando os 3bits de extensão
	overflowX = get_OverX(hexa);	
	overflowY = get_OverY(hexa);
	
	//adicionando o bit mais significativo
	x = x | overflowX;
	y = y | overflowY;
	
	setarFR(registradores[x],registradores[y]);
	
	//exibicao da saida
	//1 min 0 maius
	fprintf(output,"cmp ");imprimeRegistradores(x,1,output);fprintf(output,", ");imprimeRegistradores(y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] FR = 0x%.8X\n",registradores[FR]);
}
void cmpi(uint32_t hexa, FILE* output){
	
	uint32_t x, im16;
		
	//pegando os operandos 
	x = get_X(hexa);
	im16 = get_IM16(hexa);
	
	//pra ver se tem over ou nem
	setarFR(registradores[x],im16); 
	fprintf(output, "cmpi r%d, %d\n[F] FR = 0x%08X\n", x, im16, registradores[FR]);
}
void shl(uint32_t hexa, FILE* output){
	
	uint32_t x, y, z;
	uint32_t overflowY, overflowX,overflowZ;
	uint64_t ERR;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	z = get_Z(hexa);
	
	//salvando os 3bits de extensão
	overflowX = get_OverX(hexa);	
	overflowY = get_OverY(hexa);
	overflowZ = get_OverZ(hexa);
	
	//adicionando o bit mais significativo
	x = x | overflowX;
	y = y | overflowY;
	z = z | overflowZ;
		
	//realizando operacoes
	registradores[ER] = 0x00000000;
	ERR = ((uint64_t)(registradores[x])) << (y + 1);
	registradores[z] = ERR & 0x00000000FFFFFFFF;
	registradores[ER] = ERR >> 32;
	
	//exibicao da saida
	fprintf(output,"shl ");imprimeRegistradores(z,1,output);fprintf(output,", ");imprimeRegistradores(x,1,output);fprintf(output,", %d",y);fprintf(output,"\n");
	fprintf(output,"[U] ER = 0x%.8X, ",registradores[ER]);imprimeRegistradores(z,0,output);fprintf(output," = ");imprimeRegistradores(x,0,output);fprintf(output," << %d = 0x%.8X\n",(y + 1), registradores[z]);
	printf("Z %d X %d Y %d",registradores[z],registradores[x],registradores[y]);
	getchar();
	

}
void shr(uint32_t hexa, FILE* output){ //Ta perfeito

	uint32_t x, y, z;
	uint32_t overflowY, overflowX,overflowZ;
	uint64_t ERR;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	z = get_Z(hexa);
	
	//salvando os 3bits de extensão
	overflowX = get_OverX(hexa);	
	overflowY = get_OverY(hexa);
	overflowZ = get_OverZ(hexa);
	
	//adicionando o bit mais significativo
	x = x | overflowX;
	y = y | overflowY;
	z = z | overflowZ;
	
	//realizando operacoes
	ERR = ((uint64_t)registradores[ER]) << 32;
	ERR = ERR | ((uint64_t)registradores[x]);
	registradores[z] = ERR >> (y+1);
	registradores[ER] = (ERR & 0xFFFFFFFF00000000);
	
	//exibicao da saida
	fprintf(output,"shr ");imprimeRegistradores(z,1,output);fprintf(output,", ");imprimeRegistradores(x,1,output);fprintf(output,", %d",y);fprintf(output,"\n");
	fprintf(output,"[U] ER = 0x%.8X, ",registradores[ER]);imprimeRegistradores(z,0,output);fprintf(output," = ");imprimeRegistradores(x,0,output);fprintf(output," >> %d = 0x%.8X\n",(y + 1), registradores[z]);
	
}
void and(uint32_t hexa, FILE* output){
	
	uint32_t x, y, z;
	uint32_t overflowY, overflowX,overflowZ;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	z = get_Z(hexa);
	
	//salvando os 3bits de extensão
	overflowX = get_OverX(hexa);	
	overflowY = get_OverY(hexa);
	overflowZ = get_OverZ(hexa);
	
	//adicionando o bit mais significativo
	x = x | overflowX;
	y = y | overflowY;
	z = z | overflowZ;

	//realizando a operacao
	registradores[z] = registradores[x] & registradores[y];
	
	//exibicao da saida
	fprintf(output,"and ");imprimeRegistradores(z,1,output);fprintf(output,", ");imprimeRegistradores(x,1,output);fprintf(output,", ");imprimeRegistradores(y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] ");imprimeRegistradores(z,0,output);fprintf(output," = ");imprimeRegistradores(x,0,output);fprintf(output," & ");imprimeRegistradores(y,0,output);fprintf(output," = 0x%.8X\n",registradores[z]);			
	
}
void andi(uint32_t hexa, FILE* output){
	
	uint32_t x, y, im16;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	im16 = get_IM16(hexa);
	
	//realizando a operacao
	registradores[x] = registradores[y] & im16;
	
	//exibicao da saida
	fprintf(output,"andi r%d, r%d, %d\n[F] R%d = R%d & 0x%04X = 0x%08X\n", x, y, im16, x, y, im16, registradores[x]);
				
	
}
void not(uint32_t hexa, FILE* output){
	
	uint32_t x, y;
	uint32_t overflowY, overflowX;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);

	
	//salvando os 3bits de extensão
	overflowX = get_OverX(hexa);	
	overflowY = get_OverY(hexa);
	
	//adicionando o bit mais significativo
	x = x | overflowX;
	y = y | overflowY;

	//realizando a operacao
	registradores[x] = ~registradores[y];
	
	//exibicao da saida
	fprintf(output,"not ");imprimeRegistradores(x,1,output);fprintf(output,", ");imprimeRegistradores(y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] ");imprimeRegistradores(x,0,output);fprintf(output," = ~");imprimeRegistradores(y,0,output);fprintf(output," = 0x%.8X\n",registradores[x]);
}
void noti(uint32_t hexa, FILE* output){
	
	uint32_t x, im16;
	
	//pegando os operandos 
	x = get_X(hexa);
	im16 = get_IM16(hexa);
	
	//realizando a operacao
	registradores[x] = ~im16;
	
	//exibicao da saida
	fprintf(output,"noti r%d, %d\n[F] R%d = ~0x%.4X = 0x%.8X\n", x, im16, x,im16,registradores[x]);
				
	
}
void or(uint32_t hexa, FILE* output){
	
	uint32_t x, y, z;
	uint32_t overflowY, overflowX,overflowZ;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	z = get_Z(hexa);
	
	//salvando os 3bits de extensão
	overflowX = get_OverX(hexa);	
	overflowY = get_OverY(hexa);
	overflowZ = get_OverZ(hexa);
	
	//adicionando o bit mais significativo
	x = x | overflowX;
	y = y | overflowY;
	z = z | overflowZ;

	//realizando a operacao
	registradores[z] = registradores[x] | registradores[y];
	
	//exibicao da saida
	fprintf(output,"or ");imprimeRegistradores(z,1,output);fprintf(output,", ");imprimeRegistradores(x,1,output);fprintf(output,", ");imprimeRegistradores(y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] ");imprimeRegistradores(z,0,output);fprintf(output," = ");imprimeRegistradores(x,0,output);fprintf(output," | ");imprimeRegistradores(y,0,output);fprintf(output," = 0x%.8X\n",registradores[z]);			
	
}
void ori(uint32_t hexa, FILE* output){
	
	uint32_t x, y, im16;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	im16 = get_IM16(hexa);
	
	//realizando a operacao
	registradores[x] = registradores[y] | im16;
	
	//exibicao da saida
	fprintf(output,"ori r%d, r%d, %d\n[F] R%d = R%d | 0x%04X = 0x%08X\n", x, y, im16, x, y, im16, registradores[x]);
				
	
}
void xor(uint32_t hexa, FILE* output){
	
	uint32_t x, y, z;
	uint32_t overflowY, overflowX,overflowZ;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	z = get_Z(hexa);
	
	//salvando os 3bits de extensão
	overflowX = get_OverX(hexa);	
	overflowY = get_OverY(hexa);
	overflowZ = get_OverZ(hexa);
	
	//adicionando o bit mais significativo
	x = x | overflowX;
	y = y | overflowY;
	z = z | overflowZ;

	//realizando a operacao
	registradores[z] = registradores[x] ^ registradores[y];
	
	//exibicao da saida
	//exibicao da saida
	fprintf(output,"xor ");imprimeRegistradores(z,1,output);fprintf(output,", ");imprimeRegistradores(x,1,output);fprintf(output,", ");imprimeRegistradores(y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] ");imprimeRegistradores(z,0,output);fprintf(output," = ");imprimeRegistradores(x,0,output);fprintf(output," ^ ");imprimeRegistradores(y,0,output);fprintf(output," = 0x%.8X\n",registradores[z]);			

}
void xori(uint32_t hexa, FILE* output){
	
	uint32_t x, y, im16;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	im16 = get_IM16(hexa);
	
	//realizando a operacao
	registradores[x] = registradores[y] ^ im16;
	
	//exibicao da saida
	fprintf(output,"xori r%d, r%d, %d\n[F] R%d = R%d ^ 0x%04X = 0x%08X\n", x, y, im16, x, y, im16, registradores[x]);
				
	
}
void ldw(uint32_t hexa, FILE* output){
	
	uint32_t x,y,im16;
	
	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	im16 = get_IM16(hexa);
	
	setarR0();
	registradores[x] = memoria[(registradores[y]) + im16];
	//exibicao da saida
	fprintf(output, "ldw r%d, r%d, 0x%04X\n[F] R%d = MEM[(R%d + 0x%04X) << 2] = 0x%08X\n", x, y, im16, x, y, im16, registradores[x]);
			
}
void ldb(uint32_t hexa, FILE* output){
	uint32_t x,y,im16;
	uint32_t byte;

	//pegando os operandos 
	x = get_X(hexa);
	y = get_Y(hexa);
	im16 = get_IM16(hexa);
	
	byte = memoria[(registradores[y]+im16)>>2];

	setarLDB(byte,im16,x);
	
	//exibicao da saida
	fprintf(output, "ldb r%d, r%d, 0x%04X\n[F] R%d = MEM[R%d + 0x%04X] = 0x%02X\n", x, y, im16, x, y, im16, registradores[x]);
			
	
}
void stw(uint32_t hexa, FILE* output){
	
	uint32_t x, y, im16;
	
	im16 = get_IM16(hexa);
	y = get_Y(hexa);
	x = get_X(hexa);
	
//	setarSTW((registradores[x] + im16),registradores[y],y);
 
	//viagem psicodelica
	memoria[(registradores[x] + im16)] = registradores[y]; 
	
	//exibicao da saida
	fprintf(output, "stw r%d, 0x%04X, r%d\n[F] MEM[(R%d + 0x%04X) << 2] = R%d = 0x%08X\n", x, im16, y, x, im16, y, registradores[y]);
				
}
void stb(uint32_t hexa, FILE* output){
	
	uint32_t x, y, im16;
	uint32_t end;
//	uint64_t ERR;
	  
	im16 = get_IM16(hexa);
	y = get_Y(hexa);
	x = get_X(hexa);	
	
	//posicao do byte que sera alterado
	end = ((registradores[x] + im16)- (((registradores[x] + im16)/4)*4));
	
	setarSTB(end,registradores[x],registradores[y],im16,y);
	
	switch (end){
		case 0:
			fprintf(output, "stb r%d, 0x%04X, r%d\n[F] MEM[R%d + 0x%04X] = R%d = 0x%02X\n", x, im16, y, x, im16, y, registradores[y] & 0x000000FF);
		break;
		case 1:
			fprintf(output, "stb r%d, 0x%04X, r%d\n[F] MEM[R%d + 0x%04X] = R%d = 0x%02X\n", x, im16, y, x, im16, y, registradores[y] & 0x0000FF);
		break;
		case 2:
			fprintf(output, "stb r%d, 0x%04X, r%d\n[F] MEM[R%d + 0x%04X] = R%d = 0x%02X\n", x, im16, y, x, im16, y, registradores[y] & 0x00FF);
		break;
		case 3:
			fprintf(output, "stb r%d, 0x%04X, r%d\n[F] MEM[R%d + 0x%04X] = R%d = 0x%02X\n", x, im16, y, x, im16, y, registradores[y] & 0xFF);
		break;
	}	
}
void bun(uint32_t hexa, FILE* output){
	
	uint32_t im26;
		
	im26 = get_IM26(hexa);	
	
	registradores[PC] = im26 << 2;
	i = (registradores[PC]-1)/4;

	fprintf(output, "bun 0x%08X\n[S] PC = 0x%08X\n", im26, registradores[PC]);
}
void beq(uint32_t hexa, FILE* output){
	
	uint32_t im26;

	im26 = get_IM26(hexa);
	
	fprintf(output,"beq 0x%.8X\n",im26);
	
	if((registradores[FR] & 0b00001) == 0b00001){ //EQ
		registradores[PC] = im26 << 2;
		i = (registradores[PC]/4)-1;
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]);
	}
	else{
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]+4);
	}	

}
void blt(uint32_t hexa, FILE* output){
	
	uint32_t im26;
	
	im26 = get_IM26(hexa);
	
	fprintf(output,"blt 0x%.8X\n",im26);

	if((registradores[FR] & 0b00010) == 0b00010){ //LT
		registradores[PC] = im26 << 2;
		i = (registradores[PC]/4)-1;
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]);
	}
	else{
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]+4);
	}

}
void bgt(uint32_t hexa, FILE* output){
	
	uint32_t im26;
	
	im26 = get_IM26(hexa);
	fprintf(output,"bgt 0x%.8X\n",im26);

	if((registradores[FR] & 0x00000004) == 0x00000004){ //GT
		registradores[PC] = im26 << 2;
		i = (registradores[PC]/4)-1;
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]);
	}
	else{
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]+4);
	}
}
void bne(uint32_t hexa, FILE* output){ //rever essa funcao dps

	uint32_t im26;

	//pegando os bits do imediato
	im26 = get_IM26(hexa);

	fprintf(output,"bne 0x%.8X\n",im26);

	if((registradores[FR] & 0b00001) != 0b00001){ //~EQ
		registradores[PC] = im26 << 2;
		i = (registradores[PC]/4)-1;
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]);
	}
	else{
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]+4);
	}

}
void ble(uint32_t hexa, FILE* output){ //rever essa funcao dps

	uint32_t im26;

	//pegando os bits do imediato
	im26 = get_IM26(hexa);

	fprintf(output,"ble 0x%.8X\n",im26);

	if((registradores[FR] & 0b00011) == 0b00011){ //LT v EQ
		registradores[PC] = im26 << 2;
		i = (registradores[PC]/4)-1;
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]);
	}
	else{
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]+4);
	}

}
void bge(uint32_t hexa, FILE* output){ //rever essa funcao dps

	uint32_t im26;

	//pegando os bits do imediato
	im26 = get_IM26(hexa);

	fprintf(output,"bge 0x%.8X\n",im26);

	if(((registradores[FR] & 0b00100) == 0b00100)||((registradores[FR] & 0b00001) == 0b00001)){ //GT v EQ
		registradores[PC] = im26 << 2;
		i = (registradores[PC]/4)-1;
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]);
	}
	else{
		fprintf(output,"[S] PC = 0x%.8X\n",registradores[PC]+4);
	}

}
void in_t(uint32_t hexa, FILE* output){

	uint32_t im26;
	uint32_t CR = 0;

	//pegando os bis dos operandos
	im26 = get_IM26(hexa);

	if (im26==0){

		registradores[PC] = 0;
		fprintf(output,"int %d\n",im26);
		fprintf(output,"[S] CR = 0x%.8X, PC = 0x%.8X\n",CR,registradores[PC]);
		fprintf(output,"[END OF SIMULATION]\n");

	}
}

void call(uint32_t hexa, FILE* output){
	
	uint32_t x,y, im16;
	
	x = get_X(hexa);
	y = get_Y(hexa);
	im16 = get_IM16(hexa);
	
	registradores[x] = (registradores[PC]+4) >> 2;
	setarR0();
	registradores[PC] = (registradores[y]+im16) << 2;
	i = (registradores[PC]/4)-1;
	fprintf(output, "call r%d, r%d, 0x%04X\n[F] R%d = (PC + 4) >> 2 = 0x%08X, PC = (R%d + 0x%04X) << 2 = 0x%08X\n", x, y, im16, x, registradores[x], y, im16, registradores[PC]);
				
	
}
void ret(uint32_t hexa, FILE* output){
	
	uint32_t x;
	
	x = get_X(hexa);
	
	registradores[PC] = registradores[x] << 2;
	i = (registradores[PC]/4)-1;
	fprintf(output, "ret r%d\n[F] PC = R%d << 2 = 0x%08X\n", x, x, registradores[PC]);

}
void push(uint32_t hexa, FILE* output){
	uint32_t x,y;
	uint32_t overflowY,overflowX;
	
	x = get_X(hexa);
	y = get_Y(hexa);
	
	overflowX = get_OverX(hexa);
	overflowY = get_OverY(hexa);
	
	x = x | overflowX;
	y = y | overflowY;
	
	//dibraldinho
	if(y == PC){		
		printf("ENTROU PUSH\n");
		registradores[y] = i;
	}
	
	//stw
	memoria[(registradores[x] + 0)] = registradores[y];
	
	//subi
	registradores[x] = registradores[x] - 1;
	
	fprintf(output,"push ");imprimeRegistradores(x,1,output);fprintf(output,", ");imprimeRegistradores(y,1,output);fprintf(output,"\n");
	fprintf(output,"[U] MEM[");imprimeRegistradores(x,0,output);fprintf(output,"--] = ");imprimeRegistradores(y,0,output);fprintf(output," = 0x%.8X\n",registradores[y]);
	
}
void pop(uint32_t hexa, FILE* output){
	uint32_t x,y;
	uint32_t overflowY,overflowX;
	
	
	x = get_X(hexa);
	y = get_Y(hexa);
	
	overflowX = get_OverX(hexa);
	overflowY = get_OverY(hexa);
	
	x = x | overflowX;
	y = y | overflowY;
	
	if(x == PC){
		printf("ENTROU POP\n");
		registradores[x] = i;
	}
	
	//addi
	registradores[y] = registradores[y] + 1;
	
	//ldw
	setarR0();
	registradores[x] = memoria[(registradores[y] + 0)];
	
	fprintf(output,"pop ");imprimeRegistradores(x,1,output);fprintf(output,", ");imprimeRegistradores(y,1,output);
	fprintf(output,"\n[U] ");imprimeRegistradores(x,0,output);fprintf(output," = MEM[++");imprimeRegistradores(y,0,output);fprintf(output,"] = 0x%.8X\n",registradores[x]);

}
int main(int argc, char* argv[]) {

	FILE* input;
	FILE* output;

	input = fopen("input.hex","r");
	output = fopen("output.txt","w");
/*	printf("#ARGS = %i\n", argc);
	printf("PROGRAMA = %s\n", argv[0]);
	printf("ARG1 = %s, ARG2 = %s\n", argv[1], argv[2]);
	// Abrindo arquivos
	FILE* input = fopen(argv[1], "r");
	FILE* output = fopen(argv[2], "w");;
*/
	uint32_t hexa;
//	uint32_t code;
	unsigned int cont = 0;

	while((fscanf(input,"%x",&hexa))!=EOF) //contar quantas linhas
		cont++;

	memoria = (uint32_t*)malloc(sizeof(uint32_t)*cont-1); //alocar memoria do tamanho do vetor

	rewind(input); //ler o arquivo de novo

	for(i=0;i<cont;i++){
		fscanf(input,"%x",&memoria[i]);
	}


	fprintf(output,"[START OF SIMULATION]\n");
	i = 0;

	while(1){

		registradores[IR] = memoria[i];
		opcode = memoria[i] >> 26;
		setarR0();
		
		switch(opcode){
			case 0x00: //000000
				if(memoria[i] == 0){
					printf("nop\n");
				}
				else{
					add(memoria[i],output);
				}
			break;
			case 0x01: //000001
				addi(memoria[i],output);
			break;
			case 0x02: //000010
				sub(memoria[i],output);
			break;
			case 0x03: //000011
				subi(memoria[i],output);
			break;
			case 0x04: //000100
				mul(memoria[i],output);
			break;
			case 0x05: //000101
				muli(memoria[i],output);
			break;
			case 0x06: //000110
				div_(memoria[i],output);
			break;
			case 0x07: //000111
				divi(memoria[i],output);
			break;
			case 0x08: //001000
				cmp(memoria[i],output);
			break;
			case 0x09: //001001
				cmpi(memoria[i],output);
			break;
			case 0x0A: //001010
				shl(memoria[i],output);
			break;
			case 0x0B: //001011
				shr(memoria[i],output);
			break;
			case 0x0C: //001100
				and(memoria[i],output);
			break;
			case 0x0D: //001101
				andi(memoria[i],output);
			break;
			case 0x0E: //001110
				not(memoria[i],output);
			break;
			case 0x0F: //001111
				noti(memoria[i],output);
			break;
			case 0x10: //010000
				or(memoria[i],output);
			break;
			case 0x11: //010001
				ori(memoria[i],output);
			break;
			case 0x12: //010010
				xor(memoria[i],output);
			break;
			case 0x13: //010011
				xori(memoria[i],output);
			break;
			case 0x14: //010100
				ldw(memoria[i],output);
			break;
			case 0x15: //010101
				ldb(memoria[i],output);
			break;
			case 0x16: //010110
				stw(memoria[i],output);
			break;
			case 0x17: //010111
				stb(memoria[i],output);
			break;
			case 0x1A: //011010
				bun(memoria[i],output);
			break;
			case 0x1B: //011011
				beq(memoria[i],output);
			break;
			case 0x1C: //011100
				blt(memoria[i],output);
			break;
			case 0x1D: //011101
				bgt(memoria[i],output);
			break;
			case 0x1E: //011110
				bne(memoria[i],output);
			break;
			case 0x1F: //011111
				ble(memoria[i],output);
			break;
			case 0x20: //100000
				bge(memoria[i],output);
			break;
			case 0x3F: //111111
				in_t(memoria[i],output);
				return 0;
			break;
			case 0x18: //100101
				push(memoria[i],output);
			break;
			case 0x19: //100101
				pop(memoria[i],output);
			break;
			case 0x25: //100101
				call(memoria[i],output);
			break;
			case 0x26: //100110
				ret(memoria[i],output);
			break;

			default:
			printf("Codigo invalido!\n");
			break;
		}
	//	printf("I = %d PC = %X\n",i, registradores[PC]);
		i++;
		registradores[PC] = i*4;

	}

	return 0;
}
