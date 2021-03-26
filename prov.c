#include <stdio.h> //standard input output
#include <stdlib.h> //standard library: memory allocation, process control
#include <sys/stat.h>  //POSIX library information about files attributes (fstat())
#include <sys/mman.h> //for memory mapped mmap() function
#include <fcntl.h> //file control options
#include <unistd.h> //for the close file feature to work
#include <sys/types.h> //for the sem stuff
#include <sys/ipc.h>
#include <sys/sem.h>

key_t key = 5;
int mutex;
struct sembuf semWait[1];
struct sembuf semSigal[1];


/* open resourceFile and map to memory region, provider of resources*/
void parent(char *memMap, int fileSize) {

    char *newMap;
    int typeExists, resourceType, resourceNum;
    int stat, go;

    while(1) {
        typeExists = 0;
        printf("\nEnter any number to begin allocating new resources: ");
        //if 0 is entered will only terminate parent process
        scanf("%d",&go);
        
        if(!go){
            exit(0);
        }

        printf("Enter Resource Type (0,1,2): ");
        scanf("%d",&resourceType);
        printf("Enter Number of Reources (0-9): ");
        scanf("%d",&resourceNum);
// semWait() ?? implement
        for (int i=0; i<=fileSize; i=i+4){
            if (resourceType==(memMap[i]-48)){
                typeExists=1;
                newMap = &memMap[i+2];
                *newMap += resourceNum;
                /*printf("\nUpdated Resource List:\n");
                for(int i=0; i<fileSize; i++){
                    printf("%c",memMap[i]);
                }
                printf("\n");*/
                stat = msync(memMap, fileSize, MS_SYNC);
                if (stat==-1){
                    printf("Error with syncing file");
                }
            }
        }
// semSignal() ??implement
        if (typeExists==0){
        printf("Unsuccesful: Resource Type does not exist\n");
        }
    }
}


void child(char *memMap, int fileSize){

    while(1) {

        sleep(10);
        /*reference: lecture notes
         page: fixed length block of data that reised in secondary memory, page of data may be copied into frame of main memory */
        int pageSize = getpagesize(); //number of bytes in a memory page
        int length = (fileSize+pageSize-1)/pageSize;
        char vec[length];
        /* reference: man7.org/linux/man-pages/man2/mincore.2.html
         mincore() returns a vector that indicates whether pages of the calling process's virtual memory are resident
         core (RAM), and so will not cause a disk access (page fault) if referenced
         The vec argument must point to an array containing at least (length+PAGE_SIZE-1)/PAGE_SIZE bytes. On return, the least significant bit of each byte will be set if the corresponding page is currently resident in memory, and be clear otherwise.
         */
 //       semWait(); would have to define these functions?
        mincore(memMap,fileSize,vec); //determine whether pages are resident in memory

        printf("\n\nChild Process Report (10 seconds have elapsed):\n");
        printf("Page Size of System : %d", pageSize);
        printf("\nUpdated Resources List:\n");
        for(int i=0; i<fileSize; i++){
            printf("%c",memMap[i]);
        }
 //        semSignal();
        printf("\nCurrent Status of Pages in Memory Region: %d", vec[0]);
        printf("\n\n");
    }
}



int main(void) {

    int resourceFile, stat;
    char *memMap, *newMap;
    struct stat buffer;
    size_t fileSize;
    
    /*semaphore stuff */
    /* IPC_CREAT semaphore is to be created if DNE
      0666 is the usual access permision in linux in rwx octal format
     */
    mutex = semget(key, 1, 0666 | IPC_CREAT); //returns -1 on fail
    semctl(mutex, 0, SETVAL, 1); //function provides a variety of semaphore control operations as specified by cmd

    /* reference: pubs.opengroup.org/onlinepubs/009696699/functions/semop.html
     semop() function shall perform atomically a user-defined array of semaphore operations on the set of semaphores associated with the semaphore identifier specified by the argument semid.
     */
    /*sem_op = semaphore operation*/
    semSigal[0].sem_op = 1;
    semWait[0].sem_op = -1;
    
    /*sem_num = semaphore number*/
    semSigal[0].sem_num = 0;
    semWait[0].sem_num = 0;
    
    /*sem_flg = operation flags*/
    semSigal[0].sem_flg = 0;
    semWait[0].sem_flg = 0;

    resourceFile = open ("res.txt", O_RDWR);
    if (resourceFile==-1){
        printf("File Error\n");
        return 1;
    }
    stat = fstat(resourceFile,&buffer);

    fileSize = buffer.st_size;


    memMap = mmap(NULL, fileSize, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, resourceFile, 0);
    if (memMap==MAP_FAILED){
        printf("Error with memory map\n");
        return 1;
    }
    close(resourceFile);

    /* www.gnu.org/software/libc/manual/html_node/Process-Creation-Example.html */
    pid_t pid = fork();
    if(pid<0){
        printf("Error woth PID");
        
    }

    if (pid == 0){ //rhis is the child process
        child(memMap,fileSize);
    }
    else{ //when positive pid is the parent process
        parent(memMap,fileSize);
        
    }

}

