/*
Author: Yiting
LANG: C++
TASK: Sudoku solver 
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<algorithm>
#define S 3
#define SIZE 9
#define GRID SIZE*SIZE
#define ROWNUM SIZE*SIZE*SIZE
#define COLNUM GRID*4
#define OFFSET_ROW SIZE*SIZE
#define OFFSET_COL OFFSET_ROW*2
#define OFFSET_GRID OFFSET_ROW*3
#define TOTAL (ROWNUM+1)*COLNUM

using namespace std;

//pointers for each node, four direction: left, right, up and down
int lptr[TOTAL], rptr[TOTAL], uptr[TOTAL], dptr[TOTAL];
//the column and row a node belongs to
int colNode[TOTAL], rowNode[TOTAL];
//the number of nodes in a column, the first node in each row
int colNodeNum[COLNUM], rowNodeFirst[ROWNUM];
//the node number counter, the record number counter
//rec array stores the temp answers and res stores the final answer
int nodeNum, recordNum, rec[ROWNUM], res[ROWNUM];
//the initial problem
int problem[SIZE][SIZE];

FILE* fin = fopen("test.in", "r");
FILE* fout = fopen("test.out", "w");

/**
 * Initialize the header node for each column.
 */
void initHeadernodes() {
    int i;
    for (i = 0; i <= COLNUM; i++) {
        lptr[i] = i - 1, rptr[i] = i + 1, uptr[i] = i, dptr[i] = i;
        colNode[i] = i, rowNode[i] = 0;
        colNodeNum[i] = 0;
    }
    lptr[0] = COLNUM;
    rptr[COLNUM] = 0;
    nodeNum = COLNUM + 1;
}

/**
 * Insert nodes into the whole structure. Keep the whole structure a double circular linked list.
 * @param i
 * @param j
 */
void insertNode(int i, int j) {
    rowNode[nodeNum] = i;
    colNode[nodeNum] = j;
    if (rowNodeFirst[i]) {
        lptr[nodeNum] = lptr[rowNodeFirst[i]];
        rptr[nodeNum] = rowNodeFirst[i];
        lptr[rptr[nodeNum]] = nodeNum;
        rptr[lptr[nodeNum]] = nodeNum;
    } else {
        lptr[nodeNum] = nodeNum;
        rptr[nodeNum] = nodeNum;
        rowNodeFirst[i] = nodeNum;
    }
    uptr[nodeNum] = uptr[j];
    dptr[nodeNum] = j;
    uptr[dptr[nodeNum]] = nodeNum;
    dptr[uptr[nodeNum]] = nodeNum;
    colNodeNum[j]++;
    nodeNum++;
}

/**
 * For one single node, four positions have to be filled in order to satisfy the requirements of the exact cover problem.
 * @param i
 * @param j
 * @param k
 */
void addNodes(int i, int j, int k) {
    int row = i * GRID + j * SIZE + k + 1;
    insertNode(row, i * SIZE + j + 1); //each grid must have a number
    insertNode(row, i * SIZE + k + 1 + OFFSET_ROW); //each row must have distinct number 1-9
    insertNode(row, j * SIZE + k + 1 + OFFSET_COL); //each column must have distinct number 1-9
    insertNode(row, (i / S * S + j / S) * SIZE + k + 1 + OFFSET_GRID); //each small block must have distinct number 1-9
}

/**
 * Remove 'header' node first, then remove nodes in the row which are in the column which the header node lies.
 * Just reset the up and down pointers of the deleting nodes since the left and right points will be used later.
 * @param node
 */
void removeNodes(int node) {
    int i, j;
    lptr[rptr[node]] = lptr[node];
    rptr[lptr[node]] = rptr[node];
    for (i = dptr[node]; i != node; i = dptr[i]) {
        for (j = rptr[i]; j != i; j = rptr[j]) {
            uptr[dptr[j]] = uptr[j];
            dptr[uptr[j]] = dptr[j];
            colNodeNum[colNode[j]]--;
        }
    }
}

/**
 * Resume nodes, opposite to removeNodes function.
 * @param node
 */
void resumeNodes(int node) {
    int i, j;
    for (i = uptr[node]; i != node; i = uptr[i]) {
        for (j = lptr[i]; j != i; j = lptr[j]) {
            colNodeNum[colNode[j]]++;
            dptr[uptr[j]] = j;
            uptr[dptr[j]] = j;
        }
    }
    rptr[lptr[node]] = node;
    lptr[rptr[node]] = node;
}

/**
 * Add nodes according to the initial problem.
 * For each position if there is already a number, add node according to that number.
 * If there isn't a number yet, add nodes according to all possible numbers (1-9).
 */
void readyToAdd() {
    int i, j, k;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            if (problem[i][j] != 0) {
                addNodes(i, j, problem[i][j] - 1);
            } else {
                for (k = 0; k < SIZE; k++)
                    addNodes(i, j, k);
            }
        }
    }
}

/**
 * A DFS search. Choose the column with least number of nodes, then remove this column.
 * Then iterate rows which has node in the removed column, remove these rows.
 * For each of these removed rows, find out columns which have node in those rows,
 * then remove the rows which has node in those columns we just find out.
 * Then do the next iteration, until no more nodes in the structure.
 * @param depth
 * @return 
 */
bool dfs(int depth) {
    if (!rptr[0]) {
        recordNum = depth;
        return true;
    }
    int i, j, k, count = TOTAL, current;
    for (i = rptr[0]; i; i = rptr[i]) {
        if (colNodeNum[i] < count) {
            count = colNodeNum[i];
            current = i;
            if (count == 1)
                break;
        }
    }
    removeNodes(current);
    for (i = dptr[current]; i != current; i = dptr[i]) {
        for (j = rptr[i]; j != i; j = rptr[j]) {
            removeNodes(colNode[j]);
        }
        rec[depth] = rowNode[i];
        if (dfs(depth + 1)) {
            return true;
        }
        for (k = lptr[i]; k != i; k = lptr[k]) {
            resumeNodes(colNode[k]);
        }
    }
    resumeNodes(current);
    return false;
}

void output() {
    int i, j;
    for (i = 0; i < recordNum; i++)
        res[(rec[i] - 1) / SIZE] = (rec[i] - 1) % SIZE + 1;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++)
            fprintf(fout, "%d ", res[i * SIZE + j]);
        fprintf(fout, "\n");
    }
}

main() {
    int i, j;
    for (i = 0; i < SIZE; i++)
        for (j = 0; j < SIZE; j++)
            fscanf(fin, "%d", &problem[i][j]);
    initHeadernodes();
    readyToAdd();
    dfs(0);
    output();
    exit(0);
}
