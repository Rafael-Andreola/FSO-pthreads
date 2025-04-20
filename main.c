#include <stdio.h>
#include <windows.h>
#include <pthread.h>
// #include <Python.h>

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

    // Py_Initialize();
    // PyRun_SimpleString("print('Olá do Python!')");
    // Py_Finalize();

    printf("Thread %d -> Inicio: %d | Nro: %d\n", param->threadNum, param->start, param->nlinhas);
}

int CountLines(FILE *file)
{
    int contador = 0;
    char linha[1024];

    while (fgets(linha, sizeof(linha), file)) {
        contador++;
    }

    printf("Numero de linhas: %d \n", contador);

    return contador;
}

void processWin32(DWORD numThreads){
    pthread_t thread[numThreads];
    ArchiveReaderParam param[numThreads + 1];

    FILE *arquivo = NULL;

    arquivo = fopen("data/devices.csv", "r");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    int lines = CountLines(arquivo);
    
    lines--; //tirando o header

    int ultimaThread = (lines) % ((int)numThreads);
    int linhasPorThread = 0;

    if(ultimaThread == 0)
    {
        linhasPorThread = (lines) / ((int)numThreads);
        ultimaThread = linhasPorThread;
    }
    else{
        linhasPorThread = (lines) / ((int)numThreads- 1);
        ultimaThread = (lines) % ((int)numThreads - 1);
    }
    
    for(int i = 0; i < (int)numThreads; i++)
    {
        param[i].file = arquivo;
        param[i].threadNum = i +1;
        param[i].start = linhasPorThread * i;

        if(i == numThreads -1)
        {
            param[i].nlinhas = ultimaThread;
        }
        else{
            param[i].nlinhas = linhasPorThread;
        }

        pthread_create(&thread[0], NULL, processArchive, &param[i]);
    }

    for(int i = 0; i < numThreads; i++)
    {
        pthread_join(thread[i], NULL);
    }
    
    fclose(arquivo);

    printf("Num de linhas %d\n", lines);
    printf("Thread finalizada\n");

    return;
}

void executeInWin32()
{
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
	
    printf("Numero de processadores logicos: %u\n", sysinfo.dwNumberOfProcessors);
	
	processWin32(sysinfo.dwNumberOfProcessors);
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
