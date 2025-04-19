#include <stdio.h>
#include <windows.h>
#include <pthread.h>

#define NUM_LINHAS_POR_THREAD 1000000

typedef struct {
    int start;
    int nlinhas;
    FILE* file;
    int threadNum;
} ArchiveReaderParam;

void* processArchive(void* args)
{
    ArchiveReaderParam* param = (ArchiveReaderParam*) args;
    printf("Thread %d -> Inicio: %d | Nro: %d\n", param->threadNum, param->start, param->nlinhas);
}

void process(DWORD numThreads){
    pthread_t thread[numThreads];
    ArchiveReaderParam param[numThreads + 1];

    FILE *arquivo = NULL;
    int contador = 0;
    char linha[1024];

    arquivo = fopen("data/devices.csv", "r");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    while (fgets(linha, sizeof(linha), arquivo)) {
        contador++;
    }

    contador--; //tirando o header

    int ultimaThread = (contador) % ((int)numThreads- 1);

    int linhasPorThread = 0;

    boolean isExactly = ultimaThread;

    if(ultimaThread == 0)
    {

    }
    else{
    }
    
    linhasPorThread = (contador) / ((int)numThreads - 1);
    
    for(int i = 0; i < numThreads; i++)
    {
        if(i < numThreads - 1)
        {
            param[i].start = linhasPorThread * i;
            param[i].nlinhas = linhasPorThread;
            param[i].file = arquivo;
            param[i].threadNum = i +1;
        }
        else{
            param[i].start = linhasPorThread * i;
            param[i].nlinhas = ultimaThread;
            param[i].file = arquivo;
            param[i].threadNum = i +1;
        }
        
        pthread_create(&thread[0], NULL, processArchive, &param[i]);
    }

    for(int i = 0; i < numThreads; i++)
    {
        pthread_join(thread[i], NULL);
    }
    
    fclose(arquivo);

    printf("Num de linhas %d\n", contador);
    printf("Thread finalizada\n");

    return;
}

void executeInWin32()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
	
    printf("Numero de processadores logicos: %u\n", sysinfo.dwNumberOfProcessors);
	
	process(sysinfo.dwNumberOfProcessors);
    return;
}

void processBySO()
{
    #if defined(_WIN32)
    	executeInWin32();
    #elif defined(__linux__)
    #endif

    return;
}

int main() {

    processBySO();

    return 0;
}
