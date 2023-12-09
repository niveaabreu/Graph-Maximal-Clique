#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <stack>
#include <mpi.h>

using namespace std;

bool maiorQue(int a, int b) {
    return a < b;
}

vector<vector<int>> LeGrafo(const std::string& nomeArquivo, int& numVertices, int& numEdges) {
    ifstream arquivo(nomeArquivo);
    arquivo >> numVertices >> numEdges;

    vector<vector<int>> grafo(numVertices, vector<int>(numVertices, 0));

    for (int i = 0; i < numEdges; ++i) {
        int u, v;
        arquivo >> u >> v;
        grafo[u - 1][v - 1] = 1;
        grafo[v - 1][u - 1] = 1;  // O grafo é não direcionado
    }

    arquivo.close();

    return grafo;
}

bool ehClique(vector<int>& candidatos, vector<vector<int>>& grafo) {
    int n = candidatos.size();
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (grafo[candidatos[i]][candidatos[j]] == 0) {
                return false;
            }
        }
    }
    return true;
}

void EncontraCliques(vector<vector<int>>& grafo, vector<int>& candidatos, vector<int>& cliqueMaximal) {
    stack<pair<vector<int>, vector<int>>> pilhaCliques;

    for (size_t i = 0; i < candidatos.size(); ++i) {
        vector<int> cliqueAtual;
        vector<int> candidatosAtuais(candidatos.begin() + i + 1, candidatos.end());

        int v = candidatos[i];
        cliqueAtual.push_back(v);

        // Encontra todas as cliques maximais incluindo v
        vector<int> novosCandidatos;
        for (int u : candidatosAtuais) {
            if (grafo[v][u] == 1) {
                novosCandidatos.push_back(u);
            }
        }

        pilhaCliques.push({novosCandidatos, cliqueAtual});
    }

    while (!pilhaCliques.empty()) {
        auto [candidatos, cliqueAtual] = pilhaCliques.top();
        pilhaCliques.pop();

        if (candidatos.empty()) {
            if (cliqueAtual.size() > cliqueMaximal.size() && ehClique(cliqueAtual, grafo)) {
                cliqueMaximal = cliqueAtual;
            }
        } else {
            int v = candidatos.back();
            candidatos.pop_back();

            // Inclui o vértice v na clique atual
            cliqueAtual.push_back(v);

            // Encontra todas as cliques maximais incluindo v
            vector<int> novosCandidatos;
            for (int u : candidatos) {
                if (grafo[v][u] == 1) {
                    novosCandidatos.push_back(u);
                }
            }

            pilhaCliques.push({novosCandidatos, cliqueAtual});

            // Exclui o vértice v da clique atual
            cliqueAtual.pop_back();

            // Encontra todas as cliques maximais sem v
            pilhaCliques.push({candidatos, cliqueAtual});
        }
    }
}

vector<int> EncontraCliqueMaxima(vector<vector<int>>& grafo) {
    int numVertices = grafo.size();
    vector<int> candidatos;
    for (int i = 0; i < numVertices; ++i) {
        candidatos.push_back(i);
    }
    vector<int> cliqueMaximal;
    EncontraCliques(grafo, candidatos, cliqueMaximal);
    return cliqueMaximal;
}

void DistribuirSubgrafos(vector<vector<int>>& graph, int numVertices, int numProcessos) {
    int verticesPorProcesso = numVertices / numProcessos;
    int verticesExtras = numVertices % numProcessos;
    int offset = 0;

    for (int rank = 1; rank < numProcessos; ++rank) {
        int numVerticesEnviar = verticesPorProcesso + (rank < verticesExtras ? 1 : 0);
        MPI_Send(&numVerticesEnviar, 1, MPI_INT, rank, 0, MPI_COMM_WORLD);

        for (int i = 0; i < numVerticesEnviar; ++i) {
            MPI_Send(graph[offset + i].data(), numVertices, MPI_INT, rank, 0, MPI_COMM_WORLD);
        }

        offset += numVerticesEnviar;
    }
}

int main() {
    MPI_Init(NULL, NULL);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        int numVertices, numEdges;
        vector<vector<int>> graph = LeGrafo("grafo.txt", numVertices, numEdges);

        // Distribui subgrafos para outros processos
        DistribuirSubgrafos(graph, numVertices, size);

        // Processo 0 faz o trabalho
        vector<int> cliqueMaxima = EncontraCliqueMaxima(graph);
        sort(cliqueMaxima.begin(), cliqueMaxima.end(), maiorQue);

        // Envia tamanho da clique maximal para outros processos
        int cliqueMaximaSize = cliqueMaxima.size();
        MPI_Bcast(&cliqueMaximaSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Envia clique maximal para outros processos
        MPI_Bcast(cliqueMaxima.data(), cliqueMaximaSize, MPI_INT, 0, MPI_COMM_WORLD);

        // Recebe resultados dos outros processos
        for (int sourceRank = 1; sourceRank < size; ++sourceRank) {
            int size;
            MPI_Recv(&size, 1, MPI_INT, sourceRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            vector<int> partialResult(size);
            MPI_Recv(partialResult.data(), size, MPI_INT, sourceRank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (partialResult.size() > cliqueMaxima.size()) {
                cliqueMaxima = partialResult;
            }
        }

        cout << "MPI: " << cliqueMaxima.size() << " Clique Maximal: ";
        for (int v : cliqueMaxima) {
            cout << v + 1 << " ";
        }
        cout << endl;
    } else {
        // Outros processos recebem tamanho da clique maximal
        int cliqueMaximaSize;
        MPI_Bcast(&cliqueMaximaSize, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Recebe clique maximal
        vector<int> cliqueMaximal(cliqueMaximaSize);
        MPI_Bcast(cliqueMaximal.data(), cliqueMaximaSize, MPI_INT, 0, MPI_COMM_WORLD);

        // Encontra a clique maximal e envia de volta para o Processo 0
        MPI_Send(&cliqueMaximaSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(cliqueMaximal.data(), cliqueMaximaSize, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
