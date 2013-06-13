/*
echoc: a demo of TCP/IP sockets connect

usage:	client [-h serverhost] [-p port]
*/

// LIBRARIES
#include <ftw.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>	/* needed for os x*/
#include <string.h>	/* for strlen */
#include <netdb.h> 
#include <fcntl.h>     /* for gethostbyname() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/errno.h>   /* defines ERESTART, EINTR */
#include <sys/wait.h>    /* defines WNOHANG, for wait() */
#include <arpa/inet.h>
#include <errno.h>	/* for printing an internet address in a user-friendly way */
#include <regex.h>
#include <openssl/md5.h>
#include <time.h>
#include "port.h"  /* defines default port */
#define STDIN 0



// Walk function
int list(char *name, struct stat *status, int type);


#define sizeOfPacket 10240
// Blob array A that has all the files and stuff in it, followed by its size
typedef struct blob
{
    char modifytime[1000];
    char name[1000];
    char md5hash[100];
    int size;
    char type[100];
    char timestamp[100];
}blob;

blob bloba[10000];
int blobasize;

// Function to check if there has been a repition of file shared somewhere
int duplicate(blob *blobi, char *name);

// Function to remove a file from sharing
void delete(char *token,blob *blobi);

// Helper function to print the contents of blob Array
void showblob(int fd, int rqst);

// regex has in it a compiled regex
regex_t regex;

//Regexsearch function
blob* regexsearch(char* token,blob* blobi);

//struct to catch values that regex would return
blob regexreturn[1000];
int regexreturnsize=0;

//md5 hash relating stuff
char *file2md5(char *filename);

//Actually this gives the last modification time
char* getFileCreationTime(char *filePath);

//To get the file size
int getFileSize(char *filename);

//To get file's extension
char *get_filename_ext(char *filename);

//To get file's timstamp
char* getFileTimestamp(char *filePath);

//To update timestamps and hashvalues of modified files
void checkall(int fd, int rqst);

void shortlist(char *start,char *end, int fd, int rqst);

void getFunction(int, int);
//scanning the function
int SetSocketBlockingEnabled(int fd, int blocking);
void upload(int fd, int rqst, char* file, int response);

int conn(char *host, int port, int forward);	/* connect to host,port; return socket: Makes the client */
void disconn(int fd);	/* close a socket connection */
int debug = 0;

void prompt(int, int); //Prompt that shows up
void serverServe(int, char*); // Sets up the server
char* checkSum(char *fileName);

char *root,*level1,*level2,*level3; //global variables, used for scanning
char* hextab = "0123456789ABCDEF";
char type[256];
struct sockaddr_in universal;

int main(int argc, char **argv)
{
    //allotting memory
    root=(char*)malloc(sizeof(char)*1024);
    level1=(char*)malloc(sizeof(char)*100);
    level2=(char*)malloc(sizeof(char)*100);
    level3=(char*)malloc(sizeof(char)*100);

    extern char *optarg;
    char *temp;
    extern int optind;
    int c, err = 0; 

    int port = SERVICE_PORT;	/* default: whatever is in port.h */
    char *host = "localhost";	/* default: this host */
    int fd;
    strcpy(type, "t");				/* file descriptor for socket */
    static char usage[] = 
        "usage: %s [-d] [-h serverhost] [-p port] [-c connection]\n";

    while ((c = getopt(argc, argv, "dh:p:c:")) != -1)
        switch (c) {
            case 'h':  /* hostname */
                host = optarg;
                break;
            case 'p':  /* port number */
                port = atoi(optarg);
                if (port < 1024 || port > 65535) {
                    fprintf(stderr, "invalid port number: %s\n", optarg);
                    err = 1;
                }
                break;
            case 'c':
                temp = optarg;
                strcpy(type, temp);
                getCorrectValue();                                        
                break;
            case '?':
                err = 1;
                break;
        }
    if ((fd = conn(host, port,1)) < 0) //connect to the server
    {
        serverServe(SERVICE_PORT, host); //server does not exist, create the server
    }

    return 0;
}

void serverServe(int port, char *host)
{
    int svc;        /* listening socket providing service */
    int rqst;
    socklen_t alen;       /* length of address structure */
    struct sockaddr_in my_addr;    /* address of this service */
    struct sockaddr_in client_addr;  /* client's address */
    int sockoptval = 1;
    char hostname[128]; /* host name, for debugging */
    int fdCS;

    gethostname(hostname, 128);

    if(strcmp(type,"u")==0)
    {
        if ((svc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) { //TCP socket
            perror("cannot create socket");
            exit(1);
        }

    }
    else
    {

        if ((svc = socket(AF_INET, SOCK_STREAM, 0)) < 0) { //TCP socket
            perror("cannot create socket");
            exit(1);
        }

    }
    setsockopt(svc, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int)); //allows us to reuse the port

    memset((char*)&my_addr, 0, sizeof(my_addr));  /* 0 out the structure */
    my_addr.sin_family = AF_INET;   /* address family */
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int retVal = SetSocketBlockingEnabled(svc,1);

    if(retVal == 0)
    {
        printf("Non blocking could not be enabled\n");
    }

    /* bind to the address to which the service will be offered */
    if (bind(svc, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    if(strcmp(type,"t")==0)
    {
    /* set up the socket for listening with a queue length of 5 */
        if (listen(svc, 5) < 0) {
            perror("listen failed");
            exit(1);
        }
    }

    printf("server started on %s, listening on port %d\n", hostname, port);

    /* loop forever - wait for connection requests and perform the service */
    alen = sizeof(client_addr);     /* length of address */
    if(strcmp(type,"t")==0)
    {
        rqst = accept(svc, (struct sockaddr *)&client_addr, &alen);


        if(rqst > 0)
        {
        //Connecting back to the client at port RETURN_SERVICE_PORT);

            fdCS = conn(host, RETURN_SERVICE_PORT,0);
            int retVal = SetSocketBlockingEnabled(fdCS,0);
            if(retVal == 0)
            {
                printf("Non blocking could not be enabled\n");
            }
            retVal = SetSocketBlockingEnabled(rqst,0);
            if(retVal == 0)
            {
                printf("Non blocking could not be enabled\n");

            }
        }
    }
    else

    {
        int recv_len;

        while(recv_len < 1)
        {
            char buf[255];

            if ((recv_len = recvfrom(svc, buf, 255, 0, (struct sockaddr *) &client_addr, &alen)) == -1)
            {
                perror("recvfrom():");
            }
        }
    }
    universal = client_addr;




    printf("Connection set up from: %s port %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    

    prompt(rqst, fdCS);		

}

char* encode_int(int i) {
  char* c = (char*)malloc(sizeof(char) * 9);
  int j;
  for (j = 0; j < 4; j++) {
    c[(j << 1)] = hextab[((i % 256) >> 4)];
    c[(j << 1) + 1] = hextab[((i % 256) % 16)];

    i = (i >> 8);
  }
  c[8] = 0;

  return c;
}
int checkSumHelper(char* str) {
  int i;
  int chk = 0x12345678;

  for (i = 0; str[i] != '\0'; i++) {
    chk += ((int)(str[i]) * (i + 1));
  }

  return chk;
}

char* checkSum(char *fileName)
{
    long f_size;
    char* code;
    size_t code_s, result;
    FILE* fp = fopen(fileName, "r");
    fseek(fp, 0, SEEK_END);
    f_size = ftell(fp); /* This returns 29696, but file is 85 bytes */
    fseek(fp, 0, SEEK_SET);
    code_s = sizeof(char) * f_size;
    code = malloc(code_s);
    result = fread(code, 1, f_size, fp);
    printf("%s\n",encode_int(checkSumHelper(code)));
    return encode_int(checkSumHelper(code));
    fclose(fp);

}


void parse(FILE* file, int rqst, int fd)
{
    char rootT[100];
    char level1T[100];
    char level2T[100];
    char level3T[100];
    int i, off;
    fscanf(file,"%s",rootT);
    if(strcmp(rootT,"IndexGet")==0)
    {
        fscanf(file,"%s",level1T);
        if(strcmp(level1T,"Shortlist")==0)
        {
            fscanf(file,"%s%s",level2T,level3T);
            shortlist(level2T,level3T,rqst, fd);
            //call shortlist(start,end)
        }
        if(strcmp(level1T,"Longlist")==0)
        {
            showblob(rqst,fd);
            //call longlist()
        }
        if(strcmp(level1T,"RegEx")==0)
        {

            FILE* temp;
            temp = fopen(".response", "w");
            fscanf(file,"%s",level2T);
            blob *returnval=regexsearch(level2T,bloba);
            for(i=0;i<regexreturnsize;i++)
                if(strcmp(returnval[i].name,"-1")!=0)
                    fprintf(temp,"%s\n",returnval[i].name);

            fclose(temp);
            upload(rqst, fd, ".response", 1);
        }
    }
    if(strcmp(rootT,"FileHash")==0)
    {
        fscanf(file,"%s",level1T);
        if(strcmp(level1T,"Verify")==0)
        {

            FILE* temp;
            temp = fopen(".response", "w");
            fscanf(file,"%s",level2T);
            char *returnhash=file2md5(level2T);
            char modifytime[100];
            strcpy(modifytime, getFileCreationTime(level2T));
            fprintf(temp,"NewHash= %s NewModifyTime= %s\n",returnhash,modifytime);
            for( i=0;i<blobasize;i++)
            {
                if(strcmp(bloba[i].name,level2T)==0)
                    fprintf(temp,"OldHash= %s OldModifyTime= %s\n",bloba[i].md5hash,bloba[i].modifytime);
            }
            fclose(temp);
            upload(rqst, fd, ".response", 1);

            //find the hash of the file level2
        }
        if(strcmp(level1T,"CheckAll")==0)
        {

            // find the hash of all the shared files
            checkall(rqst, fd);

        }
    }
    if(strcmp(rootT,"FileDownload")==0)
    {

        fscanf(file,"%s",level1T);
        upload(rqst, fd, level1T, 0);
        //download the file level1
    }
    return;
}

//Function that recieves all the data
int recvFile(int rqst, int sock)
{
    //Function variables
    ssize_t rval;
    char buffer[sizeOfPacket + 2];
    FILE* file;
    char fileN[256]; //file name
    char fileT[256]; //temp var
    char fileP[256]; //file path
    char fileS[256]; // file Size
    char fileNum[256]; // packet number of file
    char filePack[256]; // file pack size
    char fileTotalPackets[256]; // total packets in the file
    int size, totalPackets;
    int perm = 1;
    int switchS = 0;
    int fileMetaSize=0;
    int header = 0;
    int i, off;
    int totalFileWritten = 0;
    char* tempCheckSum;

    //recieve initial data sent from the other side and parse the header
    rval = recv(sock, buffer, sizeof(buffer), 0);
    if(rval < 0)
    {
        return 3;
    }
    if(recv == 0)
    {
        printf("Connection closed from other side\n");
        return 3;
    }
    //Parse file header
    if(buffer[0] == '+')
    {

        header = 0;

        for (i = 2; i < strlen(buffer); i++)
        {
            fileMetaSize = fileMetaSize + 1;
            if(buffer[i]!='\n')
            {
                if(switchS == 0)
                {
                    if(buffer[i]== ' ')
                    {
                        fileS[i-2]='\0';
                        switchS = 1;
                        continue;
                    }
                    fileS[i-2] = buffer[i];
                }
                else if(switchS == 1)
                {
                    if(buffer[i] != ' ')
                        fileP[i-(strlen(fileS)+1)-2] = buffer[i];
                    else
                    {
                        switchS = 4;
                        fileP[i-(strlen(fileS)+1)-2]='\0';
                    }
                }
                else if(switchS == 4)
                {
                    if(buffer[i] != ' ')
                        fileTotalPackets[i-(strlen(fileS)+strlen(fileP)+2)-2] = buffer[i];
                    else
                    {
                        switchS = 2;
                        fileTotalPackets[i-(strlen(fileS)+strlen(fileP)+2)-2]='\0';
                    }
                }
                else if(switchS == 2)
                {
                    if(buffer[i] != ' ')
                        filePack[i-(strlen(fileS) + strlen(fileP)+strlen(fileTotalPackets)+3)-2] = buffer[i];
                    else
                    {
                        switchS = 3;
                        filePack[i-(strlen(fileS)+strlen(fileP)+strlen(fileTotalPackets)+3)-2]='\0';
                    }
                }
                else if(switchS == 3)
                {
                    fileNum[i-(strlen(fileS) + strlen(fileP)+strlen(filePack)+ strlen(fileTotalPackets) +4 )-2] = buffer[i];
                }

            }
            else
            {
                fileNum[i-(strlen(fileS)+strlen(fileP)+strlen(filePack)+strlen(fileTotalPackets)+4)-2]='\0';
                break;
            }
            /* code */
        }

        int charCount = 0;
        for(i = strlen(fileP) - 1; i >= 0  ;i--)
        {
            if(fileP[i] == '/')
            {
                fileT[charCount] = '\0';

                break;
            }
            else
            {
                fileT[charCount] = fileP[i];
                charCount++;
            }

        }
        int j;

        for(j = 0 ; j < strlen(fileT);j++)
        {
            fileN[j] = fileT[(strlen(fileT)-1)-j];
        }

        off = fileMetaSize + 2;
        size = atoi(filePack);
        file = fopen(".tempW","wb+");
        while(1)
        {
            int out = 0;

            do
            {

                int written = fwrite(&buffer[off], 1, rval - off, file);
                if(written < 1)
                {
                // printf("could not be %d\n", written);

                //printf("Can not write to file\n");
                    break;
                //fclose(file);
                //return -1;
                }
                off += written;
                totalFileWritten += written;
                if(totalFileWritten >= size)
                {
                // printf("File written %d\n", totalFileWritten);

                    out = 1;
                    break;
                }
            }   
            while((off < rval) && out==0);
            if(out!=1)
            {
                rval = recv(sock, buffer, sizeof(buffer), 0);
                if(rval < 0)
                {
                //printf("Value of rval is %d\n", rval);
                    return 3;
                }
                if(recv == 0)
                {
                    printf("Connection closed from other side\n");
                    return 3;
                }
                off = 0;
            }


            if(totalFileWritten >= size)
            {
                break;
            }
        }
        totalFileWritten = 0;
        off =0;
        fclose(file);
        char name[100];
        strcpy(name,".tempW");
        tempCheckSum = checkSum(name);


        fileN[strlen(fileT)] = '\0';

        int tempno;
        tempno = atoi(fileNum);
        if(tempno == 0)
        {
            file = fopen(fileN, "wb"); //change it to ab+
        }
        else
            file = fopen(fileN, "ab+"); //change it to ab+

        size = atoi(filePack);

    }
    //parse response header

    else if(buffer[0] == '*')
    {
        header = 2;
        for (i = 2; i < strlen(buffer); i++)
        {
            fileMetaSize = fileMetaSize + 1;
            if(buffer[i]!='\n')
            {
                if(switchS == 0)
                {
                    if(buffer[i]== ' ')
                    {
                        fileS[i-2]='\0';
                        switchS = 1;
                        continue;
                    }
                    fileS[i-2] = buffer[i];
                }
                else
                {
                    fileP[i-(strlen(fileS)+1)-2] = buffer[i];
                }
            }
            else
            {
                fileP[i-(strlen(fileS)+1)-2]='\0';
                break;
            }
            /* code */
        }


        int charCount = 0;
        for(i = strlen(fileP) - 1; i >= 0  ;i--)
        {
            if(fileP[i] == '/')
            {
                fileT[charCount] = '\0';

                break;
            }
            else
            {
                fileT[charCount] = fileP[i];
                charCount++;
            }

        }
        int j;

        for(j = 0 ; j < strlen(fileT);j++)
        {
            fileN[j] = fileT[(strlen(fileT)-1)-j];
        }

        fileN[strlen(fileT)] = '\0';


        file = fopen(fileN, "w");
        if(!file)
        {
            printf("file name of resp - > %s could not be opened\n", fileN);

        }
    }

    //parse communication between the two servers
    else if(buffer[0] == '-')
    {

        fileMetaSize = 0;
        header = 1;
        strcpy(fileN, ".tempFile");
        strcpy(fileS, "80");
        file = fopen(fileN, "w");
        totalFileWritten = 3;
        size = atoi(fileS);
    }

    int tempLen = atoi(fileNum);

    off = fileMetaSize + 2;

    if(!file)
    {
        printf("Can not open file %s for writing\n", fileN);
        return 3;
    }

    //printf("Printing strings File Size %s, File Name %s, File Meta data size%d\n", fileS, fileN, fileMetaSize);


    // write to the file
    if(header == 0)
    {
        FILE* toRead = fopen(".tempW", "r");
        int ch = 0;
        while ( (ch = fgetc(toRead)) != EOF ) {
            fprintf(file, "%c",ch);
        }
        fclose(toRead);
    }
    else
    {
        while(1)
        {
            int out = 0;

            do
            {

                int written = fwrite(&buffer[off], 1, rval - off, file);
                if(written < 1)
                {
                // printf("could not be %d\n", written);

                //printf("Can not write to file\n");
                    break;
                //fclose(file);
                //return -1;
                }
                off += written;
                totalFileWritten += written;
                if(totalFileWritten >= size)
                {
                    out = 1;
                    break;
                }
            }	
            while((off < rval) && out==0);
            if(out!=1)
            {
                rval = recv(sock, buffer, sizeof(buffer), 0);
                if(rval < 0)
                {
                    return 3;
                }
                if(recv == 0)
                {
                    printf("Connection closed from other side\n");
                    exit(1);
                    return 0;
                }
                off = 0;
            }


            if(totalFileWritten >= size)
            {
                break;
            }
        }   
    }
    fclose(file);

    if(header == 0)
    {
        tempLen = tempLen + 1;
        sprintf(fileNum, "%d", tempLen);
        char res[256];
        strcpy(res, "~ ");
        strcat(res, fileNum);
        strcat(res, " ");
        strcat(res, tempCheckSum);
        strcat(res, "f\n");
        int len = strlen(res);
        //send acknowledgement
        int retVal = SetSocketBlockingEnabled(rqst,1);
        sendall(rqst, res, &len);
        retVal = SetSocketBlockingEnabled(rqst,0);
        //
        int tempno = atoi(fileTotalPackets);
        //if whole file recieved
        if(tempno == tempLen)
            printf("File %s successfully received\n", fileN);
    }	

    //calling the function requested
    if(header == 1)
    {
        file =fopen(fileN,"r");
        parse(file, rqst, sock);
    }

    //printing the output to screen
    if(header == 2)
    {
        FILE* forRead = fopen(fileN, "r");
        if(!forRead)
        {
            printf("Could not open file for readingn\n");
            return 2;
        }
        else
        {
            int ch = 0;
            while ( (ch = fgetc(forRead)) != EOF ) {
                printf("%c", ch);
            }
        }

    }
    return 2;	

}

//enable socket blocking for a socket
int SetSocketBlockingEnabled(int fd, int blocking)
{
    if (fd < 0) return 0;

    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return 0;
    flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0) ? 1 : 0;
}


//polling for a key press
int checkKeyPress()
{

    struct timeval tv;
    fd_set readfds;

    tv.tv_sec = 0;
    tv.tv_usec = 500000;

    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);

    select(STDIN+1, &readfds, NULL, NULL, &tv);
    if (FD_ISSET(STDIN, &readfds))
    {
        scanf("%s",root);

    }

    return 0;

}

//send a file through the socket
int str_server(int connected, char* send_data)
{
    char buffer[1025];


    const char* filename = send_data;

    /* Sending file data */
    FILE *file = fopen(filename, "rb");

    if(!file)
    {
        printf("Can't open file for reading.");
        return ;
    }


    while(!feof(file))
    {
        int rval = fread(buffer, 1, sizeof(buffer), file);
        if(rval < 1)
        {
            printf("printing value of rval %d\n", rval);
            printf("Cant read from file %s\n",filename);
            fclose(file);
            return 0; 
        }
        int off =0;
        do
        {
            int sent = send(connected, &buffer[off], rval - off, 0);
            if(sent < 1)
            {
                printf("Can't write a socket.\n");
                fclose(file);
                return 0;
            }
            off += sent;
        }
        while(off < rval);


    }
    fclose(file);
    return 1;
}

//return file size
char* sendFileSize(const char *filename)
{
    char file_size[256];
    struct stat file_stat;
    int fileDescriptor = open(filename, O_RDONLY);
    if (fstat(fileDescriptor, &file_stat) < 0)
    {
        fprintf(stderr, "Error fstat --> %s", strerror(errno));
        return -1;
//        exit(EXIT_FAILURE);
    }
    close(fileDescriptor);
    sprintf(file_size, "%d", file_stat.st_size);
    return file_size;
}

//connect s2 to s1
void serve(int port, int fdSC)
{
    int svc;        /* listening socket providing service */
    int rqst;/* socket accepting the request */
    socklen_t alen;       /* length of address structure */
    struct sockaddr_in my_addr;    /* address of this service */
    struct sockaddr_in client_addr;  /* client's address */
    int sockoptval = 1;
    char hostname[128]; /* host name, for debugging */

    gethostname(hostname, 128);

    
    if(strcmp(type,"u")==0)
    {
        if ((svc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) { //TCP socket
            perror("cannot create socket");
            exit(1);
        }
    }
    else
    {


    if ((svc = socket(AF_INET, SOCK_STREAM, 0)) < 0) { //TCP socket
        perror("cannot create socket");
        exit(1);
    }
    }

    setsockopt(svc, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int)); //allows us to reuse the port

    memset((char*)&my_addr, 0, sizeof(my_addr));  /* 0 out the structure */
    my_addr.sin_family = AF_INET;   /* address family */
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int retVal = SetSocketBlockingEnabled(svc,1);
    if(retVal == 0)
    {
        printf("Non blocking could not be enabled\n");
    }
    /* bind to the address to which the service will be offered */
    if (bind(svc, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind failed");
        exit(1);
    }

    /* set up the socket for listening with a queue length of 5 */
    if (listen(svc, 5) < 0) {
        perror("listen failed");
        exit(1);
    }

    printf("server started on %s, listening on port %d\n", hostname, port);

    /* loop forever - wait for connection requests and perform the service */
    alen = sizeof(client_addr);     /* length of address */

    //while(1) 
    //{
    rqst = accept(svc, (struct sockaddr *)&client_addr, &alen);

    printf("received a connection from: %s port %d\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    retVal = SetSocketBlockingEnabled(fdSC,0);
    if(retVal == 0)
    {
        printf("Non blocking could not be enabled\n");
    }
    retVal = SetSocketBlockingEnabled(rqst, 0);
    if(retVal == 0)
    {
        printf("Non blocking could not be enabled\n");

    }

    prompt(rqst, fdSC);

}




/* conn: connect to the service running on host:port */
/* return -1 on failure, file descriptor for the socket on success */
int conn(char *host, int port, int forward)
{
    struct hostent *hp;	/* host information */
    unsigned int alen;	/* address length when we get the port number */
    struct sockaddr_in myaddr;	/* our address */
    struct sockaddr_in servaddr;	/* server address */
    int fd;  /* fd is the file descriptor for the connected socket */

    if (debug) printf("conn(host=\"%s\", port=\"%d\")\n", host, port);
    


    if(strcmp(type,"u")==0)
    {
        if ((fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) 
        { //TCP socket
            perror("cannot create socket");
            exit(1);
        }

    }
    else
    {

        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        {
            perror("cannot create socket");
            return -1;
        }
    }

    /* bind to an arbitrary return address */
    /* because this is the client side, we don't care about the */
    /* address since no application will connect here  --- */
    /* INADDR_ANY is the IP address and 0 is the socket */
    /* htonl converts a long integer (e.g. address) to a network */
    /* representation (agreed-upon byte ordering */

    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);
    int retVal = SetSocketBlockingEnabled(fd,1);
    if(retVal == 0)
    {
        printf("Non blocking could not be enabled\n");
    }

    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        close(fd);
        return -1;
    }

    /* this part is for debugging only - get the port # that the operating */
    /* system allocated for us. */
    alen = sizeof(myaddr);
    if (getsockname(fd, (struct sockaddr *)&myaddr, &alen) < 0) {
        perror("getsockname failed");
        close(fd);
        return -1;
    }
    if (debug) printf("local port number = %d\n", ntohs(myaddr.sin_port));

    /* fill in the server's address and data */
    /* htons() converts a short integer to a network representation */

    memset((char*)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    /* look up the address of the server given its name */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "could not obtain address of %s\n", host);
        close(fd);
        return -1;
    }

    /* put the host's address into the server address structure */
    memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);

    /* connect to server */
    if(strcmp(type,"t")==0)
    {
    if (connect(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 
    {
        //perror("connect failed");
        close(fd);
        return -1;
    }
    if (debug) printf("connected socket = %d\n", fd);
    if(forward == 1)
        serve(RETURN_SERVICE_PORT, fd);
    }
    else
    {
        while(1)
        {
        char message[256];
        strcpy(message, "dummy");
        if (sendto(fd, message, strlen(message) , 0 , (struct sockaddr *) &servaddr, sizeof(servaddr))==-1)
        {
            perror("sendto()");
            exit(1);
        }
        break;
    }
    universal = servaddr;


    }

    return fd;
}
void getCorrectValue()
{
    int s;
    int total = 0;        // how many bytes we've sent
    int n;
    strcpy(type,"t");
    return;
}
//prompt that shows up
void prompt(int rqst, int fd)
{

    while(1)
    {

        strcpy(root,"123prev123");

        int val = checkKeyPress();

        if(strcmp(root, "123prev123") != 0)
        {
            getFunction(rqst, fd); //checks for the fuction entered
        }

        int ret = recvFile(rqst, fd);
        if(ret == 0)
        {
            perror("connection closed:");
            exit(1);
        }
        if(ret == 1)
        {
            printf("Error\n");
            exit(1);
        }
        if(ret == 3)
        {
           continue;
        }
        if(ret == 2)
        {
            continue;
        }
        continue;			

    }	
}
int sendall(int s, char *buf, int *len)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        if(type=='u')
        {
            if (sendto(s, buf, strlen(buf) , 0 , (struct sockaddr *) &universal, sizeof(universal))==-1)
            {
                perror("sendto()");
                exit(1);
            }

        }
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 
/* disconnect from the service */
/* lame: we can just as easily do a shutdown() or close() ourselves */
void shortlist(char *start,char* end, int rqst, int fd)
{
    FILE* temp;
    temp = fopen(".response", "w");

    int i;
    //fprintf(temp,"Checking it out %d\n", rqst);
    for (i=0;i<blobasize;i++)
        if(strcmp(bloba[i].name,"-1")!=0)
            if(strcmp(bloba[i].timestamp,start)>=0 && strcmp(bloba[i].timestamp,end)<=0)
                fprintf(temp,"%s %s %s %s %d %s\n",bloba[i].type,bloba[i].name,bloba[i].modifytime,bloba[i].md5hash,bloba[i].size,bloba[i].timestamp);

    fclose(temp);

    upload(rqst, fd, ".response", 1);           
}


char *file2md5(char *filename)
{

    unsigned char c[MD5_DIGEST_LENGTH];
    int i;
    FILE *inFile = fopen (filename, "rb");
    MD5_CTX mdContext;
    int bytes;
    char *out = (char*)malloc(33);
    unsigned char data[1024];

    if (inFile == NULL) {
        printf ("%s can't be opened.\n", filename);
        return 0;
    }

    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);

    for (i = 0; i < 16; ++i) {
        snprintf(&(out[i*2]), 16*2, "%02x", (unsigned int)c[i]);
    }
    fclose (inFile);
    return out;
}

void delete(char *token,blob *blobi)
{
    int i,j;
    for(i=0;i<blobasize;i++)
    {
        if(strcmp(token,blobi[i].name)==0)
        {
            strcpy(blobi[i].name,"-1");
            strcpy(blobi[i].modifytime,"-1");
            return;
        }

    }
    char temp[100];
    strcpy(temp,"^");
    strcat(temp,token);
    strcat(temp,"/*");
    //printf("%s\n",temp);
    blob *returnval=regexsearch(temp,blobi);
    for(i=0;i<blobasize;i++)
    {
        for(j=0;j<regexreturnsize;j++)
        {
            if(strcmp(returnval[j].name,blobi[i].name)==0)
            {
                strcpy(blobi[i].name,"-1");
                strcpy(blobi[i].modifytime,"-1");
            }
        }
    }
    return;
}

void checkall(int rqst, int fd)
{
    int i;
    for(i=0;i<blobasize;i++)
    {
        strcpy(bloba[i].md5hash,file2md5(bloba[i].name));
        strcpy(bloba[i].modifytime,getFileCreationTime(bloba[i].name));
    }
    showblob(rqst, fd);
    return;
}

blob* regexsearch(char *token,blob *blobi)
{
    int i;
    regexreturnsize=0;
    int reti = regcomp(&regex, token, 0);
    if( reti ){ fprintf(stderr, "Could not compile regex\n");return; }

    for(i=0;i<blobasize;i++)
    {
        reti = regexec(&regex,blobi[i].name, 0, NULL, 0);
        if(!reti)
        {
            strcpy(regexreturn[regexreturnsize].name,blobi[i].name);
            strcpy(regexreturn[regexreturnsize++].modifytime,blobi[i].modifytime);
            printf("%s\n",blobi[i].name);
        }

    }
    regfree(&regex);
    return regexreturn;
}

void showblob(int rqst, int fd)
{
    int i;
    if(rqst!=-1)
    {
        FILE* temp;
        temp = fopen(".response", "w");

        //fprintf(temp, "Kickass blob!!\n");
        for(i=0;i<blobasize;i++)
            if(strcmp(bloba[i].name,"-1")!=0)
                fprintf(temp,"%s %s %s %s %d %s\n",bloba[i].type,bloba[i].name,bloba[i].modifytime,bloba[i].md5hash,bloba[i].size,bloba[i].timestamp);
        fclose(temp);
        upload(rqst, fd, ".response", 1); 
        return;
    }
    else
    {
        for(i=0;i<blobasize;i++)
            if(strcmp(bloba[i].name,"-1")!=0)
                printf("%s %s %s %s %d %s\n",bloba[i].type,bloba[i].name,bloba[i].modifytime,bloba[i].md5hash,bloba[i].size,bloba[i].timestamp);
        return;

    }

}

int duplicate(blob *blobi, char *name)
{
    int i;
    for(i=0;i<blobasize;i++)
        if(strcmp(name,blobi[i].name)==0)
            return 1;
    return 0;
}

char* getFileCreationTime(char *filePath)
{
    char modifytime[50];
    struct stat attrib;
    stat(filePath, &attrib);
    strcpy(modifytime,ctime(&attrib.st_mtime));
    /*int timestamp=(int)ctime(&attrib.st_mtime);
      sprintf(modifytime,"%d",timestamp);*/
    return modifytime;
}

char *getFileTimestamp(char *filePath)
{
    unsigned int timestamp;
    char buf[100];
    struct stat attrib;
    struct tm* timeinfo;
    stat(filePath, &attrib);
    // printf("%d\n",&attrib.st_mtime);

    timeinfo=localtime(&attrib.st_mtime);
    strftime(buf, sizeof(buf), "%Y.%m.%d*%H:%M:%S", timeinfo);
    //puts(buf);

    /*int timestamp=(int)ctime(&attrib.st_mtime);
      sprintf(modifytime,"%d",timestamp);*/
    return buf;
}

int getFileSize(char *filename)
{
    struct stat st;
    stat(filename, &st);
    int size = st.st_size;
    return size;
}

char *get_filename_ext(char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int list(char *name,  struct stat *status, int type) {
    //char buffer[100];
    char modifytime[50];
    if(type == FTW_NS)
        return 0;
    strcpy(modifytime,getFileCreationTime(name));
    //timestamp=getFileTimestamp(name);
    if(type == FTW_F)
    {
        if(!duplicate(bloba,name))
        {
            char *returnhash=file2md5(name);
            strcpy(bloba[blobasize].name,name);
            strcpy(bloba[blobasize].modifytime,modifytime);
            strcpy(bloba[blobasize].md5hash,returnhash);
            bloba[blobasize].size=getFileSize(name);
            strcpy(bloba[blobasize].timestamp,getFileTimestamp(name));
            strcpy(bloba[blobasize++].type,get_filename_ext(name));
        }
    }
    if(type == FTW_D && strcmp(".", name) != 0)
    {
        // do nothing
        int a=0;
    }
    return 0;
}

int getTotalPackets(int file_size)
{
    int ans = file_size/sizeOfPacket;
    if(file_size % sizeOfPacket == 0)
        return ans;
    else
        return ans + 1; 
}

void upload(int fd, int rqst, char* file, int response)
{
    char* file_size;
    char *fs;
    char packetSize[256];
    char fileMetaData[256];
    char packetMetaData[256];
    char final[256];
    ssize_t rval;
    char buffer[sizeOfPacket + 2];
    int i, fileNum;
    char fileNumStr[256];
    char strTotalPackets[256];
    int retVal, retValue, len;
    file_size = sendFileSize(file);
    if(file_size == -1)
        return;

    int sizeOfFile = atoi(file_size);
    if(response == 1)
    {
        strcpy(fileMetaData, "* ");
        strcat(fileMetaData, file_size);
        strcat(fileMetaData," ");
        strcat(fileMetaData, file);
        strcat(fileMetaData,"\n");
        if(type=='u')
        {
            if (sendto(fd, fileMetaData, strlen(fileMetaData) , 0 , (struct sockaddr *) &universal, sizeof(universal))==-1)
            {
                perror("sendto()");
                exit(1);
            }

        }
        int len = send(fd, fileMetaData, strlen(fileMetaData), 0);
        int retValue = str_server(fd, file);
        return;
    }
    int totalPackets = getTotalPackets(sizeOfFile);
    sprintf(strTotalPackets, "%d", totalPackets);

    fs = sendFileSize(file);
     if(fs == -1)
        return;

    strcpy(fileMetaData, "+ ");
    strcat(fileMetaData, fs);
    strcat(fileMetaData," ");
    strcat(fileMetaData, file);
    strcat(fileMetaData," ");
    strcat(fileMetaData, strTotalPackets);

    FILE* fptr = fopen(file, "r");
    FILE* tempW = fopen(".packet","w");
    FILE* forRead;
    int ch = 0;
    int packSize = 0;
    int number = 0;
    char strNumber[3000];
    char tempCheckSum[256];
    char checkSumStr[256];
    strcpy(tempCheckSum, "123cd");
    strcpy(checkSumStr, "cd123");

    while ( ((ch = fgetc(fptr)) != EOF) ) 
    {
        packSize++;
        fprintf(tempW, "%c", ch);
        if(packSize == sizeOfPacket)
        {
            fileNum = -1;
            while( (fileNum < number) || (strcmp(checkSumStr, tempCheckSum) != 0) )
            {
                sprintf(packetSize, "%d", sizeOfPacket);
                strcpy(packetMetaData," ");
                strcat(packetMetaData, packetSize);
                sprintf(strNumber, "%d", number);
                strcat(packetMetaData," ");
                strcat(packetMetaData, strNumber);
                strcat(packetMetaData,"\n");
                strcpy(final, fileMetaData);
                strcat(final, packetMetaData);
                len = send(fd, final, strlen(final), 0);
                fclose(tempW);
                char* tempstr;
                tempstr = checkSum(".packet");
                strcpy(tempCheckSum, tempstr);
                retValue = packetUpload(fd);
                retVal = SetSocketBlockingEnabled(rqst,1);
                rval = recv(rqst, buffer, sizeof(buffer), 0);
                if(recv<0)
                {
                    printf("Recvieved error");
                    exit(1);
                }
                if(recv == 0)
                {
                    printf("Connection closed from other side\n");
                    exit(1);
                }
                i = 2;
                int state = 0;
                while(buffer[i]!='f')
                {
                    if(buffer[i] != ' ' && state == 0)
                    {
                        fileNumStr[i-2] = buffer[i];
                    }
                    else
                    {
                        fileNumStr[i] = '\0';
                        state = 1;
                    }
                    if(state == 1)
                    {
                        checkSumStr[i-2-(strlen(fileNumStr) + 1)] = buffer[i];
                    }
                    i++;
                }
                checkSumStr[i-2-(strlen(fileNumStr) + 1)] = '\0';
                fileNum = atoi(fileNumStr);
                retVal = SetSocketBlockingEnabled(rqst,0);
                if(strcmp(checkSumStr, tempCheckSum)!=0)
                {
                    printf("Unequal check sum, requesting packet again\n");
                    continue;
                }
                if(fileNum < number)
                {
                    printf("Order of packets wrong. Requesting packet again\n");
                    continue;
                }
            }
            tempW = fopen(".packet","w");
            packSize = 0;
            number++;
        }
    }
    if(packSize!=0)
    {
        char strSize[256];
        fileNum = -1;
        while(fileNum <= number)
        {
            sprintf(strSize, "%d", packSize);
            sprintf(strNumber, "%d", number);

            strcpy(packetMetaData," ");
            strcat(packetMetaData, strSize);
            sprintf(strNumber, "%d", number);
            strcat(packetMetaData," ");
            strcat(packetMetaData, strNumber);
            strcat(packetMetaData,"\n");
            strcpy(final, fileMetaData);
            strcat(final, packetMetaData);

            int len = send(fd, final, strlen(final), 0);
            fclose(tempW);
            int retValue = packetUpload(fd);
            int retVal = SetSocketBlockingEnabled(rqst,1);
            rval = recv(rqst, buffer, sizeof(buffer), 0);
            if(rval > 0)
            {
                i = 2;
                while(buffer[i]!='f')
                {
                    fileNumStr[i-2] = buffer[i];
                    i++;
                }
                fileNum = atoi(fileNumStr);
            }
            if(recv == 0)
            {
                printf("Connection closed from other side\n");
                exit(1);
            }
            retVal = SetSocketBlockingEnabled(rqst,0);
        } 
        packSize = 0;
        number++;
    }

    return;
}

int packetUpload(int fd)
{
    char name[100];
    strcpy(name,".packet");
    return str_server(fd, name);
}

void getFunction(int rqst, int fd)
{

    char header[80];
    strcpy(header, "- "); 
    int i;
    if(strcmp(root,"IndexGet")==0)
    {
        scanf("%s",level1);
        if(strcmp(level1,"Shortlist")==0)
        {
            scanf("%s%s",level2,level3);
            strcat(header, root);
            strcat(header, " ");

            strcat(header, level1);
            strcat(header, " ");

            strcat(header, level2);
            strcat(header, " ");

            strcat(header, level3);
            strcat(header, " ");
            for(i = strlen(header); i< 79;i++)
            {
                header[i] = '0';
            }
            header[79] = '\0';
            int lambai = strlen(header);
            if (sendall(rqst, header, &lambai) == -1) 
            {
                perror("sendall");
                printf("We only sent %d bytes because of the error!\n", lambai);
            }


            //shortlist(level2,level3);
            //call shortlist(start,end)
        }
        if(strcmp(level1,"Longlist")==0)
        {

            strcat(header, root);
            strcat(header, " ");

            strcat(header, level1);
            strcat(header, " ");
            for(i = strlen(header); i< 79;i++)
            {
                header[i] = '0';
            }
            header[79] = '\0';
            int lambai = strlen(header);
            if (sendall(rqst, header, &lambai) == -1) 
            {
                perror("sendall");
                printf("We only sent %d bytes because of the error!\n", lambai);
            }


            //			showblob();
            //call longlist()
        }
        if(strcmp(level1,"RegEx")==0)
        {
            scanf("%s",level2);
            printf("IndexGet RegEx %s\n",level2);
            strcat(header, root);
            strcat(header, " ");

            strcat(header, level1);
            strcat(header, " ");

            strcat(header, level2);
            strcat(header, " ");
            for(i = strlen(header); i< 79;i++)
            {
                header[i] = '0';
            }
            header[79] = '\0';
            int lambai = strlen(header);
            if (sendall(rqst, header, &lambai) == -1) 
            {
                perror("sendall");
                printf("We only sent %d bytes because of the error!\n", lambai);
            }


            /*blob *returnval=regexsearch(level2,bloba);
              for(i=0;i<regexreturnsize;i++)
              if(strcmp(returnval[i].name,"-1")!=0)
              printf("%s\n",returnval[i].name);
              }*/
      }
  }
  if(strcmp(root,"FileHash")==0)
  {
    scanf("%s",level1);
    if(strcmp(level1,"Verify")==0)
    {
        scanf("%s",level2);
        printf("FileHash Verify %s\n",level2);
        strcat(header, root);
        strcat(header, " ");

        strcat(header, level1);
        strcat(header, " ");

        strcat(header, level2);
        strcat(header, " ");
        for(i = strlen(header); i< 79;i++)
        {
            header[i] = '0';
        }
        header[79] = '\0';
        int lambai = strlen(header);
        if (sendall(rqst, header, &lambai) == -1) 
        {
            perror("sendall");
            printf("We only sent %d bytes because of the error!\n", lambai);
        }
        /*char *returnhash=file2md5(level2);
          char modifytime[100];
          strcpy(modifytime, getFileCreationTime(level2));
          printf("NewHash= %s NewModifyTime= %s\n",returnhash,modifytime);
          for( i=0;i<blobasize;i++)
          {
          if(strcmp(bloba[i].name,level2)==0)
          printf("OldHash= %s OldModifyTime= %s\n",bloba[i].md5hash,bloba[i].modifytime);
          }*/
        //find the hash of the file level2
  }
  if(strcmp(level1,"CheckAll")==0)
  {
    strcat(header, root);
    strcat(header, " ");

    strcat(header, level1);
    strcat(header, " ");
    for(i = strlen(header); i< 79;i++)
    {
        header[i] = '0';
    }
    header[79] = '\0';
    int lambai = strlen(header);
    if (sendall(rqst, header, &lambai) == -1) 
    {
        perror("sendall");
        printf("We only sent %d bytes because of the error!\n", lambai);
    }
        // find the hash of all the shared files
    printf("FileHash CheckAll\n");
        //	checkall(rqst);
}
}
if(strcmp(root,"FileDownload")==0)
{
    scanf("%s",level1);
    strcat(header, root);
    strcat(header, " ");

    strcat(header, level1);
    strcat(header, " ");
    for(i = strlen(header); i< 79;i++)
    {
        header[i] = '0';
    }
    header[79] = '\0';
    int lambai = strlen(header);
    if (sendall(rqst, header, &lambai) == -1) 
    {
        perror("sendall");
        printf("We only sent %d bytes because of the error!\n", lambai);
    }
    //printf("FileDownload %s\n",level1);
    //upload(fd, level1);
    //download the file level1
}
if(strcmp(root,"FileUpload")==0)
{
    scanf("%s",level1);
    upload(rqst, fd, level1, 0);
    //upload the file level1;
}
if(strcmp(root,"Add")==0)
{
    scanf("%s",level1);

    ftw(level1, list, 1);
    showblob(-1, -1);
}
if(strcmp(root,"Delete")==0)
{

    scanf("%s",level1);

    delete(level1,bloba);
    printf("Delete\n");
    showblob(-1, -1);
}
if(strcmp(root,"disconn")==0)
{
    disconn(rqst);
    disconn(fd);
    exit(1);
}
return;
}

void disconn(int fd)
{
    if (debug) printf("disconn(%d)\n", fd);
    shutdown(fd, 2);    /* 2 means future sends & receives are disallowed */
}



