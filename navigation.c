#include "navigation.h"
#include "vehicle.h"
#include "graphics.h"
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

int calculatePathFromSlotToExit(int slotIndex, Vec3* pathOut, int* countOut) {
    if (slotIndex < 0 || slotIndex >= slotCount) {
        *countOut = 0;
        return 0;
    }

    ParkingSlot target = slots[slotIndex];

    // Find aisle node closest to this slot's X position
    int slotAisleNode = -1;
    int aisleStartNode = (target.z < 0.0f) ? 12 : 21;
    float aisleZ = (target.z < 0.0f) ? -9.0f : 9.0f;

    for (int i = 0; i < 9; i++) {
        if (fabsf(nodes[aisleStartNode + i].x - target.x) < 0.1f) {
            slotAisleNode = aisleStartNode + i;
            break;
        }
    }

    if (slotAisleNode == -1) slotAisleNode = (target.z < 0.0f) ? 2 : 3;

    // A* algorithm from slotAisleNode to Exit (Node 11)
    int startNode = slotAisleNode;
    int goalNode = 11; // Exit gate
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
    fScore[startNode] = heuristic(startNode, goalNode);
    openSet[openCount++] = startNode;

    while (openCount > 0) {
        int bestIdx = 0;
        for (int i = 1; i < openCount; i++) {
            if (fScore[openSet[i]] < fScore[openSet[bestIdx]]) {
                bestIdx = i;
            }
        }

        int current = openSet[bestIdx];

        if (current == goalNode) {
            int tempPath[MAX_GRAPH_NODES];
            int tempCount = 0;
            int curr = current;
            while (curr != -1) {
                tempPath[tempCount++] = curr;
                curr = cameFrom[curr];
            }

            int count = 0;

            // === REALISTIC PULL-OUT SEQUENCE ===
            // Waypoint 0: Slot center (car starts here, inside the slot)
            pathOut[count].x = target.x;
            pathOut[count].y = 0.1f;
            pathOut[count].z = target.z;
            count++;

            // Waypoint 1: 70% pull-out along slot axis toward aisle
            // This creates a gradual reverse out of the slot before reaching the aisle
            float pullOutZ = target.z + (aisleZ - target.z) * 0.65f;
            pathOut[count].x = target.x;
            pathOut[count].y = 0.1f;
            pathOut[count].z = pullOutZ;
            count++;

            // Waypoint 2: Aisle centerline (fully out of the slot)
            pathOut[count].x = target.x;
            pathOut[count].y = 0.1f;
            pathOut[count].z = aisleZ;
            count++;

            // Waypoint 3+: Graph path from aisle node to exit (in order)
            for (int i = tempCount - 1; i >= 0; i--) {
                pathOut[count].x = nodes[tempPath[i]].x;
                pathOut[count].y = 0.1f;
                pathOut[count].z = nodes[tempPath[i]].z;
                count++;
            }

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
                fScore[neighbor] = tentative_g + heuristic(neighbor, goalNode);

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

    // Use thicker line for exit path for better visibility
    if (activeVehicle.isExiting) {
        glLineWidth(5.5f);
        glColor3f(1.0f, 0.5f, 0.0f); // Orange exit navigation line
    } else {
        glLineWidth(4.5f);
        glColor3f(0.0f, 0.95f, 1.0f); // Bright Cyan navigation line
    }

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < currentPathCount; i++) {
        glVertex3f(currentPath[i].x, is3D ? 0.18f : 0.25f, currentPath[i].z);
    }
    glEnd();

    // Draw waypoints as prominent dots
    glPointSize(9.0f);
    if (activeVehicle.isExiting) {
        glColor3f(1.0f, 0.2f, 0.1f); // Red exit waypoint dots
    } else {
        glColor3f(1.0f, 0.95f, 0.1f); // Yellow entry waypoint dots
    }
    glBegin(GL_POINTS);
    for (int i = 0; i < currentPathCount; i++) {
        glVertex3f(currentPath[i].x, is3D ? 0.22f : 0.3f, currentPath[i].z);
    }
    glEnd();

    // ---- EXIT PATH: Directional arrows and labels ----
    if (activeVehicle.isExiting) {
        float pathY = is3D ? 0.25f : 0.35f;

        // Draw directional arrow triangles along the path to show travel direction
        glColor3f(1.0f, 0.6f, 0.1f);
        for (int i = 0; i < currentPathCount - 1; i++) {
            float dx = currentPath[i + 1].x - currentPath[i].x;
            float dz = currentPath[i + 1].z - currentPath[i].z;
            float segLen = sqrtf(dx * dx + dz * dz);
            if (segLen < 2.0f) continue; // Skip very short segments

            // Normalized direction vector
            float nx = dx / segLen;
            float nz = dz / segLen;
            // Perpendicular vector (for arrow width)
            float px = -nz;
            float pz = nx;

            // Draw arrow at midpoint of this segment
            float mx = (currentPath[i].x + currentPath[i + 1].x) * 0.5f;
            float mz = (currentPath[i].z + currentPath[i + 1].z) * 0.5f;
            float arrowLen = 0.9f;
            float arrowW = 0.4f;

            glBegin(GL_TRIANGLES);
            // Arrow tip (points in travel direction)
            glVertex3f(mx + nx * arrowLen, pathY, mz + nz * arrowLen);
            // Arrow left wing
            glVertex3f(mx - nx * arrowLen * 0.3f + px * arrowW, pathY, mz - nz * arrowLen * 0.3f + pz * arrowW);
            // Arrow right wing
            glVertex3f(mx - nx * arrowLen * 0.3f - px * arrowW, pathY, mz - nz * arrowLen * 0.3f - pz * arrowW);
            glEnd();
        }

        // Draw start marker at the slot position (first waypoint)
        if (currentPathCount > 0) {
            // Green circle marker at start
            glColor3f(0.1f, 0.95f, 0.3f);
            glPointSize(14.0f);
            glBegin(GL_POINTS);
            glVertex3f(currentPath[0].x, is3D ? 0.28f : 0.36f, currentPath[0].z);
            glEnd();

            // "START" label at the slot
            glColor3f(0.1f, 0.95f, 0.3f);
            drawText3D(currentPath[0].x - 1.8f,
                       is3D ? 0.5f : 0.4f,
                       currentPath[0].z - 1.5f,
                       "START", GLUT_BITMAP_HELVETICA_12);

            // "REVERSE" label near the pull-out phase (between waypoints 0 and 1)
            if (currentPathCount > 1) {
                float revLabelX = (currentPath[0].x + currentPath[1].x) * 0.5f;
                float revLabelZ = (currentPath[0].z + currentPath[1].z) * 0.5f;
                glColor3f(1.0f, 0.4f, 0.2f);
                drawText3D(revLabelX + 1.2f,
                           is3D ? 0.5f : 0.4f,
                           revLabelZ,
                           "REVERSE", GLUT_BITMAP_HELVETICA_12);
            }
        }

        // Draw exit marker at the end (last waypoint = exit gate)
        if (currentPathCount > 1) {
            // Red circle marker at exit
            glColor3f(1.0f, 0.15f, 0.1f);
            glPointSize(14.0f);
            glBegin(GL_POINTS);
            glVertex3f(currentPath[currentPathCount - 1].x, is3D ? 0.28f : 0.36f, currentPath[currentPathCount - 1].z);
            glEnd();

            // "EXIT >>" label at the gate
            glColor3f(1.0f, 0.3f, 0.1f);
            drawText3D(currentPath[currentPathCount - 1].x - 1.5f,
                       is3D ? 0.5f : 0.4f,
                       currentPath[currentPathCount - 1].z + 1.0f,
                       "EXIT >>", GLUT_BITMAP_HELVETICA_12);
        }

        // Draw turn indicators at corners where direction changes significantly
        for (int i = 1; i < currentPathCount - 1; i++) {
            float dx1 = currentPath[i].x - currentPath[i - 1].x;
            float dz1 = currentPath[i].z - currentPath[i - 1].z;
            float dx2 = currentPath[i + 1].x - currentPath[i].x;
            float dz2 = currentPath[i + 1].z - currentPath[i].z;

            float len1 = sqrtf(dx1 * dx1 + dz1 * dz1);
            float len2 = sqrtf(dx2 * dx2 + dz2 * dz2);
            if (len1 < 0.01f || len2 < 0.01f) continue;

            // Calculate angle between consecutive segments
            float dot = (dx1 * dx2 + dz1 * dz2) / (len1 * len2);
            if (dot < 0.7f) { // Significant turn (> ~45 degrees)
                // Draw a larger yellow dot at turn points
                glColor3f(1.0f, 0.9f, 0.0f);
                glPointSize(12.0f);
                glBegin(GL_POINTS);
                glVertex3f(currentPath[i].x, is3D ? 0.26f : 0.34f, currentPath[i].z);
                glEnd();
            }
        }
    }

    if (is3D) glEnable(GL_LIGHTING);
}
