# FSO-pthreads

Este projeto implementa um processador de dados de sensores IoT utilizando `pthreads` para paralelizar a análise de um grande volume de dados armazenados em um arquivo CSV. A aplicação foi desenvolvida em linguagem C e roda em ambiente Windows.

## Objetivo

Distribuir a carga de processamento de registros de sensores entre múltiplos processadores usando threads, gerando estatísticas mensais (mínimo, máximo, média) por sensor e por dispositivo.

---

## Funcionalidades

- Leitura de arquivo CSV com registros IoT.
- Filtragem de dados a partir de março de 2024.
- Processamento paralelo de registros com número de threads definido automaticamente com base nos núcleos de CPU disponíveis.
- Geração de arquivo `resultado.csv` contendo:
  - Dispositivo
  - Ano-mês
  - Tipo de sensor
  - Valor máximo, médio e mínimo
  - Quantidade de registros
- Geração de arquivos individuais por dispositivo com as mesmas informações.

---

## Estrutura do CSV de entrada

Formato delimitado por pipe (`|`), com as seguintes colunas:

```
id|device|contagem|data|temperatura|umidade|luminosidade|ruido|eco2|etvoc|latitude|longitude
```

As análises consideram apenas os sensores a partir de março de 2024 e excluem colunas como `id`, `latitude` e `longitude`.

---

## Como compilar

Este projeto é voltado ao ambiente Windows com suporte a `pthreads`. Utilize MinGW com suporte à POSIX Threads.

1. Instale o MinGW com suporte a `pthreads-w32`.
2. Compile com:

```sh
gcc -o analisador.exe programa.c -lpthread
```

---

## Como executar

```sh
./analisador.exe
```

O programa espera o arquivo CSV em `./Data/devices.csv` e salva os resultados em `./Data/output/`.

---

## Estrutura dos Arquivos de Saída

- `./Data/output/resultado.csv`: arquivo geral com todos os dispositivos.
- `./Data/output/<device>.csv`: arquivos individuais para cada dispositivo.

Formato dos arquivos:

```
device|ano-mes|sensor|valor_maximo|valor_medio|valor_minimo|quantidade_registros
```

---

## Funcionamento Interno

### Leitura do CSV

- O programa abre o arquivo `devices.csv`.
- Ignora o cabeçalho.
- Carrega todos os registros a partir de março de 2024.
- Os dados são armazenados em uma estrutura `Registro`.

### Distribuição entre Threads

- O número de threads é automaticamente definido pelo número de núcleos lógicos disponíveis (`GetSystemInfo()`).
- O conjunto de registros é dividido igualmente entre as threads por índice (faixa de registros), **não por dispositivo nem por data**.
- Cada thread processa um intervalo do vetor de registros global.

### Processamento por Thread

Cada thread:
- Para cada registro:
  - Localiza ou cria a entrada correspondente no vetor `resultados`.
  - Atualiza:
    - Valor mínimo e máximo de cada sensor.
    - Soma para posterior cálculo de média.
    - Contador de registros.
- O acesso ao vetor `resultados` é protegido por mutex para evitar condições de corrida.

### Escrita dos Resultados

- Após o término das threads, a thread principal salva os resultados:
  - Um arquivo geral com todos os dados (`resultado.csv`).
  - Um arquivo por dispositivo (`<device>.csv`).

---

## Detalhes das Threads

- As threads criadas são **threads em modo usuário** utilizando a biblioteca POSIX Threads (pthreads).
- Em ambientes Windows, o controle de scheduling ainda depende do sistema operacional, mas elas não são threads em modo núcleo explícito.

---

## Concorrência e Sincronização

- O vetor `resultados` é compartilhado entre threads.
- Para evitar **condições de corrida**, o acesso e modificação de entradas neste vetor são protegidos por um `pthread_mutex_t`.
- Isso garante que apenas uma thread por vez possa verificar/criar/atualizar um resultado por dispositivo/mês.

---

## Pontos de Atenção

- O programa atualmente só implementa a versão para Windows. O bloco de código `#elif defined(__linux__)` está vazio.
