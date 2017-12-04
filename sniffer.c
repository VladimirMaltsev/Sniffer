#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <linux/if_ether.h>
#include <linux/if.h>

#include <arpa/inet.h>


int main(int argc, char **argv) {

    int sock, opt_sock, n;
    struct ifreq interface;
    char buffer [65536];
    unsigned char *iphead, *ethhead;

    if ((sock = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ALL))) < 0) {
        perror ("socket error");
        exit (1);
    }
    
    //привязка сокета к устройству 
    if ((opt_sock = setsockopt(sock, SOL_SOCKET, 
        SO_BINDTODEVICE, "eth0\x00", strlen("eth0\x00"+1))) != -1) {
            perror ("Couldn`t bind the socket to the device");
            close(sock);
            exit (1);
    }
    
    //установление promiscious mode для приема абсолютно всех пакетов
    ioctl (sock, SIOCGIFFLAGS, &interface);
    interface.ifr_flags |= IFF_PROMISC;
    ioctl (sock, SIOCSIFFLAGS, &interface);

    while (1) {
        printf (">--------------------------------------\n");
        n = recvfrom(sock, buffer, sizeof(buffer), 0, 0, 0);
        printf ("%d bytes were read\n", n);

        if (n < 42) {
            perror ("recvfrom(): incomplete packet");
            //interface.ifr_flags &= ~IFF_PROMISC;
            //ioctl (sock, SIOCSIFFLAGS, &interface);
            close (sock);
            exit (0);
        }

        ethhead = buffer;
        printf("Source MAC address: "
           "%02x:%02x:%02x:%02x:%02x:%02x\n",
           ethhead[0],ethhead[1],ethhead[2],
           ethhead[3],ethhead[4],ethhead[5]);

        printf ("Destination MAC address: "
                "%02x:%02x:%02x:%02x:%02x:%02x\n",
                ethhead[6],ethhead[7],ethhead[8],
                ethhead[9],ethhead[10],ethhead[11]);

        iphead = buffer + 14; 
        if (*iphead==0x45) { 
            printf("Source host %d.%d.%d.%d\n",
                    iphead[12],iphead[13],
                    iphead[14],iphead[15]);
            printf("Dest host %d.%d.%d.%d\n",
                    iphead[16],iphead[17],
                    iphead[18],iphead[19]);
            printf("Source,Dest ports %d,%d\n",
                    (iphead[20]<<8)+iphead[21],
                    (iphead[22]<<8)+iphead[23]);
            printf("Layer-4 protocol %d\n",iphead[9]);
        }
    }

    return 0;
}