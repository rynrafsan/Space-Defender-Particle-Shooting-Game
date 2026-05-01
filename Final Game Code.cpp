#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

//Sound:
#ifdef _WIN32
    #include <windows.h>
    void playCrashSound() { Beep(220, 130); }
#else
    void playCrashSound() { printf("\a"); fflush(stdout); }
#endif

// ─── Window & Limits ──────────────────────────────────────────────────────────
#define WIN_W 700
#define WIN_H 700
#define MAX_PARTICLES 200
#define MAX_BULLETS   20
#define MAX_ENEMIES   12
#define MAX_STARS     80
#define PI            3.14159f

// ─── Game States ──────────────────────────────────────────────────────────────
#define STATE_SPLASH   0
#define STATE_PLAY     1
#define STATE_GAMEOVER 2

// ─── Asteroid Name Pool ───────────────────────────────────────────────────────
const char* NAMES[] = {
    "Fahim", "Nuhash", "Prova", "Noon",
    "Labid", "Tantu",  "Protik", "Rizwan"
};
#define NAME_COUNT 8

// ─── Structs ──────────────────────────────────────────────────────────────────
typedef struct {
    float x, y;
    float dx, dy;
    float r, g, b;
    float alpha;
    float size;
    int   life;
    int   active;
} Particle;

typedef struct {
    float x, y;
    int   active;
} Bullet;

typedef struct {
    float x, y;
    float speed;
    float radius;    // Added for asteroid sizing
    int   nameIdx;   // Added for randomly assigned names
    int   active;
} Enemy;

typedef struct {
    float x, y;
    float speed;
    float brightness;
} Star;

// ─── Globals ──────────────────────────────────────────────────────────────────
int      gameState = STATE_SPLASH;
Particle particles[MAX_PARTICLES];
Bullet   bullets[MAX_BULLETS];
Enemy    enemies[MAX_ENEMIES];
Star     stars[MAX_STARS];

float playerX = WIN_W / 2.0f;
float playerY = 60.0f;
float playerSpeed = 6.0f;

int score      = 0;
int lives      = 3;
int level      = 1;
int enemyTimer = 0;
int moveLeft   = 0;
int moveRight  = 0;

// ─── Utilities ────────────────────────────────────────────────────────────────
float randF(float lo, float hi) {
    return lo + ((float)rand() / RAND_MAX) * (hi - lo);
}

// ─── Shapes & Text ────────────────────────────────────────────────────────────
void drawCircle(float cx, float cy, float r) {
    int i;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (i = 0; i <= 30; i++) {
        float a = i * 2.0f * PI / 30.0f;
        glVertex2f(cx + cosf(a) * r, cy + sinf(a) * r);
    }
    glEnd();
}

void drawCircleOutline(float cx, float cy, float r) {
    int i;
    glBegin(GL_LINE_LOOP);
    for (i = 0; i <= 30; i++) {
        float a = i * 2.0f * PI / 30.0f;
        glVertex2f(cx + cosf(a) * r, cy + sinf(a) * r);
    }
    glEnd();
}

void drawRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
        glVertex2f(x,   y);
        glVertex2f(x+w, y);
        glVertex2f(x+w, y+h);
        glVertex2f(x,   y+h);
    glEnd();
}

void drawRectOutline(float x, float y, float w, float h) {
    glBegin(GL_LINE_LOOP);
        glVertex2f(x,   y);
        glVertex2f(x+w, y);
        glVertex2f(x+w, y+h);
        glVertex2f(x,   y+h);
    glEnd();
}

void drawText24(float x, float y, const char* str) {
    int i; glRasterPos2f(x, y);
    for (i = 0; i < (int)strlen(str); i++) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, str[i]);
}

void drawText18(float x, float y, const char* str) {
    int i; glRasterPos2f(x, y);
    for (i = 0; i < (int)strlen(str); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
}

void drawText12(float x, float y, const char* str) {
    int i; glRasterPos2f(x, y);
    for (i = 0; i < (int)strlen(str); i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, str[i]);
}

// ─── Initialization ───────────────────────────────────────────────────────────
void initStars() {
    int i;
    for (i = 0; i < MAX_STARS; i++) {
        stars[i].x          = randF(0, WIN_W);
        stars[i].y          = randF(0, WIN_H);
        stars[i].speed      = randF(0.3f, 1.2f);
        stars[i].brightness = randF(0.4f, 1.0f);
    }
}

void resetGame() {
    int i;
    score    = 0;
    lives    = 3;
    level    = 1;
    playerX  = WIN_W / 2.0f;
    enemyTimer = 0;

    for (i = 0; i < MAX_PARTICLES; i++) particles[i].active = 0;
    for (i = 0; i < MAX_BULLETS;   i++) bullets[i].active   = 0;
    for (i = 0; i < MAX_ENEMIES;   i++) enemies[i].active   = 0;
}

// ─── Particle Effects ─────────────────────────────────────────────────────────
void spawnParticle(float x, float y, float r, float g, float b) {
    int i;
    for (i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].x      = x;
            particles[i].y      = y;
            particles[i].dx     = randF(-3.5f, 3.5f);
            particles[i].dy     = randF(-3.5f, 3.5f);
            particles[i].r      = r;
            particles[i].g      = g;
            particles[i].b      = b;
            particles[i].alpha  = 1.0f;
            particles[i].size   = randF(2.5f, 6.0f);
            particles[i].life   = (int)randF(20, 45);
            particles[i].active = 1;
            return;
        }
    }
}

void spawnExplosion(float x, float y) {
    int i;
    for (i = 0; i < 40; i++) {
        float rnd = randF(0, 1);
        float r, g, b;
        if (rnd < 0.33f)      { r=1.0f; g=0.9f; b=0.2f; }
        else if (rnd < 0.66f) { r=1.0f; g=0.5f; b=0.1f; }
        else                  { r=1.0f; g=1.0f; b=0.9f; }
        spawnParticle(x, y, r, g, b);
    }
}

void spawnTrail() {
    spawnParticle(playerX - 8, playerY - 10, 0.0f, 0.6f, 1.0f);
    spawnParticle(playerX + 8, playerY - 10, 0.0f, 0.8f, 1.0f);
}

// ─── Game Logic Entities ──────────────────────────────────────────────────────
void spawnEnemy() {
    int i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].x       = randF(40, WIN_W - 40);
            enemies[i].y       = WIN_H + 20;
            enemies[i].speed   = randF(1.2f, 1.8f + level * 0.3f);
            enemies[i].radius  = randF(18.0f, 28.0f);       // Asteroid size
            enemies[i].nameIdx = rand() % NAME_COUNT;       // Assign random name
            enemies[i].active  = 1;
            return;
        }
    }
}

void shootBullet() {
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

void updateStars() {
    int i;
    for (i = 0; i < MAX_STARS; i++) {
        stars[i].y -= stars[i].speed;
        if (stars[i].y < 0) {
            stars[i].y = WIN_H;
            stars[i].x = randF(0, WIN_W);
        }
    }
}

void updateParticles() {
    int i;
    for (i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;

        particles[i].x    += particles[i].dx;
        particles[i].y    += particles[i].dy;
        particles[i].dy   -= 0.05f;
        particles[i].life -= 1;
        particles[i].alpha = (float)particles[i].life / 40.0f;
        particles[i].size *= 0.97f;

        if (particles[i].life <= 0 || particles[i].alpha <= 0)
            particles[i].active = 0;
    }
}

void updateBullets() {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;
        bullets[i].y += 12.0f;
        if (bullets[i].y > WIN_H + 10)
            bullets[i].active = 0;
    }
}

void updateEnemies() {
    int i, j;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        enemies[i].y -= enemies[i].speed; // Move only straight down

        // Off screen -> lose life
        if (enemies[i].y < -30) {
            enemies[i].active = 0;
            lives--;
            if (lives <= 0) gameState = STATE_GAMEOVER;
            continue;
        }

        // Bullet Collision
        for (j = 0; j < MAX_BULLETS; j++) {
            if (!bullets[j].active) continue;

            float dx = bullets[j].x - enemies[i].x;
            float dy = bullets[j].y - enemies[i].y;
            float dist = sqrtf(dx*dx + dy*dy);

            // Using the dynamic radius of the asteroid
            if (dist < enemies[i].radius + 8) {
                spawnExplosion(enemies[i].x, enemies[i].y);
                playCrashSound(); // Trigger sound on hit!
                enemies[i].active = 0;
                bullets[j].active = 0;
                score += 100;
                break;
            }
        }
    }
}

// ─── Main Update Loop ─────────────────────────────────────────────────────────
void update() {
    if (gameState != STATE_PLAY) return;

    if (moveLeft  && playerX > 30)         playerX -= playerSpeed;
    if (moveRight && playerX < WIN_W - 30) playerX += playerSpeed;

    static int trailTimer = 0;
    trailTimer++;
    if (trailTimer % 3 == 0) spawnTrail();

    enemyTimer++;
    int spawnRate = 80 - level * 8;
    if (spawnRate < 25) spawnRate = 25;
    if (enemyTimer >= spawnRate) {
        spawnEnemy();
        enemyTimer = 0;
    }

    level = score / 500 + 1;
    if (level > 10) level = 10;

    updateStars();
    updateParticles();
    updateBullets();
    updateEnemies();
}

// ─── Draw Functions ───────────────────────────────────────────────────────────
void drawBackground() {
    glColor3f(0.02f, 0.02f, 0.10f);
    drawRect(0, 0, WIN_W, WIN_H);
}

void drawStars() {
    int i;
    for (i = 0; i < MAX_STARS; i++) {
        float b = stars[i].brightness;
        glColor3f(b * 0.8f, b * 0.8f, b);
        drawCircle(stars[i].x, stars[i].y, 1.2f);
    }
}

void drawParticles() {
    int i;
    for (i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) continue;
        glColor4f(particles[i].r, particles[i].g, particles[i].b, particles[i].alpha);
        drawCircle(particles[i].x, particles[i].y, particles[i].size);
    }
}

void drawPlayer() {
    float x = playerX, y = playerY;

    // Body
    glColor3f(0.0f, 0.8f, 1.0f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x, y + 30);
        glVertex2f(x - 22, y - 14);
        glVertex2f(x + 22, y - 14);
    glEnd();

    // Wings
    glColor3f(0.0f, 0.5f, 0.9f);
    glBegin(GL_TRIANGLES);
        glVertex2f(x - 22, y - 14);
        glVertex2f(x - 36, y - 22);
        glVertex2f(x - 14, y - 22);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex2f(x + 22, y - 14);
        glVertex2f(x + 36, y - 22);
        glVertex2f(x + 14, y - 22);
    glEnd();

    // Cockpit
    glColor3f(0.7f, 0.95f, 1.0f);
    drawCircle(x, y + 10, 7);

    // Engine glow
    glColor4f(0.0f, 1.0f, 1.0f, 0.8f);
    drawCircle(x - 10, y - 16, 5);
    drawCircle(x + 10, y - 16, 5);
}

// Asteroid system + Names
void drawEnemies() {
    int i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        float cx = enemies[i].x;
        float cy = enemies[i].y;
        float r  = enemies[i].radius;

        // Base rock
        glColor3f(0.52f, 0.42f, 0.32f);
        drawCircle(cx, cy, r);

        // Highlight patch
        glColor3f(0.70f, 0.60f, 0.50f);
        drawCircle(cx - r*0.28f, cy + r*0.22f, r*0.44f);

        // Dark crater
        glColor3f(0.35f, 0.27f, 0.20f);
        drawCircle(cx + r*0.22f, cy - r*0.18f, r*0.26f);

        // Outline
        glColor3f(0.28f, 0.20f, 0.12f);
        glLineWidth(1.5f);
        drawCircleOutline(cx, cy, r);
        glLineWidth(1.0f);

        // Text label setup
        const char* name = NAMES[enemies[i].nameIdx];
        float labelX = cx - (strlen(name) * 3.8f);
        float labelY = cy - r - 16.0f;

        // Drop shadow
        glColor3f(0.0f, 0.0f, 0.0f);
        drawText12(labelX + 1, labelY - 1, name);

        // Yellow text
        glColor3f(1.0f, 0.92f, 0.40f);
        drawText12(labelX, labelY, name);
    }
}

void drawBullets() {
    int i;
    for (i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;

        glColor3f(0.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
            glVertex2f(bullets[i].x - 3, bullets[i].y);
            glVertex2f(bullets[i].x + 3, bullets[i].y);
            glVertex2f(bullets[i].x + 3, bullets[i].y + 18);
            glVertex2f(bullets[i].x - 3, bullets[i].y + 18);
        glEnd();

        // Bullet glow tip
        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
        drawCircle(bullets[i].x, bullets[i].y + 18, 3.5f);
    }
}

void drawHUD() {
    char buf[64];

    glColor3f(0.0f, 1.0f, 0.5f);
    sprintf(buf, "Score: %d", score);
    drawText18(10, WIN_H - 30, buf);

    glColor3f(1.0f, 0.8f, 0.0f);
    sprintf(buf, "Level: %d", level);
    drawText18(WIN_W / 2 - 40, WIN_H - 30, buf);

    glColor3f(1.0f, 0.3f, 0.3f);
    sprintf(buf, "Lives: %d", lives);
    drawText18(WIN_W - 110, WIN_H - 30, buf);

    glColor3f(0.4f, 0.4f, 0.5f);
    drawText12(10, 14, "Arrow Keys: Move   |   Space: Shoot");
}

// ─── UI / Branding Splash Screen ──────────────────────────────────────────────
void drawSplash() {
    drawBackground();
    drawStars();

    // University branding box
    glColor3f(0.07f, 0.18f, 0.45f);
    drawRect(130, 472, 350, 66);
    glColor3f(0.30f, 0.55f, 1.0f);
    drawRectOutline(130, 472, 350, 66);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText24(148, 514, "Daffodil International University");

    glColor3f(0.65f, 0.82f, 1.0f);
    drawText12(175, 482, "Department of Computer Science & Engineering");

    // Game title
    glColor3f(0.15f, 0.88f, 1.0f);
    drawText24(188, 426, "SPACE DEFENDER");

    glColor3f(0.80f, 0.90f, 1.0f);
    drawText18(158, 392, "A 2D Particle Asteroid Shooter");

    // Divider
    glColor3f(0.20f, 0.40f, 0.75f);
    glBegin(GL_LINES);
        glVertex2f(80, 376);  glVertex2f(620, 376);
    glEnd();

    // Team
    glColor3f(0.40f, 0.90f, 0.60f);
    drawText12(80, 354, "Team Members:");
    glColor3f(0.90f, 0.90f, 0.90f);
    drawText12(80, 336, "Raiyan Jahan        (ID: 0242310005101599)");
    drawText12(80, 318, "Rizwan Karim Lam    (ID: 0242310005101608)");
    drawText12(80, 300, "Tantu Chakrabarty   (ID: 0242310005101426)");

    // Divider
    glColor3f(0.18f, 0.38f, 0.70f);
    glBegin(GL_LINES);
        glVertex2f(80, 286);  glVertex2f(620, 286);
    glEnd();

    // Controls
    glColor3f(0.50f, 0.72f, 1.0f);
    drawText12(80, 260, "Controls:");
    glColor3f(0.75f, 0.75f, 0.85f);
    drawText12(80, 240, "Left / Right Arrows  :  Move ship");
    drawText12(80, 220, "Space                :  Shoot bullet");
    drawText12(80, 200, "R                   :  Restart after game over");

    // Start prompt
    glColor3f(0.20f, 1.0f, 0.45f);
    drawText18(222, 138, "Press SPACE to Start");
}

void drawGameOver() {
    char buf[64];

    // Dark overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.65f);
    drawRect(0, 0, WIN_W, WIN_H);
    glDisable(GL_BLEND);

    glColor3f(1.0f, 0.2f, 0.2f);
    drawText24(WIN_W/2 - 80, WIN_H/2 + 30, "GAME OVER");

    glColor3f(1.0f, 1.0f, 1.0f);
    sprintf(buf, "Final Score: %d", score);
    drawText18(WIN_W/2 - 70, WIN_H/2 - 10, buf);

    glColor3f(0.6f, 1.0f, 0.6f);
    drawText18(WIN_W/2 - 85, WIN_H/2 - 55, "Press R to Restart");
}

// ─── Display & Timers ─────────────────────────────────────────────────────────
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Enable Alpha Blending for particle effects!
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (gameState == STATE_SPLASH) {
        drawSplash();
    } else {
        drawBackground();
        drawStars();
        drawParticles();
        drawEnemies();
        drawBullets();
        drawPlayer();
        drawHUD();

        if (gameState == STATE_GAMEOVER) drawGameOver();
    }

    glDisable(GL_BLEND);
    glutSwapBuffers();
}

void timer(int val) {
    update();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ─── Input Callbacks ──────────────────────────────────────────────────────────
void keyDown(unsigned char key, int x, int y) {
    if (key == ' ') {
        if (gameState == STATE_SPLASH) {
            gameState = STATE_PLAY;
            resetGame();
        } else if (gameState == STATE_PLAY) {
            shootBullet();
        }
    }
    if ((key == 'r' || key == 'R') && gameState == STATE_GAMEOVER) {
        gameState = STATE_PLAY;
        resetGame();
    }
}

void specialDown(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT)  moveLeft  = 1;
    if (key == GLUT_KEY_RIGHT) moveRight = 1;
}

void specialUp(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT)  moveLeft  = 0;
    if (key == GLUT_KEY_RIGHT) moveRight = 0;
}

// ─── Main
int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Space Defender - DIU Computer Graphics Project");

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
