#include <stdio.h>
#include <unistd.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BR_FILE  "/dev/br"



int main()
{
    int fd, noblockfd;
    int v;
    int ret;
    int flags;

    fd = open(BR_FILE, O_RDONLY);
    if (fd < 0)
    {
        perror("open fail!");
        return -1;
    }
    
    ret = read(fd, &v, sizeof(v));
    if (ret < 0)
    {
        perror("read error!");
        close(fd);
        return -2;
    }

    printf("flag: %d\n", v);


    printf("noblock test:\n");
    noblockfd = open(BR_FILE, O_RDONLY, O_NONBLOCK);
    if (fd < 0)
    {
        perror("open fail!");
        close(fd);
        return -1;
    }
    
    flags = fcntl(noblockfd, F_GETFL, 0);
    fcntl(noblockfd, F_SETFL, flags|O_NONBLOCK);

    ret = read(noblockfd, &v, sizeof(v));
    if (ret < 0)
    {
        perror("read error!");
        close(fd);
        close(noblockfd);
        return -2;
    }
    printf("flag: %d\n", v);



    close(fd);
    close(noblockfd);
    return 0;
}
