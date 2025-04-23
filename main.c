#include <stdio.h>
#include <windows.h>
#include <pthread.h>

typedef struct {
    int start;
    int nlinhas;
    int threadNum;
} ArchiveReaderParam;

#define MAX_LINE 1024

typedef struct {
    int id;
    char device[50];
    int contagem;
    char data[20];
    float temperatura;
    float umidade;
    float luminosidade;
    float ruido;
    int eco2;
    int etvoc;
    double latitude;
    double longitude;
} Registro;

Registro MapRegistro(char *token)
{
    Registro r;

    printf("%s", token);

    // r.id = atoi(token);
    // token = strtok(NULL, "|");
    // strcpy(r.device, token);

    // token = strtok(NULL, "|");
    // r.contagem = atoi(token);

    // token = strtok(NULL, "|");
    // strcpy(r.data, token);

    // token = strtok(NULL, "|");
    // r.temperatura = atof(token);

    // token = strtok(NULL, "|");
    // r.umidade = atof(token);

    // token = strtok(NULL, "|");
    // r.luminosidade = atof(token);

    // token = strtok(NULL, "|");
    // r.ruido = atof(token);

    // token = strtok(NULL, "|");
    // r.eco2 = atoi(token);

    // token = strtok(NULL, "|");
    // r.etvoc = atoi(token);

    // token = strtok(NULL, "|");
    // r.latitude = atof(token);

    // token = strtok(NULL, "|");
    // r.longitude = atof(token);

    return r;
}

void* processArchive(void* args)
{
    ArchiveReaderParam* param = (ArchiveReaderParam*) args;
    printf("Thread %d -> Inicio: %d | Nro: %d\n", param->threadNum, param->start, param->nlinhas);
    
    FILE *file = fopen("data/devices.txt", "r");

    if (!file) {
        perror("Erro ao abrir o arquivo\n");
        return NULL;
    }

    char linha[1024];
    int capacidade = param->nlinhas;

    for (int i = 0; i < param->start; i++) {
        if (fgets(linha, sizeof(linha), file) == NULL) {
            fprintf(stderr, "%d - Arquivo tem menos de %d linhas! %s\n",i, param->start, linha);
            fclose(file);
            return NULL;
        }
    }
    fgets(linha, sizeof(linha), file);
    
    // Registro registros[capacidade];

    for(int total = 0; total < param->nlinhas; total++)
    {
        printf("%s", linha);
        char *token = strtok(linha, "|");

        // registros[total] = MapRegistro(token);
        // printf("%d - %s", registros[total].id, registros[total].device);
        fgets(linha, sizeof(linha), file);
    }
    fclose(file);

    return NULL;
}

int CountLines()
{
    FILE *file = NULL;

    file = fopen("data/devices.txt", "r");

    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return 0;
    }

    int contador = 0;
    char linha[MAX_LINE];

    while (fgets(linha, sizeof(linha), file)) {
        contador++;
    }

    printf("Numero de linhas: %d \n", contador);
    fclose(file);

    return contador;
}

void processWin32(DWORD numThreads){
    pthread_t thread[numThreads];
    ArchiveReaderParam param[numThreads + 1];

    int lines = CountLines();

    if(lines == 0)
    {
        return;
    }
    
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
        param[i].threadNum = i +1;

        if(i == 0)
        {
            param[i].start = 1;
        }
        else
        {
            param[i].start = (linhasPorThread * i);
        }
        
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
