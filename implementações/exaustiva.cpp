#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <stack>

using namespace std;

bool maiorQue(int a, int b) {
    return a < b;
}

vector<vector<int>> LeGrafo(const std::string& nomeArquivo, int& numVertices) {
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

    for (int i = 0; i < candidatos.size(); ++i) {
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
    int numVertices;
    vector<vector<int>> grafo;

    grafo = LeGrafo("grafo.txt", numVertices);
    vector<int> cliqueMaxima = EncontraCliqueMaxima(grafo);
    sort(cliqueMaxima.begin(), cliqueMaxima.end(), maiorQue);
    cout << "EXAUSTIVA: " << cliqueMaxima.size() << " Clique Maximal: ";
    for (int v : cliqueMaxima) {
        cout << v + 1 << " ";
    }
    cout << endl;

    return 0;
}
