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

int main() {
    MPI_Init(NULL, NULL);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        int numVertices, numEdges;
        vector<vector<int>> graph = LeGrafo("grafo.txt", numVertices, numEdges);

        // Distribui informações do grafo para outros processos
        for (int destRank = 1; destRank < size; ++destRank) {
            MPI_Send(&numVertices, 1, MPI_INT, destRank, 0, MPI_COMM_WORLD);
            MPI_Send(&numEdges, 1, MPI_INT, destRank, 0, MPI_COMM_WORLD);
            for (int i = 0; i < numVertices; ++i) {
                MPI_Send(graph[i].data(), numVertices, MPI_INT, destRank, 0, MPI_COMM_WORLD);
            }
        }

        // Processo 0 faz o trabalho
        vector<int> cliqueMaxima = EncontraCliqueMaxima(graph);
        sort(cliqueMaxima.begin(), cliqueMaxima.end(), maiorQue);

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
        // Outros processos fazem o trabalho
        int numVertices, numEdges;
        MPI_Recv(&numVertices, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&numEdges, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        vector<vector<int>> graph(numVertices, vector<int>(numVertices));
        for (int i = 0; i < numVertices; ++i) {
            MPI_Recv(graph[i].data(), numVertices, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        vector<int> candidatos;
        for (int i = 0; i < numVertices; ++i) {
            candidatos.push_back(i);
        }

        vector<int> cliqueMaximal;
        EncontraCliques(graph, candidatos, cliqueMaximal);

        // Envia resultado de volta para o Processo 0
        int tamanho = cliqueMaximal.size();
        MPI_Send(&tamanho, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(cliqueMaximal.data(), cliqueMaximal.size(), MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
