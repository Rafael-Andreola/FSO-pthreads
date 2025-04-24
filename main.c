#include <stdio.h>
#include <windows.h>
#include <pthread.h>


#define MAX_LINE 1024
#define TAM_DEVICE 50
#define TAM_DATA   30

typedef struct {
    int start;
    int nlinhas;
    int threadNum;
} ArchiveReaderParam;

typedef struct {
    int id;
    char device[TAM_DEVICE];
    int contagem;
    char data[TAM_DATA];
    float temperatura;
    float umidade;
    float luminosidade;
    float ruido;
    int eco2;
    int etvoc;
    double latitude;
    double longitude;
} Registro;

void limpar_registro(Registro *r) {
    r->id = 0;
    strcpy(r->device, "");
    r->contagem = 0;
    strcpy(r->data, "");
    r->temperatura = 0.0;
    r->umidade = 0.0;
    r->luminosidade = 0.0;
    r->ruido = 0.0;
    r->eco2 = 0;
    r->etvoc = 0;
    r->latitude = 0.0;
    r->longitude = 0.0;
}

Registro MapRegistro(char *linha)
{
    Registro r;
    limpar_registro(&r);  // Preenche os campos com valores "vazios"

    int campo = 0;
    char *token = strtok(linha, "|\n");
    while (token != NULL) {
        if (strlen(token) > 0) {
            switch (campo) {
                case 0: r.id = atoi(token); break;
                case 1: strcpy(r.device, token); break;
                case 2: r.contagem = atoi(token); break;
                case 3: strcpy(r.data, token); break;
                case 4: r.temperatura = atof(token); break;
                case 5: r.umidade = atof(token); break;
                case 6: r.luminosidade = atof(token); break;
                case 7: r.ruido = atof(token); break;
                case 8: r.eco2 = atoi(token); break;
                case 9: r.etvoc = atoi(token); break;
                case 10: r.latitude = atof(token); break;
                case 11: r.longitude = atof(token); break;
            }
        }
        campo++;
        token = strtok(NULL, "|\n");
    }
    
    return r;
}

void salvar_registros_binario(const char *nome_arquivo, Registro *registros, size_t total) {

    char archive[30];
    sprintf(archive, "Data/%s", nome_arquivo);

    FILE *fp = fopen(archive, "wb");

    if (!fp) {
        perror("Erro ao criar arquivo binário");
        return;
    }

    for (size_t i = 0; i < total; i++) {
        fwrite(&registros[i].id, sizeof(int), 1, fp);
        fwrite(&registros[i].device, sizeof(char), TAM_DEVICE, fp);
        fwrite(&registros[i].contagem, sizeof(int), 1, fp);
        fwrite(&registros[i].data, sizeof(char), TAM_DATA, fp);
        fwrite(&registros[i].temperatura, sizeof(float), 1, fp);
        fwrite(&registros[i].umidade, sizeof(float), 1, fp);
        fwrite(&registros[i].luminosidade, sizeof(float), 1, fp);
        fwrite(&registros[i].ruido, sizeof(float), 1, fp);
        fwrite(&registros[i].eco2, sizeof(int), 1, fp);
        fwrite(&registros[i].etvoc, sizeof(int), 1, fp);
        fwrite(&registros[i].latitude, sizeof(double), 1, fp);
        fwrite(&registros[i].longitude, sizeof(double), 1, fp);
    }

    fclose(fp);
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

    for (int i = 0; i < param->start; i++) {
        if (fgets(linha, sizeof(linha), file) == NULL) {
            fprintf(stderr, "%d - Arquivo tem menos de %d linhas! %s\n",i, param->start, linha);
            fclose(file);
            return NULL;
        }
    }

    Registro *registros = malloc(param->nlinhas * sizeof(Registro));
    clock_t inicio = clock();

    int total = 0;

    for(total = 0; total < param->nlinhas; total++)
    {
        fgets(linha, sizeof(linha), file);
        registros[total] = MapRegistro(linha);
    }

    clock_t fim = clock();

    fclose(file);

    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
    printf("Thread %d: Tempo de execucao: %.4f segundos -> Numero de registros: %d\n", param->threadNum, tempo, total);

    char archiveName[30];
    sprintf(archiveName, "registros_%d.bin", param->threadNum);

    salvar_registros_binario(archiveName , registros, total);

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
