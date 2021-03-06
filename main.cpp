#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <chrono>

#include "./include/Arquivo.hpp"
#include "./include/sortalgos.hpp"
#include "./include/utils.hpp"

#define ERR_NENHUM_ARG 1
#define ERR_FALHA_ARQ  2
#define M 5 // numero de cojuntos
#define N 5 // numero de registros aleatorios

#define N_ALGORITMOS 4 // numero de algoritmos de ordenacao
#define HEAPSORT  0
#define QUICKSORT 1
#define MERGESORT 2
#define TIMSORT   3


int main(int argc, char *argv[])
{
	const char *progname = *argv;

	if (argc < 2) 
	{
		std::cerr << progname << ": nenhum arquivo fornecido\n";
		exit(ERR_NENHUM_ARG);
	}

	while (--argc) 
	{	
		std::ifstream fin;
		fin.open(*++argv);

		if (!fin.is_open()) 
		{
			std::cerr << progname << ": falha ao abrir o arquivo `" << *argv << "`\n";
			exit(ERR_FALHA_ARQ);
		}

		std::vector<Registro> vetRegistros;
		Arquivo::lerRegistros(fin, vetRegistros);
		fin.close();

		// Pre-processamento
		int comparacoes = 0;
		int trocas = 0;
		heapSort(vetRegistros, comparacoes, trocas, Registro::comparaLocalData);
		calculaTotalDiarios(vetRegistros);

		std::ofstream fout;
		fout.open("brazil_covid19_cities_processado.csv");
		if (fout.is_open()) {
			Arquivo::escreverRegistros(vetRegistros, fout);
		}
		else {
			std::cout << progname << ": falha ao escrever arquivo `brazil_covid19_cities_processado.csv`\n";
		}
		
		fout.close();

		// matrizes 3d de valores
		int matrizComp[N_ALGORITMOS][N][M] = {};
		int matrizTrocas[N_ALGORITMOS][N][M] = {};
		
		std::chrono::steady_clock::time_point begin;
		std::chrono::steady_clock::time_point end;
		int matrizTempos[N_ALGORITMOS][N][M] = {};

		// matrizes 2d de médias
		int mediasComp[N_ALGORITMOS][N] = {};
		int stdComp[N_ALGORITMOS][N] = {};
		int mediasTrocas[N_ALGORITMOS][N] = {};
		int stdTrocas[N_ALGORITMOS][N] = {};
		int mediasTempos[N_ALGORITMOS][N] = {};
		int stdTempos[N_ALGORITMOS][N] = {};

		// Ordenacao
		int arr[] = {10000, 50000, 100000, 500000, 1000000};
		for(size_t i = 0; i < N; ++i) {
			for(size_t j = 0; j < M; ++j) {
				// Obter N elementos aleatorios -> Registro::nAleatorios(vetRegistros, N=10k..50k..100k..etc..);
				
				std::vector<Registro> vetControle = Registro::nAleatorios(vetRegistros, arr[i]);
				
				// Criar copia a ser ordenada
				std::vector<Registro> vet = vetControle;
				
				begin = std::chrono::steady_clock::now();
				heapSort(vet, matrizComp[HEAPSORT][i][j], matrizTrocas[HEAPSORT][i][j], Registro::comparaCasos);
				end = std::chrono::steady_clock::now();
				matrizTempos[HEAPSORT][i][j] = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

				// Recuperar ordem inicial dos elementos
				vet = vetControle;
				begin = std::chrono::steady_clock::now();
				quickSort(vet, 0, vet.size()-1, matrizComp[QUICKSORT][i][j], matrizTrocas[QUICKSORT][i][j], Registro::comparaCasos);
				end = std::chrono::steady_clock::now();
				matrizTempos[QUICKSORT][i][j] = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
				
				if(i < 3){
					//Tim Sort para apenas 100k
					vet = vetControle;
					begin = std::chrono::steady_clock::now();
					timSort(vet.begin(), vet.end(), matrizComp[TIMSORT][i][j], matrizTrocas[TIMSORT][i][j], Registro::comparaCasos);
					end = std::chrono::steady_clock::now();
					matrizTempos[TIMSORT][i][j] = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
				}
				
				/*
				vet = vetControle;
				begin = std::chrono::steady_clock::now();
				heapSort(vet, matrizComp[HEAPSORT][i][j], matrizTrocas[HEAPSORT][i][j], Registro::comparaCasos);
				end = std::chrono::steady_clock::now();
				matrizTempos[0][i][j] = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
				*/
			}

			// Calcular medias e desvio padrao para cada algoritmo
			for(size_t codigo = 0; codigo < N_ALGORITMOS; ++codigo)
			{
				// Media de comparacoes
				// Passa para a função um vetor 1d pertencente à matriz 3d de valores
				// Guarda a média do vetor em uma posição na matriz 2d de médias
				mediasComp[codigo][i] = calculaMedia(matrizComp[codigo][i], M);
				stdComp[codigo][i] = pow(calculaVariancia(matrizComp[codigo][i], M, mediasComp[codigo][i]), 0.5);
				// Media de trocas
				mediasTrocas[codigo][i] = calculaMedia(matrizTrocas[codigo][i], M);
				stdTrocas[codigo][i] = pow(calculaVariancia(matrizTrocas[codigo][i], M, mediasTrocas[codigo][i]), 0.5);
				// Media do tempo
				mediasTempos[codigo][i] = calculaMedia(matrizTempos[codigo][i], M);
				stdTempos[codigo][i] = pow(calculaVariancia(matrizTempos[codigo][i], M, mediasTempos[codigo][i]), 0.5);
			}
		}

		std::ofstream relatorio;
		relatorio.open("saida.txt");
		if(!relatorio.fail())
			Arquivo::escreverTXT(relatorio, arr, matrizComp, matrizTrocas, matrizTempos, mediasComp, 
			mediasTrocas, mediasTempos, stdComp, stdTrocas, stdTempos);
		else
			std::cout << progname << ": falha ao escrever arquivo `saida.txt`\n";
		relatorio.close();

		// Salvar resultados para leitura com Pandas (Python)
		relatorio.open("saida.csv");
		if(!relatorio.fail())
			Arquivo::escreverCSV(relatorio, arr, matrizComp, matrizTrocas, matrizTempos);
		else
			std::cout << progname << ": falha ao escrever arquivo `saida.csv`\n";
		relatorio.close();
		
	}

	return 0;
}