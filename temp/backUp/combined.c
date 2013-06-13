#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <regex.h>
#include <openssl/md5.h>
#include <time.h>

// Walk function
int list(const char *name, const struct stat *status, int type);

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
int duplicate(blob *blobi,const char *name);

// Function to remove a file from sharing
void delete(char *token,blob *blobi);

// Helper function to print the contents of blob Array
void showblob();

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
void checkall();

void shortlist(char *start,char *end);

int main()
{
    int i;
    char *root,*level1,*level2,*level3;
    root=(char*)malloc(sizeof(char)*100);
    level1=(char*)malloc(sizeof(char)*100);
    level2=(char*)malloc(sizeof(char)*100);
    level3=(char*)malloc(sizeof(char)*100);
    while(1){
        scanf("%s",root);
        if(strcmp(root,"IndexGet")==0)
        {
            scanf("%s",level1);
            if(strcmp(level1,"Shortlist")==0)
            {
                scanf("%s%s",level2,level3);
                printf("IndexGet Shortlist %s %s\n",level2,level3);
                shortlist(level2,level3);
                //call shortlist(start,end)
            }
            if(strcmp(level1,"Longlist")==0)
            {
                printf("IndexGet Longlist\n");
                showblob();
                //call longlist()
            }
            if(strcmp(level1,"RegEx")==0)
            {
                scanf("%s",level2);
                printf("IndexGet RegEx %s\n",level2);
                blob *returnval=regexsearch(level2,bloba);
                for(i=0;i<regexreturnsize;i++)
                    if(strcmp(returnval[i].name,"-1")!=0)
                        printf("%s\n",returnval[i].name);
            }
        }
        if(strcmp(root,"FileHash")==0)
        {
            scanf("%s",level1);
            if(strcmp(level1,"Verify")==0)
            {
                scanf("%s",level2);
                printf("FileHash Verify %s\n",level2);
                char *returnhash=file2md5(level2);
                char modifytime[100];
                strcpy(modifytime, getFileCreationTime(level2));
                printf("NewHash= %s NewModifyTime= %s\n",returnhash,modifytime);
                for( i=0;i<blobasize;i++)
                {
                    if(strcmp(bloba[i].name,level2)==0)
                        printf("OldHash= %s OldModifyTime= %s\n",bloba[i].md5hash,bloba[i].modifytime);
                }
                //find the hash of the file level2
            }
            if(strcmp(level1,"CheckAll")==0)
            {
                // find the hash of all the shared files
                printf("FileHash CheckAll\n");
                checkall();
            }
        }
        if(strcmp(root,"FileDownload")==0)
        {
            scanf("%s",level1);
            printf("FileDownload %s\n",level1);
            //download the file level1
        }
        if(strcmp(root,"FileUpload")==0)
        {
            scanf("%s",level1);
            printf("FileUpload %s\n",level1);
            //upload the file level1;
        }
        if(strcmp(root,"Add")==0)
        {
            scanf("%s",level1);
            printf("Add\n");
            ftw(level1, list, 1);
            showblob();
        }
        if(strcmp(root,"Delete")==0)
        {
            scanf("%s",level1);
            delete(level1,bloba);
            printf("Delete\n");
            showblob();
        }
    }

    return 0;
}

void shortlist(char *start,char* end)
{
    int i;
    for (i=0;i<blobasize;i++)
        if(strcmp(bloba[i].name,"-1")!=0)
            if(strcmp(bloba[i].timestamp,start)>=0 && strcmp(bloba[i].timestamp,end)<=0)
                printf("%s %s %s %s %d %s\n",bloba[i].type,bloba[i].name,bloba[i].modifytime,bloba[i].md5hash,bloba[i].size,bloba[i].timestamp);
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
    printf("%s\n",temp);
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

void checkall()
{
    int i;
    for(i=0;i<blobasize;i++)
    {
        strcpy(bloba[i].md5hash,file2md5(bloba[i].name));
        strcpy(bloba[i].modifytime,getFileCreationTime(bloba[i].name));
    }
    showblob();
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

void showblob()
{
    int i;
    for(i=0;i<blobasize;i++)
        if(strcmp(bloba[i].name,"-1")!=0)
            printf("%s %s %s %s %d %s\n",bloba[i].type,bloba[i].name,bloba[i].modifytime,bloba[i].md5hash,bloba[i].size,bloba[i].timestamp);
        return;
}

int duplicate(blob *blobi,const char *name)
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

int list(const char *name, const struct stat *status, int type) {
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
