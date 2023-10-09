// Função para ler o grafo a partir do arquivo de entrada
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

using namespace std;

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

vector<int> CliqueMaxima(vector<vector<int>> &grafo, int numeroVertices){
    vector<int> cliqueMaxima;
    vector<int> candidatos;
    for (int i = 0; i < numVertices; i++){
        candidatos.push_back(i);
    }

    while (candidatos.size()>0){
        v=candidatos[candidatos.size()-1];
        candidatos.pop_back();
        bool podeAdicionar=true;

        for(int i = 0; u < cliqueMaxima.size(); i++){
            int u = cliqueMaxima[i];
            if(grafo[u][v]==0){
                podeAdicionar=false;
                break;
            }
        }
        if (podeAdicionar){
            cliqueMaxima.push_back(v);
            vector<int> novosCandidatos;

            for(int i = 0; i < candidatos.size(); i++){
                int u = candidatos[i];
                bool adjacenteATodos=true;

                for(int j = 0; j < cliqueMaxima.size(); j++){
                    int c = cliqueMaxima[j];
                    if(grafo[u][c]==0){
                        adjacenteATodos=false;
                        break;
                    }
                }
                if(adjacenteATodos){
                    novosCandidatos.push_back(u);
                }
            }
            candidatos=novosCandidatos;
        }
       
    }
    return cliqueMaxima;
}

int main(){
    vector<vector<int>> grafo;
    int numVertices;
    grafo = LerGrafo("grafo.txt", int& numVertices);
    vector<int> cliquemaxima = CliqueMaxima(grafo,numVertices);
    for(int elemento:cliquemaxima){
        cout << elemento << " ";
    }
    return 0;
}
