#include <fcntl.h>
#include <sys/resource.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "UnixOs.h"

void SetCoreFileUnlimit() {
    struct rlimit rlim;
    struct rlimit rlim_new;
    if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
        rlim_new.rlim_cur = rlim_new.rlim_max = RLIM_INFINITY;
        if (setrlimit(RLIMIT_CORE, &rlim_new) != 0) {
            rlim_new.rlim_cur = rlim_new.rlim_max = rlim.rlim_max;
            setrlimit(RLIMIT_CORE, &rlim_new);
        }
    }
}

std::string GetOsIp() {
    std::string ip;
    struct ifaddrs * ifAddrStruct = NULL;
    void * tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);

    while (ifAddrStruct != NULL)
    {
        if (ifAddrStruct->ifa_addr->sa_family == AF_INET &&
            (ifAddrStruct->ifa_flags & IFF_RUNNING) &&
            !(ifAddrStruct->ifa_flags & IFF_LOOPBACK)) {

            tmpAddrPtr = &((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            ip = addressBuffer;
            break;
        }
        ifAddrStruct = ifAddrStruct->ifa_next;
    }
    return std::move(ip);
}
