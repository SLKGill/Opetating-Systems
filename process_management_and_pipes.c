# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/types.h>


int main(void){
    int fd1[2], fd2[2], nbytes; /*file descriptor to create pipe*/
    /*element[0]=opened for reading
    element[1]=opened for writing, output becomes input for element[0] */
    pid_t childpid;
    int readbuffer;
    int sum = 0;
    int input = 0;

    /*pipe lets one process send a stream of bytes to another, creating pipe before child process allows both processes access to pipe*/
    pipe(fd1);
    pipe(fd2);

    /*when fork() returns negative value, creation of child process was unsuccessful*/
    if((childpid = fork()) == -1){
        perror("fork");
        exit(0);
    }

    /*when fork() is 0 a new child process is made*/
    if(childpid == 0){
        close(fd1[0]); /*parent wants to send data to child, parent should close element[0]*/
        close(fd2[1]); /*and child should close element[1]*/
        while(1){
            printf("Enter a number: ");
            scanf("%d", &input); 
            
            /*sending data to parent*/
            write(fd1[1], &input, 1); /*write function=number of bytes written into array, can be less than nbytes, error if -1*/
            
            if (input == -1){ /*input is -1*/
                
                /*nbytes fuction returns total bytes consumed by the elements of the array,
                  reading up to sizeof(readbuffer) bytes*/
                nbytes = read(fd2[0], &readbuffer, sizeof(readbuffer)); //reading happens at element[0],
                printf("The sum is: %d", readbuffer-255); //output the sum
                break; //terminate
            }
        }
    }
    else{
        close(fd1[1]);/*if a parent wants to receive data from child, parent close element[1]*/
        close(fd2[0]);/*and child should close element[0]*/
        while(1){
            nbytes = read(fd1[0], &readbuffer, 1);
            sum = sum + readbuffer; //summing integers
            if (readbuffer == 255){ //255 corresponds to -1, overflow
                write(fd2[1], &sum, sizeof(sum)); //data for child, opened for writing
                break;
            }
        }
    }
    return 0;
}


/*program works with positive values from 0 to 254 due to 1-byte restriction*/
