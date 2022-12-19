#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
// Key Value Structure
struct kvStore{
	char *name;
	int rNo;
}info;

//  IOCTL Definition
#define readkvStore _IO('r' , 1)
#define kvWrite _IOW('r', 3, struct kvStore *)
#define searchANode _IOWR('r', 4, struct kvStore *)

int main(){
    int fd;
    int32_t number;
    printf("Opening Char Driver ...\n");
    fd = open("/dev/myKvStore", O_RDWR);
    if(fd < 0){
        printf("Failed to Open the device...\n");
        return 0;
    }
    //printf("Press Enter to Go...\n");
    //scanf("%d", &number);

	//char *arr[] = {"a","b", "c", "d", "e", "f", "g", "h"};
    char *arr[] = {"i","j", "k", "l", "m", "n", "o", "p"};
	//char *arr[] = {"q","r", "s", "t", "u", "v", "w", "x", "y", "z"};
    for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
    {
        printf("Nam is :: %s  Value is :: %d \n", arr[i], i);
        info.name = arr[i];
        info.rNo = i;
        ioctl(fd, kvWrite, &info);
        struct kvStore readKvl;
        readKvl.rNo=0;
        readKvl.name=arr[i];
        ioctl(fd, searchANode, &readKvl);
        printf("\t THis is the Search Name :: %s and rNo :: %d\n", readKvl.name, readKvl.rNo);

    }
    printf("Reading value from the Driver\n");
	//ioctl(fd,readkvStore);
    printf("Closing the driver\n");
    close(fd);
    return 0;
}
