#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <windows.h>
#include <ctype.h>
#include <sys/stat.h>
#include <direct.h>

#define MAX_LINE 512
#define MAX_RECORDS 7000000
#define MAX_DEVICE_LEN 64
#define NUM_SENSORS 6
#define OUTPUT_DIR "./Data/output/"
#define OUTPUT_FILE "resultado.csv"

const char *sensor_names[NUM_SENSORS] = {"temperatura", "umidade", "luminosidade", "ruido", "eco2", "etvoc"};

typedef struct {
    char device[MAX_DEVICE_LEN];
    int year, month;
    float sensors[NUM_SENSORS];
} Registro;

typedef struct {
    char device[MAX_DEVICE_LEN];
    int year, month;
    float min[NUM_SENSORS];
    float max[NUM_SENSORS];
    float sum[NUM_SENSORS];
    int count;
} Resultado;

Registro *registros;
int total_registros = 0;
int num_threads;
Resultado *resultados;
int resultado_count = 0;
pthread_mutex_t resultado_mutex;

int get_num_cores() {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

int parse_data(const char *data, int *year, int *month) {
    return sscanf(data, "%d-%d", year, month);
}

void carregar_csv(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("Erro ao abrir CSV");
        exit(1);
    }
    registros = malloc(sizeof(Registro) * MAX_RECORDS);
    char linha[MAX_LINE];
    int linha_count = 0;

    while (fgets(linha, MAX_LINE, file)) {
        linha_count++;
        if (linha_count == 1) continue; // pular o cabeçalho

        char *token = strtok(linha, "|");
        int col = 0;
        Registro r;
        int incluir = 0;
        memset(&r, 0, sizeof(Registro));

        while (token) {
                switch (col) {
                    case 1: strncpy(r.device, token, MAX_DEVICE_LEN); break;
                    case 3:
                        if (parse_data(token, &r.year, &r.month) &&
                            (r.year > 2024 || (r.year == 2024 && r.month >= 3))) {
                            incluir = 1;
                        }
                        break;
                    case 4: r.sensors[0] = atof(token); break;
                    case 5: r.sensors[1] = atof(token); break;
                    case 6: r.sensors[2] = atof(token); break;
                    case 7: r.sensors[3] = atof(token); break;
                    case 8: r.sensors[4] = atof(token); break;
                    case 9: r.sensors[5] = atof(token); break;
            }
            token = strtok(NULL, "|");
            col++;
        }

        if (incluir && strlen(r.device) > 0) {
            registros[total_registros++] = r;
        }
    }
    fclose(file);
    printf("Total de registros carregados: %d\n", total_registros);
}

// Verifica se resultado existe e retorna índice, ou -1
int encontrar_resultado(const char *device, int year, int month) {
    for (int i = 0; i < resultado_count; i++) {
        if (strcmp(resultados[i].device, device) == 0 &&
            resultados[i].year == year &&
            resultados[i].month == month) {
            return i;
        }
    }
    return -1;
}

void *processar(void *arg) {
    clock_t clock_inicio = clock();

    int tid = *(int *)arg;

    int blocos = total_registros / num_threads;
    int inicio = tid * blocos;
    int fim = (tid == num_threads - 1) ? total_registros : inicio + blocos;

    printf("Thread %d processando de %d a %d\n", tid, inicio, fim);

    for (int i = inicio; i < fim; i++) {
        Registro *r = &registros[i];
        pthread_mutex_lock(&resultado_mutex);

        int idx = encontrar_resultado(r->device, r->year, r->month);

        if (idx == -1) {
            idx = resultado_count++;
            strncpy(resultados[idx].device, r->device, MAX_DEVICE_LEN);
            resultados[idx].year = r->year;
            resultados[idx].month = r->month;
            resultados[idx].count = 0;

            for (int j = 0; j < NUM_SENSORS; j++) {
                resultados[idx].min[j] = r->sensors[j];
                resultados[idx].max[j] = r->sensors[j];
                resultados[idx].sum[j] = 0;
            }
        }

        for (int j = 0; j < NUM_SENSORS; j++) {
            if (r->sensors[j] < resultados[idx].min[j]) resultados[idx].min[j] = r->sensors[j];
            if (r->sensors[j] > resultados[idx].max[j]) resultados[idx].max[j] = r->sensors[j];
            resultados[idx].sum[j] += r->sensors[j];
        }
        resultados[idx].count++;
        pthread_mutex_unlock(&resultado_mutex);
    }

    clock_t clock_fim = clock();

    double tempo = (double)(clock_fim - clock_inicio) / CLOCKS_PER_SEC;
    printf("Thread %d: Tempo de execucao: %.4f segundos\n", tid, tempo);

    return NULL;
}

void salvar_csvs() {
    mkdir("./Data");
    mkdir(OUTPUT_DIR);
    FILE *out = fopen(OUTPUT_FILE, "w");
    
    if (!out) {
        perror("Erro ao criar arquivo resultado.csv");
        exit(1);
    }
    fprintf(out, "device|ano-mes|sensor|valor_maximo|valor_medio|valor_minimo|quantidade_registros\n");

    for (int i = 0; i < resultado_count; i++) {
        Resultado *res = &resultados[i];
        char filename[256];
        snprintf(filename, sizeof(filename), "%s%s.csv", OUTPUT_DIR, res->device);

        FILE *devfile = fopen(filename, "a");
        if (!devfile) {
            perror("Erro ao criar arquivo por dispositivo");
            continue;
        }

        for (int j = 0; j < NUM_SENSORS; j++) {
            float media = res->sum[j] / res->count;

            fprintf(out, "%s|%04d-%02d|%s|%.2f|%.2f|%.2f|%d\n",
                    res->device, res->year, res->month, sensor_names[j],
                    res->max[j], media, res->min[j], res->count);

            fprintf(devfile, "%04d-%02d|%s|%.2f|%.2f|%.2f|%d\n",
                    res->year, res->month, sensor_names[j],
                    res->max[j], media, res->min[j], res->count);
        }
        fclose(devfile);
    }
    fclose(out);

    printf("Arquivo resultado.csv e arquivos por dispositivo foram gerados na pasta %s\n", OUTPUT_DIR);
}

void executeInWin32()
{
    num_threads = get_num_cores();
    
    printf("Utilizando %d threads.\n", num_threads);

    carregar_csv("./Data/devices.csv");

    pthread_t threads[num_threads];
    int thread_ids[num_threads];

    resultados = malloc(sizeof(Resultado) * total_registros); // superestimado, mas seguro
    pthread_mutex_init(&resultado_mutex, NULL);

    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, processar, &thread_ids[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    salvar_csvs();

    free(registros);
    free(resultados);

    pthread_mutex_destroy(&resultado_mutex);

    return;
}

int main() {
    printf("Iniciando processamento...\n");

    #if defined(_WIN32)
    	executeInWin32();
    #elif defined(__linux__)
    #endif

    printf("Processamento finalizado.\n");
    return 0;
}
