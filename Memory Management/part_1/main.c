#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include<math.h>

// have divided whole part 1 in small functions so debugging becomes easy for me

// this equals to total inputs in addresses.txt file
int logicalAddresses = 100000 ;

// output to be displayed will be stored in these
unsigned short* logical ; // logical addresses
char* read_write ; // read and write operations
unsigned char* value ; // Value
int* faults ; // page faults
unsigned short* myTable ; // page table
unsigned char** mem ; // memory
int myGlobal ;

int flag ;
unsigned short dirtyRemovedNumber ;
int dirtyEncountered ;
unsigned short removedPage ;
unsigned short removedFrame ;

bool checkFileExists(char* name) {

    FILE* myFile;
	myFile = fopen(name, "r") ;

	if(myFile == NULL)
	{
		printf("File cannot open or file does not exist!\n") ;
        return false ;
	}

    return true ;
}

void reInitialize() {

    flag = 0 ;
    dirtyRemovedNumber = 0 ;
    dirtyEncountered = 0 ;
    removedPage = 0 ;
    removedFrame = 0 ;
}

void allocateMemory(char* label, int tempSize) {

    int mallocSize = logicalAddresses ;

    if (strcmp(label, "logical") == 0)
        logical = (unsigned short*) malloc(mallocSize * sizeof(unsigned short)) ;

    else if (strcmp(label, "operation") == 0)
        read_write = (char* ) malloc(mallocSize * sizeof(char)) ;

    else if (strcmp(label, "value") == 0)
        value = (unsigned char*) malloc(mallocSize * sizeof(unsigned char)) ;

    else if (strcmp(label, "pagefaults") == 0)
        faults = (int* ) malloc(mallocSize * sizeof(int)) ;

    else if (strcmp(label, "table") == 0)
        myTable = (unsigned short*) malloc(tempSize * sizeof(unsigned short)) ;

    else if (strcmp(label, "memory") == 0)
        mem = (unsigned char**) malloc(tempSize * sizeof(unsigned char*)) ;
}

unsigned short assignData(char* label, int count, int number) {

    unsigned short valuee ;

    if (strcmp(label, "page") == 0)
        valuee = ((logical[count]) & (0xFF << 8)) >> 8 ;

    else if (strcmp(label, "offset") == 0)
        valuee = logical[count] & 0xFF ;

    else if (strcmp(label, "bit") == 0)
        valuee = (myTable[number] & (1 << 8)) >> 8 ;

    return valuee ;
}

int Updater(int signal, int val) {

    if (signal == 1)
    {
        int newVal = val + 1 ;
        return newVal ;
    }
}

char* Updater2 (int signal, int pos, int l, char* arr2, char* myarray) {

    if (signal == 1)
    {
        unsigned short updated = strtoul(arr2, NULL, 4 * 4) ;
        logical[pos] = updated ;
        char* arr1 = malloc (sizeof (char) * l) ;
        return arr1 ;
    }

    else if (signal == 2) 
    {
        read_write[pos] = myarray[1] ;
		faults[pos] = 0 ;
        char* tempp = malloc (sizeof (char) * l) ;
        return tempp ;
    }
}

void initializeData(char* name) {

    FILE* myFile ;
	myFile = fopen(name, "r") ;
	int cycle = 0 ;

    for (; ;) {

        int length = 7 ;
        char storage[length] ;

		if((fgets(storage, length, myFile)) == NULL)
            break ;

        int length2 = length - 3 ;
		char storage2[length2] ;

		int tracker = 2 ;
        int i = 0 ;

		while (i < length2)
		{	
			storage2[i] = storage[tracker] ;
			tracker = Updater(1, tracker) ;
            i = Updater(1, i) ;
		}

        char* arr1 = Updater2 (1, cycle, length, storage2, NULL) ;

		if((fgets(arr1, length, myFile)) == NULL)
			break ;

        char* temp = Updater2 (2, cycle, length, storage2, arr1) ;

		cycle += 1 ;
    }
}

int powerReturner(int val) {

    return (1 << val) ;
}

void writeStore(char* op, unsigned short removedPage, unsigned short removedFrame, 
    unsigned short number_of_page, int countFrames, int pos) {

    if (strcmp(op, "r_b") == 0) {

        FILE * store ;
        store = fopen("BACKING_STORE_1.bin", "r+b") ;
        fseek(store, (256 * removedPage), 0) ;
        fwrite(mem[removedFrame], sizeof(unsigned char) * 256, 1, store) ;
        fclose(store) ;
    }

    else if (strcmp(op, "rb") == 0) { 

        FILE * store ;
        store = fopen("BACKING_STORE_1.bin", "rb") ;
        fseek(store, (256 * number_of_page), 0) ;
        fread(mem[removedFrame], sizeof(unsigned char) * 256, 1, store) ;
        fclose(store) ;
    }

    else if (strcmp(op, "rb3") == 0) {

        FILE * store ;
        store = fopen("BACKING_STORE_1.bin", "rb") ;
        fseek(store, (256 * number_of_page), 0) ;
        fread(mem[countFrames], sizeof(unsigned char) * 256, 1, store) ;
        fclose(store) ;
    }

    else if (strcmp(op, "final") == 0) {

        FILE * store ;
        store = fopen("BACKING_STORE_1.bin", "r+b") ;
        fseek(store, (256 * pos), 0) ;
        fwrite(mem[(myTable[pos] & 0xFF)], sizeof(unsigned char) * 256, 1, store) ;
        fclose(store) ;
    }
}

int updateTable(int signal) {

    if (signal == 1) {

        int p1 = 0x3FF ;
        int p2 = myTable[myGlobal] ;
        int p3 = p2 & p1 ;
        myTable[myGlobal] = p3 ;
        return p3 ;
    }

    else if (signal == 2) {
        
        int p1 = 0x3FF ;
        int p2 = myTable[myGlobal] ;
        int p3 = p2 & p1 ;
        int p4 = (1 << 10) ;
        int p5 = p3 | p4 ;
        myTable[myGlobal] = p5 ;
        return p5 ;
    }

    else if (signal == 3) {
        
        int p1 = 0xBFF ;
        int p2 = myTable[myGlobal] ;
        int p3 = p2 & p1 ;
        myTable[myGlobal] = p3 ;
        return p3 ;
    }

    else if (signal == 4) {

        int p1 = (0xF << 8) ;
        int p2 = myTable[myGlobal] ;
        int p3 = p2 & p1 ;
        int p4 = p3 >> 10 ;
        return p4 ;
    }
}

unsigned short evict(int pages, unsigned short number_of_page, unsigned short offset, unsigned short f, int i) {

    reInitialize() ;
    
    for (; ;)
    {
        myGlobal = 0 ;
        while (myGlobal < pages)
        {

            bool truth = ((myTable[myGlobal] & (1 << 8)) >> 8) == 0 ; // checking validity of bit
            if(truth == true) {

                myGlobal += 1 ;
                continue ;
            }

            int myExtracted = updateTable(4) ;
            bool mytruth1 = myExtracted == 1 ;
            bool mytruth2 = myExtracted == 2 ;
            bool mytruth3 = myExtracted == 3 ;
                
            if(((myTable[myGlobal] & (0xF << 8)) >> 10) == 0)
            {	
                // found evicted
                
                bool truth2 = ((myTable[removedPage] & 0x2FF) >> 9) == 1 ; // dirty bit
                if(truth2 == true)
                {
                    dirtyEncountered = 1 ;
                    dirtyRemovedNumber = myGlobal ;
                }

                else
                {
                    flag = 1 ;
                    removedPage = myGlobal ;
                    unsigned short extracted = myTable[myGlobal] ;
                    extracted = extracted & 0xFF ;
                    removedFrame = extracted ;
                    myGlobal = Updater(1, myGlobal) ;
                    break ;
                }
                
            }

            else {

                if(mytruth1 == true)
                {
                    int tempppp = updateTable(1) ;
                }

                else if(mytruth2 == true)
                {
                    int tempppp = updateTable(2) ;
                }

                else if(mytruth3 == true)
                {
                    int tempppp = updateTable(3) ;
                }
            }

            myGlobal += 1 ;
        }

        if(flag == 1)
            break ;
        

        else if(dirtyEncountered ==  1)
        {
            removedPage = dirtyRemovedNumber ;
            removedFrame = myTable[removedPage] & 0xFF ;
            break ;
        }
    }


    bool truth3 = ((myTable[removedPage] & 0x2FF) >> 9) == 1 ; // dirty bit
    if(truth3 == true)
    {
        // write back to backing store

        writeStore("r_b", removedPage, removedFrame, 0, 0, 0) ;
    }

    myTable[removedPage] = myTable[removedPage] & 0 ;
    myTable[number_of_page] = myTable[number_of_page] | removedFrame ;

    writeStore("rb", removedPage, removedFrame, number_of_page, 0, 0) ;

    value[i] = mem[removedFrame][offset] ;

    bool finalTruth = read_write[i] == '1' ;
    if(finalTruth == true)
    {

        myTable[number_of_page] = myTable[number_of_page] | (1 << 9) ;
        value[i] = value[i] >> 1 ;
        mem[removedFrame][offset] = value[i] ;
    }

    f = removedFrame ;

    return f ;

}

void turnBit(char* op, int pos, unsigned short number_of_page, unsigned short offset, int countFrames) {

    if (strcmp(op, "dirty") == 0) {
        
        myTable[number_of_page] = myTable[number_of_page] | (1 << 9) ;
        value[pos] = value[pos] >> 1 ;
        mem[countFrames][offset] = value[pos] ;
    }

    else if (strcmp(op, "valid") == 0) {
        
        myTable[number_of_page] = myTable[number_of_page] | (1 << 8) ;
        faults[pos]++ ;
    }

    else if (strcmp(op, "dirty2") == 0) {

        myTable[number_of_page] = myTable[number_of_page] | (1<<9) ;
        value[pos] = value[pos] >> 1 ;
        mem[(myTable[number_of_page] & 0xFF)][offset] = value[pos] ;
    }
}

void printer1(int val, unsigned short number_of_page, unsigned short offset) {

    printf("   ") ;

    bool truth1 = number_of_page <= 15 ;
    bool truth2 = offset <= 15 ;
    bool truth3 = truth1 && truth2 ;
    if (truth3 == true)
        printf("0x0%hx0%hx\t\t", number_of_page, offset) ;

    else if (truth1 == true)
        printf("0x0%hx%hx\t\t", number_of_page, offset);

    else if (truth2 == true)
        printf("0x%hx0%hx\t\t", number_of_page, offset);
    
    else
        printf("0x%hx\t\t", logical[val]);
}

void printer2(unsigned short offset, unsigned short f) {

    bool truth1 = f <= 15 ;
    bool truth2 = offset <= 15 ;
    bool truth3 = truth2 && truth1 ;
    if (truth3 == true)
        printf("0x0%hx0%hx\t", f, offset);

    else if (truth1 == true)
        printf("0x0%hx%hx\t", f, offset);

    else if (truth2 == true)
        printf("0x%hx0%hx\t", f, offset);
    
    else
        printf("0x%hx%hx\t", f, offset);
}

void printer3(int val) {

    bool truth1 = read_write[val] == '0' ;
    bool truth2 = faults[val] == 1 ;

    switch (truth1) {

        case true: 
            printf("\t   Read\t     ");
            break;
        default:
            printf("\t   Write     ");
            break ;
    }

    printf("\t 0x%hhx\t", value[val]);

    switch (truth2) {

        case true: 
            printf("    Yes\n");
            break;
        default:
            printf("    No\n");
            break ;
    }
}

int main(int argc, char** argv)
{
    // getting name of file
    const char *myarg = argv[1] ;

    // checking if file exists or name provided is valid or not
    bool exists = checkFileExists(argv[1]) ;
    if (exists == false)
        return 0 ;

    // allocating global array memory
    allocateMemory("logical", 0) ;
    allocateMemory("operation", 0) ;
    allocateMemory("value", 0) ;
    allocateMemory("pagefaults", 0) ;

    // initializing data now
    initializeData(argv[1]) ;

	int pages = powerReturner(8) ; // 2 ^ 8

    // allocating and initializing page table
    allocateMemory("table", pages) ;
    int pageIterator = 0 ;
	while (pageIterator < pages)
	{
		myTable[pageIterator] = 0 ;
        pageIterator += 1 ;
	}

	unsigned short howManyFrames = powerReturner(6) ; // 2 ^ 6
	int countFrames = 0 ;

    // allocating and initializing memory
    allocateMemory("memory", howManyFrames) ;
    int memoryIterator = 0 ;
    int memoryIterator2 = 0 ;
	while (memoryIterator < howManyFrames)
	{
		mem[memoryIterator] = (unsigned char*) malloc(256 * sizeof(unsigned char)) ;
		while (memoryIterator2 < 256)
		{
			mem[memoryIterator][memoryIterator2] = 0 ;
            memoryIterator2 += 1 ;
		}

        memoryIterator += 1 ;
	}
	
    printf("Logical Address    Physical Address      Read\\Write     Value     Page Fault    \n") ;

	int faultsCounter = 0 ; // page faults

    int i = 0 ;
    while (i < logicalAddresses)
	{
		unsigned short number_of_page = assignData("page", i, 0) ;
		unsigned short offset =   assignData("offset", i, 0) ;
		unsigned short f ;

        bool truth = (assignData("bit", i, number_of_page)) == 0 ; // checking if bit is valid or not

		if(truth == true)
		{
			
			if(countFrames >= 64)
			{
                f = evict(pages, number_of_page, offset, f, i) ;
			}

			else
			{
				// Allocate frame number

				myTable[number_of_page] = myTable[number_of_page] | countFrames ;

                writeStore("rb3", 0, 0, number_of_page, countFrames, 0) ;

				value[i] = mem[countFrames][offset] ;

                bool truthh = read_write[i] == '1' ;

				if(truthh == true)
				{

                    turnBit("dirty", i, number_of_page, offset, countFrames) ;
				}

				f = countFrames ;
				countFrames++ ;
				
			}

            turnBit("valid", i, number_of_page, offset, countFrames) ;
			faultsCounter++ ;
		}

		else
		{
			value[i] = mem[(myTable[number_of_page] & 0xFF)][offset] ;

            bool truthhh = read_write[i] == '1' ; 

			if(truthhh == true)
			{

                turnBit("dirty2", i, number_of_page, offset, countFrames) ;
			}	

			if(((myTable[number_of_page] & (0xF << 8)) >> 10) < 3)
			{
				myTable[number_of_page] = myTable[number_of_page] + (1 << 10) ;
			}

			f = (myTable[number_of_page] & 0xFF) ;
		}

		// Now printing

        printer1(i, number_of_page, offset) ;

        printer2(offset, f) ;

        printer3(i) ;

        i += 1 ;
	}

	// writing back to binary file if dirty bit is one

    int k = 0 ;
	while (k < pages)
	{
		if(((myTable[k] & (1 << 8)) >> 8) == 0)
        {
            k += 1 ;
            continue ;
        }

		if(((myTable[k] & 0x2FF) >> 9) == 1)
		{

            writeStore("final", 0, 0, 0, 0, k) ;
		}

		myTable[k] = 0 ;

        k += 1 ;
	}

    // statistics

	printf("\nTotal number of page faults: %d\n",faultsCounter) ;
	printf("Total number of logical addresses: %d\n", logicalAddresses) ;
	printf("Percentage:%f\n", (faultsCounter * 100.0 / logicalAddresses)) ;

}