// compilación: g++ -std=c++17 -Wall -Wextra -pedantic laberinto.cpp -o laberinto.exe "-Wl,--stack,33554432"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <queue>
#include <utility>
using namespace std;

// nodo del arbol: una casilla con cualquier cantidad de hijos
class Node {
public:
	string value;
	Node *parent;
	vector<Node*> children;

	Node(string text) {
		parent = nullptr;
		value = text;
	}

	Node* add_child(Node* child) {
		// el padre es quien lo descubrió
		child -> parent = this;
		children.push_back(child);
		return child;
	}

	vector<Node*>* ancestros(Node* root);
};

// devuelve el camino desde este nodo hasta la raiz, los dos incluidos
// empieza en este nodo, por eso sirve para escribir de la meta hacia el inicio
vector<Node*>* Node::ancestros(Node* root) {
  vector<Node*>* result = new vector<Node*>;
  Node* explorer = this;
  while(explorer != root) {
    // guardo primero el nodo actual
    result -> push_back(explorer);
    // y subo al padre
    explorer = explorer -> parent;
    if(explorer == nullptr) return nullptr;
  }
  // al final meto la raiz
  result -> push_back(explorer);
  return result;
}

// datos del laberinto que comparten todas las funciones
int rows, columns;
vector<string> maze;
// abajo, izquierda, arriba, derecha: este orden nunca cambia
vector<vector<int>> neighbours = { {1,0},{0,-1},{-1,0},{0,1} };
// posiciones ya usadas
set<vector<int>> visited;
// se guarda el nodo de la meta cuando se crea
Node* goalNode = nullptr;

// revisa limites, paredes ('0') y celdas que ya visitamos
bool is_valid(int row, int column) {
	if(row < 0 || row >= rows || column < 0 || column >= columns) return false;
	if(maze[row][column] == '0') return false;
	if(visited.count({row, column}) > 0) return false;
	return true;
}

// dfs recursivo: marco la celda apenas entro y pruebo los 4 vecinos
void dfs(int row, int column, Node* node) {
	// se marca al entrar, no al salir
	visited.insert({row, column});
	for(vector<int> neighbor : neighbours) {
		int neighborRow = row + neighbor[0];
		int neighborColumn = column + neighbor[1];
		if (is_valid(neighborRow, neighborColumn)) {
			// el texto queda como (3,3), listo para copiar al archivo
			Node* child = new Node("(" + to_string(neighborRow) + "," + to_string(neighborColumn) + ")");
			node -> add_child(child);
			// guardo la meta
			if(maze[neighborRow][neighborColumn] == 'B') goalNode = child;
			// sigo por este hijo antes de los otros
			dfs(neighborRow, neighborColumn, child);
		}
	}
}

// arbol dfs con el inicio como raiz
Node* build_dfs(int startRow, int startColumn) {
	Node* root = new Node("(" + to_string(startRow) + "," + to_string(startColumn) + ")");
	dfs(startRow, startColumn, root);
	return root;
}

// arbol bfs con cola: se marca visitada al encolar, no al sacar
Node* build_bfs(int startRow, int startColumn) {
	Node* root = new Node("(" + to_string(startRow) + "," + to_string(startColumn) + ")");
	// cola de nodos del arbol
	queue<Node*> nodeQueue;
	// y su posicion, porque el nodo no guarda fila ni columna
	queue<vector<int>> positionQueue;
	nodeQueue.push(root);
	positionQueue.push({startRow, startColumn});
	visited.insert({startRow, startColumn});

	while(!nodeQueue.empty()) {
		Node* node = nodeQueue.front();
		nodeQueue.pop();
		// la posicion va junto con el nodo
		vector<int> position = positionQueue.front();
		positionQueue.pop();

		for(vector<int> neighbor : neighbours) {
			int neighborRow = position[0] + neighbor[0];
			int neighborColumn = position[1] + neighbor[1];
			if(is_valid(neighborRow, neighborColumn)) {
				Node* child = new Node("(" + to_string(neighborRow) + "," + to_string(neighborColumn) + ")");
				node -> add_child(child);
				if(maze[neighborRow][neighborColumn] == 'B') goalNode = child;
				// se marca al encolar
				visited.insert({neighborRow, neighborColumn});
				nodeQueue.push(child);
				positionQueue.push({neighborRow, neighborColumn});
			}
		}
	}
	return root;
}

// preorden: primero el nodo y despues cada hijo
void preorder(Node* node, vector<Node*>* result) {
	result -> push_back(node);
	for(Node* child : node -> children) preorder(child, result);
}

// postorden: primero los hijos y despues el nodo
void postorder(Node* node, vector<Node*>* result) {
  for(Node* child : node->children) {
    postorder(child, result);
  }
  result -> push_back(node);
}

// bfs sobre el arbol ya armado, sacando de la cola
void bfs(Node* root, vector<Node*>* result) {
	queue<Node*> nodeQueue;
	nodeQueue.push(root);
	while(!nodeQueue.empty()) {
		// saco y guardo en el mismo renglon
		Node* node = nodeQueue.front(); nodeQueue.pop();
		result -> push_back(node);
		for(Node* child : node -> children) nodeQueue.push(child);
	}
}

// escribe una lista de nodos en una sola linea, separados por coma
void write_file(string filename, vector<Node*>* path) {
	ofstream fout(filename);
	// para no poner coma antes del primero
	bool first = true;
	for(Node* node : *path) {
		if(!first) fout << ",";
		fout << node -> value;
		first = false;
	}
	// siempre termina con salto de linea
	fout << endl;
	fout.close();
}

int main() {
	for(int number = 1; number <= 6; number++) {
		string filename = "laberinto_" + to_string(number) + ".in";
		ifstream fin(filename);

		fin >> rows; fin >> columns;

		pair<int, int> start;
		pair<int, int> goal;

		maze.clear();
		string line;
		for(int row = 0; row < rows; row++) {
			fin >> line;
			maze.push_back(line);
			for(int column = 0; column < columns; column++) {
				// busco el inicio y la meta mientras leo, para no recorrer despues
				if(line[column] == 'A') {
					start.first = row;
					start.second = column;
				}
				else if(line[column] == 'B') {
					goal.first = row;
					goal.second = column;
				}
			}
		}
		fin.close();

		cout << "Laberinto " << number << endl;
		cout << "Inicio en (" << start.first << ", " << start.second << ")\n";
		cout << "Fin en    (" << goal.first << ", " << goal.second << ")\n";
		cout << endl;

		// nombre base de los 5 archivos
		string prefix = "laberinto_" + to_string(number);


		// DFS
		// se limpia para cada algoritmo
		visited.clear();
		// si no, queda la meta del laberinto anterior
		goalNode = nullptr;
		Node* dfsRoot = build_dfs(start.first, start.second);

		vector<Node*> dfsPreorder;
		preorder(dfsRoot, &dfsPreorder);
		write_file(prefix + "_dfs_preorden.out", &dfsPreorder);

		vector<Node*> dfsPostorder;
		postorder(dfsRoot, &dfsPostorder);
		write_file(prefix + "_dfs_postorden.out", &dfsPostorder);

		// viene de la meta hacia el inicio
		vector<Node*>* dfsSolution = goalNode -> ancestros(dfsRoot);
		write_file(prefix + "_dfs_solucion.out", dfsSolution);


		// BFS
		visited.clear();
		goalNode = nullptr;
		Node* bfsRoot = build_bfs(start.first, start.second);

		vector<Node*> bfsOrder;
		bfs(bfsRoot, &bfsOrder);
		write_file(prefix + "_bfs_recorrido.out", &bfsOrder);

		vector<Node*>* bfsSolution = goalNode -> ancestros(bfsRoot);
		write_file(prefix + "_bfs_solucion.out", bfsSolution);
	}
	return 0;
}