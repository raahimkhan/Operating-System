#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

const unsigned long PAGE_SIZE = 1024; // 2^10

const unsigned long MEM_SIZE = 131072; // 128 KB
const unsigned long L1_PAGE_ENTRIES = 64; // 2^6
const unsigned long L2_PAGE_ENTRIES = 256; // 2^8

const int TOTAL_FRAMES = 128;
const int L1_FRAMES = 1;
const int L2_FRAMES = 32;
const int REM_FRAMES = 95; 

const unsigned int COUNTERBITS = 3 << 18;
const unsigned int REFBIT = 1 << 19;
const unsigned int MODBIT = 1 << 18;
const unsigned int DIRTYBIT = 1 << 17;
const unsigned int VALIDBIT = 1 << 16;

int evict(unsigned int* table, int end) {
	while(true) {
		for(int i = 0; i < end; i++) {
			if(table[i] != 0) {
				unsigned int bits = table[i] & COUNTERBITS;
				if(bits == 0) {
					table[i] = table[i] & ~(1 << 16);
					return (table[i] << 16) >> 16;
				} else {
					if(bits == COUNTERBITS) {
						bits = REFBIT;
					} else if (bits == REFBIT) {
						bits = MODBIT;
					} else if (bits == MODBIT) {
						bits = 0;
					}
					table[i] = table[i] & ~(1 << 18);
					table[i] = table[i] & ~(1 << 19);
					table[i] = table[i] | bits; 
				}
			}
		} 
	}
}

void printMV(bool hit1, bool hit2, unsigned int before, unsigned int after, unsigned char op, unsigned int one, unsigned int val) {
	printf("MV\t");
	if(hit1 == true) {
		printf("0\t");
	} else {
		printf("1\t");
	}
	if(hit2 == true) {
		printf("0\t");
	} else {
		printf("1\t");
	}
	printf("null\tnull\t%x\t%x\t%x, %x, %x\n", before, after, op, one, val);
}

void printMM(bool hit1, bool hit2, bool hit3, bool hit4, unsigned int before, unsigned int after, unsigned char op, unsigned int one, unsigned int two) {
	printf("MM\t");
	if(hit1 == true) {
		printf("0\t");
	} else {
		printf("1\t");
	}
	if(hit2 == true) {
		printf("0\t");
	} else {
		printf("1\t");
	}
	if(hit3 == true) {
		printf("0\t");
	} else {
		printf("1\t");
	}
	if(hit4 == true) {
		printf("0\t");
	} else {
		printf("1\t");
	}
	printf("%x\t%x\t%x, %x, %x\n", before, after, op, one, two);
}

int main() {
    unsigned char** PHYSICAL_MEMORY = (unsigned char **) malloc(TOTAL_FRAMES * PAGE_SIZE);
    for(int i = 0; i < TOTAL_FRAMES; i++) {
    	PHYSICAL_MEMORY[i] = NULL;
    }
    unsigned int* L1_PAGE_TABLE = (unsigned int *) malloc(L1_PAGE_ENTRIES * sizeof(unsigned int));
    for(int i = 0; i < L1_PAGE_ENTRIES; i++) {
    	L1_PAGE_TABLE[i] = 0;
    }
    unsigned int** L2_PAGE_TABLES = (unsigned int **) malloc(L1_PAGE_ENTRIES * sizeof(unsigned int*));
    for(int i = 0; i < L1_PAGE_ENTRIES; i++) {
    	L2_PAGE_TABLES[i] = (unsigned int *) malloc(L2_PAGE_ENTRIES * sizeof(unsigned int));
    	for(int j = 0; j < L2_PAGE_ENTRIES; j++) {
    		L2_PAGE_TABLES[i][j] = 0;
    	}
    }
    int l2counter = 0; // max can be 32
    int pageCounter = 0; // max can be 95

	FILE *fptr = fopen("BACKING_STORE_2.bin", "rb");
	unsigned long currentInstruction = 0x00C17C00;

	bool first = true;

	while (currentInstruction < 0x00C193E8) {
		// read 8 byte instruction
		unsigned char* instruction = (unsigned char *) malloc(8 * sizeof(unsigned char));
	    fseek(fptr, currentInstruction, 0); 
	    fread(instruction, 8 * sizeof(unsigned char), 1, fptr); 

	    // opcode + one + two + value
	    unsigned char opcode = instruction[0];
	    unsigned char* addressOne = (unsigned char *) malloc(3 * sizeof(unsigned char));
	    for(int i = 0; i < 3; i++) {
	    	addressOne[i] = instruction[1+i];
	    }
	    unsigned int one = addressOne[0] << 16 | addressOne[1] << 8 | addressOne[2];
	    unsigned char* addressTwo = (unsigned char *) malloc(3 * sizeof(unsigned char));
	    for(int i = 0; i < 3; i++) {
	    	addressTwo[i] = instruction[4+i];
	    }
	    unsigned int two = addressTwo[0] << 16 | addressTwo[1] << 8 | addressTwo[2];
	    unsigned char* value = (unsigned char *) malloc(4 * sizeof(unsigned char));
	    for(int i = 0; i < 4; i++) {
	    	value[i] = instruction[4+i];
	    }
	    unsigned int val = value[0] << 24 | value[1] << 16 | value[2] << 8 | value[3];

  		// one_l1offset + one_l2offset + one_pageoffset + two_l1offset + two_l2offset + two_pageoffset
  		unsigned short one_l1offset = (one << 8) >> 26;
  		unsigned short one_l2offset = (one << 14) >> 24;
  		unsigned short one_pageoffset = (one << 22) >> 22;
  		unsigned short two_l1offset = (two << 8) >> 26;
  		unsigned short two_l2offset = (two << 14) >> 24;
  		unsigned short two_pageoffset = (two << 22) >> 22;

  		// L1 Table
  		bool L1hit = false;
  		if((L1_PAGE_TABLE[one_l1offset] & VALIDBIT) == 0) {
  			int emptyL2Frame = -1;
    		if(l2counter < 32) {
	    		for(int i = L1_FRAMES; i <= L2_FRAMES; i++) {
	    			if(PHYSICAL_MEMORY[i] == NULL) {
	    				emptyL2Frame = i;
	    				break;
	    			}
	    		}
	    		l2counter++;
    		} else {
    			emptyL2Frame = evict(L1_PAGE_TABLE, L1_PAGE_ENTRIES);
    		}
    		unsigned char* temp;
    		PHYSICAL_MEMORY[emptyL2Frame] = temp;
    		L1_PAGE_TABLE[one_l1offset] = 0 | VALIDBIT | emptyL2Frame;
  		} else {
  			L1hit = true;
  			L1_PAGE_TABLE[one_l1offset] = L1_PAGE_TABLE[one_l1offset] | REFBIT;
  		}

  		// L2 Table
  		bool L2hit = false;
  		if((L2_PAGE_TABLES[one_l1offset][one_l2offset] & VALIDBIT) == 0) {
  			int emptyFrame = -1;
	    	if (pageCounter < 95) {
	    		for(int i = 33; i < TOTAL_FRAMES; i++) {
	    			if(PHYSICAL_MEMORY[i] == NULL) {
	    				emptyFrame = i;
	    				break;
	    			}
	    		}
	    		pageCounter++;
	    	} else {
	    		emptyFrame = evict(L2_PAGE_TABLES[one_l1offset], L2_PAGE_ENTRIES);
	    		if((L2_PAGE_TABLES[one_l1offset][emptyFrame] & DIRTYBIT) != 0) {
                    fseek(fptr, (one >> 10) << 10, SEEK_SET);
                    fwrite(PHYSICAL_MEMORY[emptyFrame], PAGE_SIZE * sizeof(unsigned char), 1, fptr);
    			}
	    	}
	    	unsigned char* temppage = (unsigned char *) malloc(PAGE_SIZE * sizeof(unsigned char));
  			fseek(fptr, (one >> 10) << 10, 0); 
	    	fread(temppage, PAGE_SIZE * sizeof(unsigned char), 1, fptr); 
	    	PHYSICAL_MEMORY[emptyFrame] = temppage;
    		L2_PAGE_TABLES[one_l1offset][one_l2offset] = 0 | VALIDBIT | emptyFrame;
  		} else {
  			L2hit = true;
  			L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | REFBIT;
  		}

  		if (first) {
  			first = false;
  			printf("Type\tL1miss1\tL2miss1\tL1miss2\tL2miss2\tBefore\tAfter\tInstruction\n");
  		}

  		unsigned int before = 0;
  		if(opcode == 0x10) {
  			before = PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset];
  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] += value[3];
  			L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
  			printMV(L1hit, L2hit, before, PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset], opcode, one, value[3]);
  		} else if (opcode == 0x20) {
  			before = PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset];
  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] = value[3];
  			L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
  			printMV(L1hit, L2hit, before, PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset], opcode, one, value[3]);
  		} else if (opcode == 0x30) {
  			before = PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset];
  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] -= value[3];
  			L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
  			printMV(L1hit, L2hit, before, PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset], opcode, one, value[3]);
  		} else if (opcode == 0x40) {
  			before = PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset];
  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] *= value[3];
  			L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
  			printMV(L1hit, L2hit, before, PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset], opcode, one, value[3]);
  		} else if (opcode == 0x50) {
  			before = PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset];
  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] &= value[3];
  			L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
  			printMV(L1hit, L2hit, before, PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset], opcode, one, value[3]);
  		} else if (opcode == 0x60) {
  			before = PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset];
  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] |= value[3];
  			L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
  			printMV(L1hit, L2hit, before, PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset], opcode, one, value[3]);
  		} else if (opcode == 0x70) {
  			before = PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset];
  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] ^= value[3];
  			L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
  			printMV(L1hit, L2hit, before, PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset], opcode, one, value[3]);
  		} else {
  			before = PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset];

  			// L1 Table
	  		bool L1hit2 = false;
	  		if((L1_PAGE_TABLE[two_l1offset] & VALIDBIT) == 0) {
	  			int emptyL2Frame = -1;
	    		if(l2counter < 32) {
		    		for(int i = L1_FRAMES; i <= L2_FRAMES; i++) {
		    			if(PHYSICAL_MEMORY[i] == NULL) {
		    				emptyL2Frame = i;
		    				break;
		    			}
		    		}
		    		l2counter++;
	    		} else {
	    			emptyL2Frame = evict(L1_PAGE_TABLE, L1_PAGE_ENTRIES);
	    		}
	    		unsigned char* temp;
	    		PHYSICAL_MEMORY[emptyL2Frame] = temp;
	    		L1_PAGE_TABLE[two_l1offset] = 0 | VALIDBIT | emptyL2Frame;
	  		} else {
	  			L1hit2 = true;
	  			L1_PAGE_TABLE[two_l1offset] = L1_PAGE_TABLE[two_l1offset] | REFBIT;
	  		}

	  		// L2 Table
	  		bool L2hit2 = false;
	  		if((L2_PAGE_TABLES[two_l1offset][two_l2offset] & VALIDBIT) == 0) {
	  			int emptyFrame = -1;
		    	if (pageCounter < 95) {
		    		for(int i = 33; i < TOTAL_FRAMES; i++) {
		    			if(PHYSICAL_MEMORY[i] == NULL) {
		    				emptyFrame = i;
		    				break;
		    			}
		    		}
		    		pageCounter++;
		    	} else {
		    		emptyFrame = evict(L2_PAGE_TABLES[two_l1offset], L2_PAGE_ENTRIES);
		    		if((L2_PAGE_TABLES[two_l1offset][emptyFrame] & DIRTYBIT) != 0) {
	                    fseek(fptr, (two >> 10) << 10, SEEK_SET);
	                    fwrite(PHYSICAL_MEMORY[emptyFrame], PAGE_SIZE * sizeof(unsigned char), 1, fptr);
	    			}
		    	}
		    	unsigned char* temppage = (unsigned char *) malloc(PAGE_SIZE * sizeof(unsigned char));
	  			fseek(fptr, (two >> 10) << 10, 0); 
		    	fread(temppage, PAGE_SIZE * sizeof(unsigned char), 1, fptr); 
		    	PHYSICAL_MEMORY[emptyFrame] = temppage;
	    		L2_PAGE_TABLES[two_l1offset][two_l2offset] = 0 | VALIDBIT | emptyFrame;
	  		} else {
	  			L2hit2 = true;
	  			L2_PAGE_TABLES[two_l1offset][two_l2offset] = L2_PAGE_TABLES[two_l1offset][two_l2offset] | REFBIT;
	  		}

	  		unsigned char twoval = PHYSICAL_MEMORY[(L2_PAGE_TABLES[two_l1offset][two_l2offset]) & 0xff][two_pageoffset];

	  		if(opcode == 0x11) {
	  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] += twoval;
  				L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
	  		} else if (opcode == 0x21) {
	  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] = twoval;
  				L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
	  		} else if (opcode == 0x31) {
	  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] -= twoval;
  				L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
	  		} else if (opcode == 0x41) {
	  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] *= twoval;
  				L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
	  		} else if (opcode == 0x51) {
	  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] &= twoval;
  				L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
	  		} else if (opcode == 0x61) {
	  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] |= twoval;
  				L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
	  		} else if (opcode == 0x71) {
	  			PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset] ^= twoval;
  				L2_PAGE_TABLES[one_l1offset][one_l2offset] = L2_PAGE_TABLES[one_l1offset][one_l2offset] | DIRTYBIT | MODBIT;
	  		}

  			printMM(L1hit, L2hit, L1hit2, L2hit2, before, PHYSICAL_MEMORY[(L2_PAGE_TABLES[one_l1offset][one_l2offset]) & 0xff][one_pageoffset], opcode, one, two);
  		}

    	currentInstruction = currentInstruction + 0x8;
	}
	
	for(int i = 0; i < L1_PAGE_ENTRIES; i++) {
    	for(int j = 0; j < L2_PAGE_ENTRIES; j++) {
    		if((((L2_PAGE_TABLES[i][j]) & MODBIT) != 0) && ((L2_PAGE_TABLES[i][j] & VALIDBIT) != 0)) {
    			fseek(fptr, (i << 18 | j << 10), SEEK_SET);
	            fwrite(PHYSICAL_MEMORY[L2_PAGE_TABLES[i][j] & 0xff], PAGE_SIZE * sizeof(unsigned char), 1, fptr);
    		}
    	}
    }
}
