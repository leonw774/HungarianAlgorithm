#include <ctime>
#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <type_traits>

using namespace std;

template <typename T, typename enable_if<is_arithmetic<T>::value, T>::type = true>
class Hungarian {
    size_t size, _xsize, _ysize;
    vector<vector<T>> w;         // the weights
    vector<vector<bool>> noEdge; // record which zero weight is no edge and which is just zero

    vector<int> xMatch, yMatch;  // the matching node on the other partition
    vector<T> xLabel, yLabel;    // label of X and Y nodes
    vector<bool> xVisited, yVisited;

    // record the augmenting tree for bfs
    // (prev[j] == i) means that the j-th node in the y partition is pointed by the i-th node in X
    // the nodes in y partition can only point to its matching node so we don't have to reocrd it
    vector<int> prev;

    // yDelta[y] = min( l(x) - l(y) - w(x, y) for (x, y) in E )
    vector<T> yDelta;

public:
    Hungarian(vector<vector<T>>& _w, bool perfect = true) {
        _xsize = _w.size(), _ysize = _w[0].size();
        size = _xsize > _ysize ? _xsize : _ysize;
        // pad smaller partition with node without edges with zero weight edges
        // zero weight edge effectively means no edge when finding maximum weight
        w = vector<vector<T>>(size, vector<T>(size, 0));
        noEdge = vector<vector<bool>>(size, vector<bool>(size));
        for (int i = 0; i < _xsize; i++) {
            for (int j = 0; j < _ysize; j++) {
                if (perfect) {
                    w[i][j] = _w[i][j];
                }
                else {
                    // don't need to be perfect == can discard all negtive edges
                    w[i][j] = max(0, _w[i][j]);
                    if (_w[i][j] < 0) {
                        noEdge[i][j] = 1;
                    }
                }
            }
        }
        // init vectors
        xMatch = vector<int>(size, -1);
        yMatch = vector<int>(size, -1);
        xLabel = vector<T>(size);
        yLabel = vector<T>(size);
        xVisited = vector<bool>(size);
        yVisited = vector<bool>(size);
        prev = vector<int>(size);
        yDelta = vector<T>(size, numeric_limits<T>::max());
    }

    vector<pair<int, int>> findMaxWeightMatch() {           
        // initialization
        fill(xMatch.begin(), xMatch.end(), -1);
        fill(yMatch.begin(), yMatch.end(), -1);
        fill(xLabel.begin(), xLabel.end(), 0);
        fill(yLabel.begin(), yLabel.end(), 0);
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                xLabel[i] = max(xLabel[i], w[i][j]);
            }
        }
        // outer loop for each nodes in X
        for (int i = 0; i < size; i++) {
            // bool success = findAugPathDFS(i);
            bool success = findAugPathBFS(i);
            if (!success) {
                // failed, return empty result
                return vector<pair<int, int>>();
            }
        }
        
        // use the final matching result
        vector<pair<int, int>> res;
        for (int i = 0; i < _xsize; i++) {
            if (xMatch[i] < _ysize && !noEdge[i][xMatch[i]]) {
                res.push_back({i, xMatch[i]});
            }
        }
        return res;
    }

private:
    // try extend the augmenting path from the x-th node in X
    bool dfs(int x) {
        xVisited[x] = 1;
        for (int j = 0; j < size; j++) {
            if (yVisited[j]) continue;
            // if is equality edge
            if (xLabel[x] + yLabel[j] == w[x][j]) {
                yVisited[j] = 1;
                // success found an unmatched Y node or the following search found one
                if (yMatch[j] == -1 || dfs(yMatch[j])) {
                    // match the Y nodes to its prev X nodes
                    // ... x0 ---> j0 --(m)-> x1 ---> j1 ...
                    yMatch[j] = x, xMatch[x] = j;
                    // ... x0 ---> j0 ---> x1 --(m)-> j1
                    return true;
                    // ... x0 --(m)-> j0 ---> x1 --(m)-> j1 ...
                }
            }
        }
        return false;
    }

private:
    // find the augmenting path starting from x-th node in X
    // using depth-first search
    bool findAugPathDFS(int x) {
        int t = 0;
        while (t < size) {
            t++;

            fill(xVisited.begin(), xVisited.end(), 0);
            fill(yVisited.begin(), yVisited.end(), 0);

            if (dfs(x)) {
                // found augmenting path
                break;
            }

            // update label and try again
            T delta = numeric_limits<T>::max();
            for (int i = 0; i < size; i++) {
                if (!xVisited[i]) continue; // visited X
                for (int j = 0; j < size; j++) {
                    if (yVisited[j]) continue; // unvisited Y
                    delta = min(delta, xLabel[i] + yLabel[j] - w[i][j]);
                }
            }
            if (delta == 0) {
                break;
            }
            if (delta == numeric_limits<T>::max()) {
                return false;
            }
            for (int k = 0; k < size; k++) {
                if (xVisited[k]) xLabel[k] -= delta;
                if (yVisited[k]) yLabel[k] += delta;
            }
        }
        return true;
    }

    // find the augmenting path starting from x-th node in X
    // using breadth-first search
    bool findAugPathBFS(int x) {
        int t = 0;
        while (t < size) {
            t++;

            fill(xVisited.begin(), xVisited.end(), 0);
            fill(yVisited.begin(), yVisited.end(), 0);
            fill(prev.begin(), prev.end(), -1);
            fill(yDelta.begin(), yDelta.end(), numeric_limits<T>::max());
            
            queue<int> q;

            xVisited[x] = 1;
            q.push(x);
            // extend the tree
            while (!q.empty()) {
                int curx = q.front();
                // cout << "curx " << curx << endl;
                q.pop();
                for (int j = 0; j < size; j++) {
                    // cout << "  j " << j << endl; 
                    if (yVisited[j]) continue;

                    // slack of j to curx
                    T slack = xLabel[curx] + yLabel[j] - w[curx][j];

                    // if is equality edge
                    if (slack == 0) {
                        yVisited[j] = 1;
                        prev[j] = curx;
                        if (yMatch[j] == -1) {
                            // found augmenting path
                            // match Y nodes to its prev X nodes
                            int y = j;
                            while (y != -1) {
                                int x = prev[y];
                                yMatch[y] = x;
                                int prev_y = xMatch[x];
                                xMatch[x] = y;
                                y = prev_y;
                            }
                            // leave function
                            return true;
                        }
                        else {
                            // add this Y node's matching X node into queue
                            q.push(yMatch[j]);
                            xVisited[yMatch[j]] = 1;
                        }
                    }
                    else if (slack < yDelta[j]) {
                        // not a equality edge: update the j's delta
                        yDelta[j] = slack;
                    }
                }
            }
            // update label and try again
            T delta = numeric_limits<T>::max();
            for (int j = 0; j < size; j++) {
                delta = min(delta, yDelta[j]);
            }
            if (delta == 0) {
                break;
            }
            if (delta == numeric_limits<T>::max()) {
                return false;
            }
            for (int k = 0; k < size; k++) {
                if (xVisited[k]) xLabel[k] -= delta;
                if (yVisited[k]) yLabel[k] += delta;
            }
        }
        return false;
    }
};

int main() {
    // make test case
    int testSize = 1024;
    int maxRand = 256;
    srand(time(NULL));
    vector<vector<int>> testWeight = vector<vector<int>>(testSize, vector<int>(testSize));
    for (int i = 0; i < testSize; i++) {
        for (int j = 0; j < testSize; j++) {
            testWeight[i][j] = rand() % maxRand;
        }
    }

    int maxWeight = 0;
    Hungarian<int> h(testWeight);
    vector<pair<int, int>> match = h.findMaxWeightMatch();
    for (pair<int, int>& m: match) {
        maxWeight += testWeight[m.first][m.second];
        // cout << "(" << m.first << "," << m.second << ") ";
    }
    cout << "max weight: " << maxWeight << endl;
    return 0;
}