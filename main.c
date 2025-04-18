#include <stdio.h>
#include <windows.h>
#include <pthread.h>

void process(int numThreads)
{
	pthread_t thread[numThreads];
	
    if (pthread_create(&thread, NULL, minhaThread, NULL)) {
        fprintf(stderr, "Erro ao criar thread\n");
        return;
    }

    pthread_join(thread, NULL);
    printf("Thread finalizada\n");
}

void executeInWin32()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
	
    printf("N�mero de processadores l�gicos: %u\n", sysinfo.dwNumberOfProcessors);
	
	process(sysinfo.dwNumberOfProcessors);
}

void processBySO()
{
    #if defined(_WIN32)
    	executeInWin32();
    #elif defined(__linux__)
    #endif
}

int main() {

    processBySO();

    return 0;
}
