#include <stdio.h> //standard input output
#include <stdlib.h> //standard library: memory allocation, process control
#include <sys/stat.h>  //POSIX library information about files attributes (fstat())
#include <sys/mman.h> //for memory mapped mmap() function
#include <fcntl.h> //file control options
#include <unistd.h> //for the close file feature to work


int main(void) {

    int resourceFile, resourceType, resourceNum; //inputs
    int stat, typeExists, go; //checks
    char *memMap, *newMap;
    struct stat buffer;
    size_t fileSize; //size_t size of objects in bytes

    resourceFile = open ("res.txt", O_RDWR); //O_RDWR flag opens for reading and writing.
    if (resourceFile==-1){
        printf("File Error\n");
        return 1;
    }
    
    /* reference: pubs.opengroup.org/onlinepubs/009696699/functions/fstat.html
    fstat gets file status, shall obtain information about an open file associated with the file descriptor resourceFile, and shall write it to the area pointed to by buff. fstat may fail if file size in bytes or the number of blocks allocated to the file or the file serial number cannot be represented correctly in the structure pointed to by buff or one of the values is too large to store in buff.
    */
    stat = fstat(resourceFile,&buffer);
    if(stat==-1){
        printf("Error with fstat()\n");
        exit(0);
    }

    fileSize = buffer.st_size;

    /* reference: linuxhint.com/using_mmap_function_linux/
        mmap used for mapping between a process address space and files, so file can ve accessed like an array in program
        mmapp(address=if null kernel can place mapping anywhere, length=number of bytes to map to, protect=control what kind of access is permitted, flags=control nature of map MAP_SHARED used to share the mapping with all other processes and changes made to mapping region will be written back to file, file descriptor, offset=where file mapping starts)
    */
    memMap = mmap(NULL, fileSize, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, resourceFile, 0);
    
    if (memMap==MAP_FAILED){
        printf("Error with memory map\n");
        return 1;
    }

    close(resourceFile);
    
    printf("\nOrginial Resource List:\n");
    for(int i=0; i<fileSize; i++){
        printf("%c",memMap[i]); //printing content of resourceFile from "array" from mmap()
    }
    printf("\n");
    

    while(1) {
        typeExists=0;
        printf("\nEnter any number to begin allocating new resources, or enter 0 if not: ");
        scanf("%d",&go);
        
        if(!go){
            return 1;
        }

        printf("Enter Resource Type (0,1,2): ");
        scanf("%d",&resourceType);
        printf("Enter Number of Reources (0-9): ");
        scanf("%d",&resourceNum);

        for (int i=0; i<=fileSize; i=i+4){
            if (resourceType==(memMap[i]-48)){
                typeExists=1;
                newMap = &memMap[i+2];
                if (resourceNum<=(int)*newMap-48){
                    *newMap -= resourceNum;
                    printf("\nUpdated Resource List:\n");
                    for(int i=0; i<fileSize; i++){
                        printf("%c",memMap[i]);
                    }
                    printf("\n");
                    
                     /*reference: pubs.opengroup.org/onlinepubs/009695399/functions/msync.html msync() function shall write all modified data to permanent storage locations, MS_SYNC performs synchronous writes*/
                    stat = msync(memMap, fileSize, MS_SYNC);
                    if (stat==-1){
                        printf("Error with syncing file");
                    }
                }
                else {
                    printf("Unsuccesful: Not enough resources to allocate\n");
                    break;
                }
            }
        }
        if (typeExists==0){
            printf("Unsuccesful: Resource Type does not exist\n");
        }
    }
 }

