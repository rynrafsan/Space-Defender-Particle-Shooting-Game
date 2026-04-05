#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Window size
#define WIN_W 600
#define WIN_H 600

// Max counts
#define MAX_BULLETS  10
#define MAX_ENEMIES   8
#define MAX_STARS    50

// ─── Simple Structs ───────────────────────────────────────────────────────────

typedef struct {
    float x, y;
    int   active;
} Bullet;

typedef struct {
    float x, y;
    int   active;
} Enemy;

typedef struct {
    float x, y;
} Star;

// ─── Game Variables ───────────────────────────────────────────────────────────

Bullet bullets[MAX_BULLETS];
Enemy  enemies[MAX_ENEMIES];
Star   stars[MAX_STARS];

float playerX   = 300.0f;
float playerY   = 50.0f;
int   score     = 0;
int   lives     = 3;
int   gameOver  = 0;
int   spawnTimer = 0;

int moveLeft  = 0;
int moveRight = 0;

// ─── Setup stars at random positions ─────────────────────────────────────────

void initStars() {
    int i;
    for (i = 0; i < MAX_STARS; i++) {
        stars[i].x = rand() % WIN_W;
        stars[i].y = rand() % WIN_H;
    }
}

// ─── Draw a filled circle ─────────────────────────────────────────────────────

void drawCircle(float cx, float cy, float r) {
    int i;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (i = 0; i <= 24; i++) {
        float angle = i * 2.0f * 3.14159f / 24.0f;
        glVertex2f(cx + cosf(angle) * r, cy + sinf(angle) * r);
    }
    glEnd();
}

// ─── Draw text on screen ──────────────────────────────────────────────────────

void drawText(float x, float y, char* str) {
    int i;
    glRasterPos2f(x, y);
    for (i = 0; i < (int)strlen(str); i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
}

// ─── Shoot a bullet ───────────────────────────────────────────────────────────

void shoot() {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x      = playerX;
            bullets[i].y      = playerY + 20;
            bullets[i].active = 1;
            return;
        }
    }
}

// ─── Spawn an enemy at the top ────────────────────────────────────────────────

void spawnEnemy() {
    int i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].x      = 40 + rand() % (WIN_W - 80);
            enemies[i].y      = WIN_H - 20;
            enemies[i].active = 1;
            return;
        }
    }
}

// ─── Reset game ───────────────────────────────────────────────────────────────

void resetGame() {
    int i;
    score      = 0;
    lives      = 3;
    gameOver   = 0;
    playerX    = WIN_W / 2.0f;
    spawnTimer = 0;

    for (i = 0; i < MAX_BULLETS; i++) bullets[i].active = 0;
    for (i = 0; i < MAX_ENEMIES; i++) enemies[i].active = 0;
}

// ─── Update everything ────────────────────────────────────────────────────────

void update() {
    int i, j;

    if (gameOver) return;

    // Move player
    if (moveLeft  && playerX > 30)          playerX -= 5;
    if (moveRight && playerX < WIN_W - 30)  playerX += 5;

    // Move bullets upward
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        bullets[i].y += 10.0f;
        if (bullets[i].y > WIN_H)
            bullets[i].active = 0;
    }

    // Spawn enemy every 70 frames
    spawnTimer++;
    if (spawnTimer >= 70) {
        spawnEnemy();
        spawnTimer = 0;
    }

    // Move enemies downward
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        enemies[i].y -= 1.5f;

        // Enemy reached bottom
        if (enemies[i].y < 20) {
            enemies[i].active = 0;
            lives--;
            if (lives <= 0) gameOver = 1;
            continue;
        }

        // Check if any bullet hit this enemy
        for (j = 0; j < MAX_BULLETS; j++) {
            if (!bullets[j].active) continue;

            float dx   = bullets[j].x - enemies[i].x;
            float dy   = bullets[j].y - enemies[i].y;
            float dist = sqrtf(dx*dx + dy*dy);

            if (dist < 22) {
                enemies[i].active = 0;
                bullets[j].active = 0;
                score += 10;
                break;
            }
        }
    }
}

// ─── Draw space background ────────────────────────────────────────────────────

void drawBackground() {
    // Dark blue background
    glColor3f(0.03f, 0.03f, 0.12f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);       glVertex2f(WIN_W, 0);
    glVertex2f(WIN_W, WIN_H); glVertex2f(0, WIN_H);
    glEnd();

    // White star dots
    int i;
    glColor3f(1.0f, 1.0f, 1.0f);
    for (i = 0; i < MAX_STARS; i++)
        drawCircle(stars[i].x, stars[i].y, 1.5f);
}

// ─── Draw player ship (simple triangle) ──────────────────────────────────────

void drawPlayer() {
    float x = playerX, y = playerY;

    // Main body
    glColor3f(0.2f, 0.7f, 1.0f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x,       y + 28);   // top (nose)
    glVertex2f(x - 20,  y - 14);   // bottom left
    glVertex2f(x + 20,  y - 14);   // bottom right
    glEnd();

    // Cockpit circle
    glColor3f(0.8f, 1.0f, 1.0f);
    drawCircle(x, y + 8, 6);
}

// ─── Draw enemies (simple red circles) ───────────────────────────────────────

void drawEnemies() {
    int i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        // Outer red circle
        glColor3f(1.0f, 0.2f, 0.2f);
        drawCircle(enemies[i].x, enemies[i].y, 18);

        // Inner yellow circle (eye)
        glColor3f(1.0f, 1.0f, 0.0f);
        drawCircle(enemies[i].x, enemies[i].y, 7);
    }
}

// ─── Draw bullets (small rectangles) ─────────────────────────────────────────

void drawBullets() {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        glColor3f(0.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(bullets[i].x - 3, bullets[i].y);
        glVertex2f(bullets[i].x + 3, bullets[i].y);
        glVertex2f(bullets[i].x + 3, bullets[i].y + 14);
        glVertex2f(bullets[i].x - 3, bullets[i].y + 14);
        glEnd();
    }
}

// ─── Draw HUD (score and lives) ───────────────────────────────────────────────

void drawHUD() {
    char buf[32];

    glColor3f(0.0f, 1.0f, 0.5f);
    sprintf(buf, "Score: %d", score);
    drawText(10, WIN_H - 30, buf);

    glColor3f(1.0f, 0.3f, 0.3f);
    sprintf(buf, "Lives: %d", lives);
    drawText(WIN_W - 110, WIN_H - 30, buf);
}

// ─── Draw Game Over screen ────────────────────────────────────────────────────

void drawGameOver() {
    char buf[32];

    glColor3f(1.0f, 0.2f, 0.2f);
    drawText(WIN_W/2 - 70, WIN_H/2 + 20, "GAME OVER");

    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buf, "Score: %d", score);
    drawText(WIN_W/2 - 45, WIN_H/2 - 20, buf);

    glColor3f(0.5f, 1.0f, 0.5f);
    drawText(WIN_W/2 - 100, WIN_H/2 - 60, "Press R to Restart");
}

// ─── Display callback ─────────────────────────────────────────────────────────

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawBackground();
    drawEnemies();
    drawBullets();
    drawPlayer();
    drawHUD();

    if (gameOver) drawGameOver();

    glutSwapBuffers();
}

// ─── Timer: runs every 16ms (~60fps) ─────────────────────────────────────────

void timer(int val) {
    update();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ─── Keyboard: spacebar shoots, R restarts ────────────────────────────────────

void keyDown(unsigned char key, int x, int y) {
    if (key == ' ')           shoot();
    if (key == 'r' || key == 'R') resetGame();
}

// ─── Arrow keys pressed ───────────────────────────────────────────────────────

void specialDown(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT)  moveLeft  = 1;
    if (key == GLUT_KEY_RIGHT) moveRight = 1;
}

// ─── Arrow keys released ──────────────────────────────────────────────────────

void specialUp(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT)  moveLeft  = 0;
    if (key == GLUT_KEY_RIGHT) moveRight = 0;
}

// ─── Main ─────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    srand(time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Simple Shooter - Raiyan Jahan");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);

    initStars();
    resetGame();

    glutDisplayFunc(display);
    glutTimerFunc(16, timer, 0);
    glutKeyboardFunc(keyDown);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);

    glutMainLoop();
    return 0;
}
