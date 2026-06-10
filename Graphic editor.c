#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
 
/* ─── Canvas dimensions ─── */
#define ROWS  30
#define COLS  80
#define MAX_OBJECTS 100
 
/* ─── Shape types ─── */
#define SHAPE_CIRCLE    1
#define SHAPE_RECTANGLE 2
#define SHAPE_LINE      3
#define SHAPE_TRIANGLE  4
 
/* ─── 2D canvas ─── */
char canvas[ROWS][COLS];
 
/* ─── Object structure ─── */
typedef struct {
    int  id;
    int  type;
    char ch;        /* '*' or '_' */
    /* circle */
    int cx, cy, r;
    /* rectangle / line share x1,y1,x2,y2 */
    int x1, y1, x2, y2;
    /* triangle – third vertex */
    int x3, y3;
} Object;
 
Object objects[MAX_OBJECTS];
int    object_count = 0;
int    next_id      = 1;
 
/* ══════════════════════════════════════════════
   CANVAS UTILITIES
══════════════════════════════════════════════ */
 
/* Clear canvas to spaces */
void clear_canvas(void) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}
 
/* Safe pixel set */
void put_pixel(int x, int y, char ch) {
    if (x >= 0 && x < COLS && y >= 0 && y < ROWS)
        canvas[y][x] = ch;
}
 
/* ══════════════════════════════════════════════
   DRAWING FUNCTIONS
══════════════════════════════════════════════ */
 
/* Draw a circle using mid-point algorithm.
   Aspect ratio 2:1 correction applied (x*2) for monospace terminals. */
void draw_circle(int cx, int cy, int r, char ch) {
    int x = 0, y = r;
    int d = 1 - r;
 
    while (x <= y) {
        /* Plot all 8 octants with 2:1 x-stretch */
        put_pixel(cx + 2*x, cy + y, ch);
        put_pixel(cx - 2*x, cy + y, ch);
        put_pixel(cx + 2*x, cy - y, ch);
        put_pixel(cx - 2*x, cy - y, ch);
        put_pixel(cx + 2*y, cy + x, ch);
        put_pixel(cx - 2*y, cy + x, ch);
        put_pixel(cx + 2*y, cy - x, ch);
        put_pixel(cx - 2*y, cy - x, ch);
 
        if (d < 0)
            d += 2*x + 3;
        else {
            d += 2*(x - y) + 5;
            y--;
        }
        x++;
    }
}
 
/* Draw a rectangle (outline) */
void draw_rectangle(int x1, int y1, int x2, int y2, char ch) {
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    for (int c = x1; c <= x2; c++) { put_pixel(c, y1, ch); put_pixel(c, y2, ch); }
    for (int r = y1; r <= y2; r++) { put_pixel(x1, r, ch); put_pixel(x2, r, ch); }
}
 
/* Bresenham's line algorithm */
void draw_line(int x1, int y1, int x2, int y2, char ch) {
    int dx =  abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
 
    while (1) {
        put_pixel(x1, y1, ch);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}
 
/* Draw a triangle from three vertices */
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, char ch) {
    draw_line(x1, y1, x2, y2, ch);
    draw_line(x2, y2, x3, y3, ch);
    draw_line(x3, y3, x1, y1, ch);
}
 
/* ══════════════════════════════════════════════
   RENDER: re-draw all objects onto canvas
══════════════════════════════════════════════ */
void render(void) {
    clear_canvas();
    for (int i = 0; i < object_count; i++) {
        Object *o = &objects[i];
        switch (o->type) {
            case SHAPE_CIRCLE:
                draw_circle(o->cx, o->cy, o->r, o->ch);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(o->x1, o->y1, o->x2, o->y2, o->ch);
                break;
            case SHAPE_LINE:
                draw_line(o->x1, o->y1, o->x2, o->y2, o->ch);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(o->x1, o->y1, o->x2, o->y2, o->x3, o->y3, o->ch);
                break;
        }
    }
}
 
/* ══════════════════════════════════════════════
   DISPLAY FUNCTION
══════════════════════════════════════════════ */
void display_picture(void) {
    render();
 
    /* Top border */
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");
 
    for (int r = 0; r < ROWS; r++) {
        printf("|");
        for (int c = 0; c < COLS; c++)
            putchar(canvas[r][c]);
        printf("|\n");
    }
 
    /* Bottom border */
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");
}
 
/* ══════════════════════════════════════════════
   OBJECT MANAGEMENT
══════════════════════════════════════════════ */
 
/* Find object index by id; returns -1 if not found */
int find_object(int id) {
    for (int i = 0; i < object_count; i++)
        if (objects[i].id == id) return i;
    return -1;
}
 
/* Print summary of all objects */
void list_objects(void) {
    if (object_count == 0) {
        printf("  (no objects)\n");
        return;
    }
    for (int i = 0; i < object_count; i++) {
        Object *o = &objects[i];
        printf("  [%d] ", o->id);
        switch (o->type) {
            case SHAPE_CIRCLE:
                printf("circle    ch='%c'  cx=%d cy=%d r=%d\n",
                       o->ch, o->cx, o->cy, o->r);
                break;
            case SHAPE_RECTANGLE:
                printf("rectangle ch='%c'  (%d,%d)->(%d,%d)\n",
                       o->ch, o->x1, o->y1, o->x2, o->y2);
                break;
            case SHAPE_LINE:
                printf("line      ch='%c'  (%d,%d)->(%d,%d)\n",
                       o->ch, o->x1, o->y1, o->x2, o->y2);
                break;
            case SHAPE_TRIANGLE:
                printf("triangle  ch='%c'  (%d,%d) (%d,%d) (%d,%d)\n",
                       o->ch, o->x1, o->y1, o->x2, o->y2, o->x3, o->y3);
                break;
        }
    }
}
 
/* ─── ADD ─── */
void add_object(void) {
    if (object_count >= MAX_OBJECTS) {
        printf("Object limit reached.\n");
        return;
    }
    Object o;
    memset(&o, 0, sizeof(o));
    o.id = next_id++;
 
    printf("Shape (1=circle 2=rectangle 3=line 4=triangle): ");
    scanf("%d", &o.type);
    if (o.type < 1 || o.type > 4) { printf("Invalid shape.\n"); return; }
 
    printf("Character (* or _): ");
    scanf(" %c", &o.ch);
    if (o.ch != '*' && o.ch != '_') { printf("Invalid character.\n"); return; }
 
    switch (o.type) {
        case SHAPE_CIRCLE:
            printf("cx cy radius: ");
            scanf("%d %d %d", &o.cx, &o.cy, &o.r);
            break;
        case SHAPE_RECTANGLE:
            printf("x1 y1 x2 y2: ");
            scanf("%d %d %d %d", &o.x1, &o.y1, &o.x2, &o.y2);
            break;
        case SHAPE_LINE:
            printf("x1 y1 x2 y2: ");
            scanf("%d %d %d %d", &o.x1, &o.y1, &o.x2, &o.y2);
            break;
        case SHAPE_TRIANGLE:
            printf("x1 y1 x2 y2 x3 y3: ");
            scanf("%d %d %d %d %d %d",
                  &o.x1, &o.y1, &o.x2, &o.y2, &o.x3, &o.y3);
            break;
    }
 
    objects[object_count++] = o;
    printf("Added object #%d.\n", o.id);
}
 
/* ─── DELETE ─── */
void delete_object(void) {
    int id;
    printf("Enter object ID to delete: ");
    scanf("%d", &id);
    int idx = find_object(id);
    if (idx == -1) { printf("Object #%d not found.\n", id); return; }
 
    /* Shift remaining objects left */
    for (int i = idx; i < object_count - 1; i++)
        objects[i] = objects[i + 1];
    object_count--;
    printf("Deleted object #%d.\n", id);
}
 
/* ─── MODIFY ─── */
void modify_object(void) {
    int id;
    printf("Enter object ID to modify: ");
    scanf("%d", &id);
    int idx = find_object(id);
    if (idx == -1) { printf("Object #%d not found.\n", id); return; }
 
    Object *o = &objects[idx];
    printf("New character (* or _): ");
    scanf(" %c", &o->ch);
 
    switch (o->type) {
        case SHAPE_CIRCLE:
            printf("New cx cy radius: ");
            scanf("%d %d %d", &o->cx, &o->cy, &o->r);
            break;
        case SHAPE_RECTANGLE:
            printf("New x1 y1 x2 y2: ");
            scanf("%d %d %d %d", &o->x1, &o->y1, &o->x2, &o->y2);
            break;
        case SHAPE_LINE:
            printf("New x1 y1 x2 y2: ");
            scanf("%d %d %d %d", &o->x1, &o->y1, &o->x2, &o->y2);
            break;
        case SHAPE_TRIANGLE:
            printf("New x1 y1 x2 y2 x3 y3: ");
            scanf("%d %d %d %d %d %d",
                  &o->x1, &o->y1, &o->x2, &o->y2, &o->x3, &o->y3);
            break;
    }
    printf("Modified object #%d.\n", id);
}
 
/* ══════════════════════════════════════════════
   MAIN MENU
══════════════════════════════════════════════ */
int main(void) {
    clear_canvas();
    int choice;
 
    printf("=== 2D ASCII Graphics Editor (chars: * and _) ===\n");
 
    while (1) {
        printf("\n--- MENU ---\n");
        printf("  1. Display picture\n");
        printf("  2. Add object\n");
        printf("  3. Delete object\n");
        printf("  4. Modify object\n");
        printf("  5. List objects\n");
        printf("  0. Quit\n");
        printf("Choice: ");
        scanf("%d", &choice);
 
        switch (choice) {
            case 1: display_picture();  break;
            case 2: add_object();       break;
            case 3: delete_object();    break;
            case 4: modify_object();    break;
            case 5: list_objects();     break;
            case 0:
                printf("Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice.\n");
        }
    }
}
