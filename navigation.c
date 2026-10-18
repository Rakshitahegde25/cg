#include "navigation.h"
#include <windows.h>
#include <GL/gl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

Vec3 currentPath[MAX_PATH_WAYPOINTS];
int currentPathCount = 0;

typedef struct {
    float x, z;
    int neighbors[8];
    int neighborCount;
} GraphNode;

#define MAX_GRAPH_NODES 40
static GraphNode nodes[MAX_GRAPH_NODES];
static int totalNodes = 0;

static void addNode(int id, float x, float z) {
    nodes[id].x = x;
    nodes[id].z = z;
    nodes[id].neighborCount = 0;
}

static void addEdge(int u, int v) {
    nodes[u].neighbors[nodes[u].neighborCount++] = v;
    nodes[v].neighbors[nodes[v].neighborCount++] = u;
}

void initNavigationNetwork(void) {
    // Road network nodes
    addNode(0, -30.0f, -26.0f); // Entrance
    addNode(1, -30.0f, -22.0f);
    addNode(2, -30.0f, -9.0f);  // Aisle 1 Entrance
    addNode(3, -30.0f, 9.0f);   // Aisle 2 Entrance
    addNode(4, -30.0f, 22.0f);
    addNode(5, -10.0f, 22.0f);
    addNode(6, 10.0f, 22.0f);
    addNode(7, 30.0f, 22.0f);
    addNode(8, 30.0f, 9.0f);
    addNode(9, 30.0f, -9.0f);
    addNode(10, 30.0f, -22.0f);
    addNode(11, 30.0f, -26.0f); // Exit

    addEdge(0, 1);
    addEdge(1, 2);
    addEdge(2, 3);
    addEdge(3, 4);
    addEdge(4, 5);
    addEdge(5, 6);
    addEdge(6, 7);
    addEdge(7, 8);
    addEdge(8, 9);
    addEdge(9, 10);
    addEdge(10, 11);

    // Aisle 1 nodes (12..20)
    float xCoords[9] = { -20.0f, -15.0f, -10.0f, -5.0f, 0.0f, 5.0f, 10.0f, 15.0f, 20.0f };
    for (int i = 0; i < 9; i++) {
        addNode(12 + i, xCoords[i], -9.0f);
    }
    addEdge(2, 12);
    for (int i = 0; i < 8; i++) {
        addEdge(12 + i, 13 + i);
    }
    addEdge(20, 9);

    // Aisle 2 nodes (21..29)
    for (int i = 0; i < 9; i++) {
        addNode(21 + i, xCoords[i], 9.0f);
    }
    addEdge(3, 21);
    for (int i = 0; i < 8; i++) {
        addEdge(21 + i, 22 + i);
    }
    addEdge(29, 8);

    totalNodes = 30;
}

static float heuristic(int a, int b) {
    float dx = nodes[a].x - nodes[b].x;
    float dz = nodes[a].z - nodes[b].z;
    return sqrtf(dx * dx + dz * dz);
}

int calculatePathToSlot(int slotIndex, Vec3* pathOut, int* countOut) {
    if (slotIndex < 0 || slotIndex >= slotCount) {
        *countOut = 0;
        return 0;
    }

    ParkingSlot target = slots[slotIndex];

    int targetNode = -1;
    int aisleStartNode = (target.z < 0.0f) ? 12 : 21;

    for (int i = 0; i < 9; i++) {
        if (fabsf(nodes[aisleStartNode + i].x - target.x) < 0.1f) {
            targetNode = aisleStartNode + i;
            break;
        }
    }

    if (targetNode == -1) targetNode = (target.z < 0.0f) ? 2 : 3;

    // A* algorithm from Node 0 (Entrance) to targetNode
    int startNode = 0;
    float gScore[MAX_GRAPH_NODES];
    float fScore[MAX_GRAPH_NODES];
    int cameFrom[MAX_GRAPH_NODES];
    int openSet[MAX_GRAPH_NODES];
    int openCount = 0;
    int closedSet[MAX_GRAPH_NODES];

    for (int i = 0; i < totalNodes; i++) {
        gScore[i] = 1e9f;
        fScore[i] = 1e9f;
        cameFrom[i] = -1;
        closedSet[i] = 0;
    }

    gScore[startNode] = 0.0f;
    fScore[startNode] = heuristic(startNode, targetNode);
    openSet[openCount++] = startNode;

    while (openCount > 0) {
        int bestIdx = 0;
        for (int i = 1; i < openCount; i++) {
            if (fScore[openSet[i]] < fScore[openSet[bestIdx]]) {
                bestIdx = i;
            }
        }

        int current = openSet[bestIdx];

        if (current == targetNode) {
            int tempPath[MAX_GRAPH_NODES];
            int tempCount = 0;
            int curr = current;
            while (curr != -1) {
                tempPath[tempCount++] = curr;
                curr = cameFrom[curr];
            }

            int count = 0;
            for (int i = tempCount - 1; i >= 0; i--) {
                pathOut[count].x = nodes[tempPath[i]].x;
                pathOut[count].y = 0.1f;
                pathOut[count].z = nodes[tempPath[i]].z;
                count++;
            }

            // Final destination coordinate inside slot
            pathOut[count].x = target.x;
            pathOut[count].y = 0.1f;
            pathOut[count].z = target.z;
            count++;

            *countOut = count;
            return 1;
        }

        openSet[bestIdx] = openSet[--openCount];
        closedSet[current] = 1;

        for (int i = 0; i < nodes[current].neighborCount; i++) {
            int neighbor = nodes[current].neighbors[i];
            if (closedSet[neighbor]) continue;

            float dx = nodes[current].x - nodes[neighbor].x;
            float dz = nodes[current].z - nodes[neighbor].z;
            float dist = sqrtf(dx * dx + dz * dz);
            float tentative_g = gScore[current] + dist;

            if (tentative_g < gScore[neighbor]) {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentative_g;
                fScore[neighbor] = tentative_g + heuristic(neighbor, targetNode);

                int inOpen = 0;
                for (int k = 0; k < openCount; k++) {
                    if (openSet[k] == neighbor) { inOpen = 1; break; }
                }
                if (!inOpen) {
                    openSet[openCount++] = neighbor;
                }
            }
        }
    }

    *countOut = 0;
    return 0;
}

void drawPath(int is3D) {
    if (currentPathCount < 2) return;

    glDisable(GL_LIGHTING);
    glLineWidth(4.5f);
    glColor3f(0.0f, 0.95f, 1.0f); // Bright Cyan navigation line

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < currentPathCount; i++) {
        glVertex3f(currentPath[i].x, is3D ? 0.18f : 0.25f, currentPath[i].z);
    }
    glEnd();

    // Draw waypoints as prominent yellow glowing dots
    glPointSize(9.0f);
    glColor3f(1.0f, 0.95f, 0.1f);
    glBegin(GL_POINTS);
    for (int i = 0; i < currentPathCount; i++) {
        glVertex3f(currentPath[i].x, is3D ? 0.22f : 0.3f, currentPath[i].z);
    }
    glEnd();

    if (is3D) glEnable(GL_LIGHTING);
}
