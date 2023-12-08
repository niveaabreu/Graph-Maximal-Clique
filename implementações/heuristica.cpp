#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

struct GrauVertice {
    int vertice;
    int grau;

    bool operator<(const GrauVertice& outro) const {
        return grau > outro.grau; // Ordena em ordem decrescente de grau
    }
};

vector<vector<int>> LerGrafo(const string& nomeArquivo, int& numVertices) {
    ifstream arquivo(nomeArquivo);
    int numArestas;
    arquivo >> numVertices >> numArestas;

    vector<vector<int>> grafo(numVertices, vector<int>(numVertices, 0));

    for (int i = 0; i < numArestas; ++i) {
        int u, v;
        arquivo >> u >> v;
        grafo[u - 1][v - 1] = 1;
        grafo[v - 1][u - 1] = 1;  // O grafo é não direcionado
    }

    arquivo.close();

    return grafo;
}

vector<int> CliqueMaxima(vector<vector<int>>& grafo, int numVertices) {
    vector<GrauVertice> graus;

    // Calcula o grau de cada vértice
    for (int i = 0; i < numVertices; ++i) {
        int grau = 0;
        for (int j = 0; j < numVertices; ++j) {
            if (grafo[i][j] == 1) {
                grau++;
            }
        }
        graus.push_back({i, grau});
    }

    // Ordena os vértices por grau de adjacência
    sort(graus.begin(), graus.end());

    vector<int> cliqueMaxima;
    vector<int> candidatos;

    for (const auto& vertice : graus) {
        candidatos.push_back(vertice.vertice);
    }

    while (!candidatos.empty()) {
        int v = candidatos.back();
        candidatos.pop_back();
        bool podeAdicionar = true;

        for (size_t i = 0; i < cliqueMaxima.size(); ++i) {
            int u = cliqueMaxima[i];
            if (grafo[u][v] == 0) {
                podeAdicionar = false;
                break;
            }
        }

        if (podeAdicionar) {
            cliqueMaxima.push_back(v);
            vector<int> novosCandidatos;

            for (size_t i = 0; i < candidatos.size(); ++i) {
                int u = candidatos[i];
                bool adjacenteATodos = true;

                for (size_t j = 0; j < cliqueMaxima.size(); ++j) {
                    int c = cliqueMaxima[j];
                    if (grafo[u][c] == 0) {
                        adjacenteATodos = false;
                        break;
                    }
                }

                if (adjacenteATodos) {
                    novosCandidatos.push_back(u);
                }
            }

            candidatos = novosCandidatos;
        }
    }

    return cliqueMaxima;
}

int main() {
    vector<vector<int>> grafo;
    int numVertices;
    grafo = LerGrafo("grafo.txt", numVertices);
    vector<int> cliqueMaxima = CliqueMaxima(grafo, numVertices);

    cout << "Clique Máxima (Heurística): " << cliqueMaxima.size();
    cout << " Vértices da Clique Máxima:" << endl;
    for (int elemento : cliqueMaxima) {
        cout << elemento + 1 << " "; // Adicionando 1 para representar o índice real do vértice
    }

    return 0;
}
