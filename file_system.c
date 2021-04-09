#include <stdio.h> //standard input output
#include <stdlib.h> //standard library: memory allocation, process control
#include <string.h> //for string comparisons
#include <unistd.h> //Unix standard, miscellaneous symbolic constants, types, functions
#include <sys/stat.h>  //POSIX library facilitates information about files attributes, st_XYZ members
#include <dirent.h> //contains constructs that facilitate directory traversing with POSIX
#include <sys/types.h> //contains basic derived types, used for sizes
#include <pwd.h> //provide a definition for struct passwd, pw_XYZ members
#include <grp.h> //declares the structure group, gr_XYZ members
#include <time.h> //functions for manipulating date and time, for strftime()


struct stat buffer;

void fileAttributes(const char *fileName){
    printf("\n\nFile Attributes of file %s\n", fileName);
    
    printf("(1) Mode:\t\t"); /*mode*/
    printf((buffer.st_mode & S_IRUSR) ? "r":"-"); /*read permission bit for the owner of the file*/
    printf((buffer.st_mode & S_IWUSR) ? "w":"-"); /*write permission bit for the owner of the file*/
    printf((buffer.st_mode & S_IXUSR) ? "x":"-"); /*execute or search permission bit for the owner of the file*/
    printf((buffer.st_mode & S_IRGRP) ? "r":"-"); /*read permission bit for the group owner of the file*/
    printf((buffer.st_mode & S_IWGRP) ? "w":"-");
    printf((buffer.st_mode & S_IXGRP) ? "x":"-");
    printf((buffer.st_mode & S_IROTH) ? "r":"-"); /*read permission bit for other users*/
    printf((buffer.st_mode & S_IWOTH) ? "w":"-");
    printf((buffer.st_mode & S_IXOTH) ? "x":"-");
    printf("\n");

    printf("(2) Number of Links:\t%d\n", buffer.st_nlink); /*number of links, count keeps track of how many directories have entries for this file*/

    struct passwd *ownerName = getpwuid(buffer.st_uid); /*owner's name, st_uid gives user ID of file owner*/
    struct group *groupName = getgrgid(buffer.st_gid); /*group name*/
    printf("(3) Owner's Name:\t%s\n", ownerName->pw_name); /*pwd member *pw_name gives user's login name*/
    printf("(4) Group Name:\t\t%s\n", groupName->gr_name); /*grp member *gr_name gives name of group */
    
    printf("(5) Size in Bytes:\t%lld\n", buffer.st_size); /*size of file in bytes*/
    printf("(6) Size in Blocks:\t%d\n", buffer.st_blksize); /*size of file in blocks*/
   
    char date[20];
    strftime(date, 20, "%x at %I:%M%p", localtime(&(buffer.st_mtime))); /*function to format date and time*/
    printf("(7) Last Modification:\t%s\n", date); /*last modification time*/
 
    printf("(8) File Name:\t\t%s\n", fileName); /*file name*/
    printf("\n");

}

void directoryTraverse(const char * input){
    struct dirent *content;
    DIR *directory = opendir(input); /*opens a directory stream*/
    
    if (directory==NULL){
        perror(input); //error with the directory handling
        return;
    }
    
    while ((content=readdir(directory)) != NULL){
        if(strcmp(content->d_name,".")!=0 && strcmp(content->d_name,"..")!=0){ /*stackoverflow.com/questions/42308988/segmentation-fault-checking-strcmp*/
            if(content->d_type == DT_DIR){ /*unsigned char d_type where DT_DIR is a directory*/
                chdir(input); /*chdir changes current directory of calling process to directory specified in path*/
                directoryTraverse(content->d_name);
                chdir("..");
            }
            else{
                printf("%s/%s\n",input,content->d_name);
            }
        }
        
    }
    closedir(directory);
    return;
}

int main (void){
    char input[128]; //use PATH_MAX if don't want to assume size of path
    printf("\nEnter Directory or File Name: ");
    scanf("%s", input);
    
    while(stat(input,&buffer) != 0){
        perror("Error");
        printf("\nEnter another Directory or File Name: ");
        scanf("%s", input);
    }
    
    if(S_ISDIR(buffer.st_mode)){ /* S_ISDIR is a POSIX macro, returns non-zero if file (input) is a directory*/
        directoryTraverse(input);
    }
    else{
        fileAttributes(input);
    }
    
    return 0;
}



/* UNIX Commands
 df: disk free, used to display the amount of available disk space for file systems on which the invoking user has appropriate read access
 ls: lists the names of files in a particular Unix directory
 ln: creates a hard link or a symbolic link (symlink) to an existing file or directory
 
inode: a data structure file system that describes a file-system object such as a file or a directory, each inode stores the attributes and disk block locations of the object's data
 */

/* assignment
 (1) reporting file attributes
 (2) traversing directories
 if the input argument to the program is a file, the program will print the file's attributes
 if the input argument to the program is a directory, the program will traverse the directory recursively
 */

/* Notes
 Block Size: unit of work for the file system, every read and write is done in full multiples of the block size; also the smallest size on disk a file can have; if you have a 16 byte Block size,then a file with 16 bytes size occupies a full block on disk.
 
 */

