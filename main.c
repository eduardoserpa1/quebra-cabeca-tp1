#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Para usar strings
#include <time.h>
#include <conio.h>
#include "hiroshima.c"


#ifdef WIN32
#include <windows.h> // includes only in MSWindows not in UNIX
#include "gl/glut.h"
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// SOIL é a biblioteca para leitura das imagens
#include "SOIL.h"


typedef unsigned char uch;
// Um pixel RGB (24 bits)
typedef struct
{
    unsigned char r, g, b;
} RGB;

// Uma imagem RGB
typedef struct
{
    int width, height;
    RGB *img;
} Img;

// Protótipos
void load(char *name, Img *pic);
void valida();
int cmp(const void *elem1, const void *elem2);
//
void compara(RGB *proximidade,RGB *pixel_desej, RGB *pixel_saida);

// Funções da interface gráfica e OpenGL
void init();
void draw();
void keyboard(unsigned char key, int x, int y);

// Largura e altura da janela
int width, height;

// Identificadores de textura
GLuint tex[3];

// As 3 imagens
Img pic[3];

// Imagem selecionada (0,1,2)
int sel;

// Enums para facilitar o acesso às imagens
#define ORIGEM 0
#define DESEJ 1
#define SAIDA 2

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("transition [origem] [destino]\n");
        printf("Origem é a fonte das cores, destino é a imagem desejada\n");
        exit(1);
    }
    glutInit(&argc, argv);

    // Define do modo de operacao da GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // pic[0] -> imagem com as cores
    // pic[1] -> imagem desejada
    // pic[2] -> resultado do algoritmo

    // Carrega as duas imagens
    load(argv[1], &pic[ORIGEM]);
    load(argv[2], &pic[DESEJ]);

    // A largura e altura da janela são calculadas de acordo com a maior
    // dimensão de cada imagem
    width = pic[ORIGEM].width > pic[DESEJ].width ? pic[ORIGEM].width : pic[DESEJ].width;
    height = pic[ORIGEM].height > pic[DESEJ].height ? pic[ORIGEM].height : pic[DESEJ].height;

    // A largura e altura da imagem de saída são iguais às da imagem desejada (1)
    pic[SAIDA].width = pic[DESEJ].width;
    pic[SAIDA].height = pic[DESEJ].height;
    pic[SAIDA].img = malloc(pic[DESEJ].width * pic[DESEJ].height * 3); // W x H x 3 bytes (RGB)

    // Especifica o tamanho inicial em pixels da janela GLUT
    glutInitWindowSize(width, height);

    // Cria a janela passando como argumento o titulo da mesma
    glutCreateWindow("Quebra-Cabeca digital");

    // Registra a funcao callback de redesenho da janela de visualizacao
    glutDisplayFunc(draw);

    // Registra a funcao callback para tratamento das teclas ASCII
    glutKeyboardFunc(keyboard);

    // Cria texturas em memória a partir dos pixels das imagens
    tex[ORIGEM] = SOIL_create_OGL_texture((unsigned char *)pic[ORIGEM].img, pic[ORIGEM].width, pic[ORIGEM].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    tex[DESEJ] = SOIL_create_OGL_texture((unsigned char *)pic[DESEJ].img, pic[DESEJ].width, pic[DESEJ].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

    // Exibe as dimensões na tela, para conferência
    printf("Origem   : %s %d x %d\n", argv[1], pic[ORIGEM].width, pic[ORIGEM].height);
    printf("Desejada : %s %d x %d\n", argv[2], pic[DESEJ].width, pic[DESEJ].height);
    sel = ORIGEM; // pic1

    // Define a janela de visualizacao 2D
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, width, height, 0.0);
    glMatrixMode(GL_MODELVIEW);

    srand(time(0)); // Inicializa gerador aleatório (se for usar random)

    printf("Processando...\n");

    // Copia imagem de origem na imagem de saída
    // (NUNCA ALTERAR A IMAGEM DE ORIGEM NEM DESEJADA)
    int tam = pic[ORIGEM].width * pic[ORIGEM].height;
    memcpy(pic[SAIDA].img, pic[ORIGEM].img, sizeof(RGB) * tam);

    // ~
    
    for(int i=0; i<tam; i++){
        RGB *pixel_desejado = &pic[DESEJ].img[i];
        RGB *pixel_saida = &pic[SAIDA].img[i];
        RGB *pixel_origem = &pic[ORIGEM].img[i];
        
        // desenha a imagem desejada na saída
        *pixel_saida = *pixel_origem;
    }
    
    int count = 0;

    while(count < 10*tam){
        long r1;
        long r2;

        r1 = genrand64_int64() % tam;
        r2 = genrand64_int64() % tam;
        
        if(r1 == r2) continue;

        RGB *p1_saida = &pic[SAIDA].img[r1];
        RGB *p2_saida = &pic[SAIDA].img[r2];

        RGB *p1_desej = &pic[DESEJ].img[r1];
        RGB *p2_desej = &pic[DESEJ].img[r2];

        RGB p1_proximidade_antes;
        RGB p2_proximidade_antes;
        RGB p1_proximidade_depois;
        RGB p2_proximidade_depois;

        compara(&p1_proximidade_antes,p1_desej,p1_saida);
        compara(&p2_proximidade_antes,p2_desej,p2_saida);

        compara(&p1_proximidade_depois,p2_desej,p1_saida);
        compara(&p2_proximidade_depois,p1_desej,p2_saida);

        if(
            (p1_proximidade_depois.r <= p1_proximidade_antes.r && p1_proximidade_depois.g <= p1_proximidade_antes.g && p1_proximidade_depois.b <= p1_proximidade_antes.b)
          ){
            count++;
            RGB aux;
            aux = pic[SAIDA].img[r1];
            pic[SAIDA].img[r1] = pic[SAIDA].img[r2];
            pic[SAIDA].img[r2] = aux;
        }
        

    }
 

    // ~

    // NÃO ALTERAR A PARTIR DAQUI!

    // Cria textura para a imagem de saída
    tex[SAIDA] = SOIL_create_OGL_texture((unsigned char *)pic[SAIDA].img, pic[SAIDA].width, pic[SAIDA].height, SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
    // Grava imagem de saída em out.bmp, para conferência
    SOIL_save_image("out.bmp", SOIL_SAVE_TYPE_BMP, pic[SAIDA].width, pic[SAIDA].height, 3, (const unsigned char *)pic[SAIDA].img);

    // Entra no loop de eventos, não retorna
    glutMainLoop();
}

void compara(RGB *proximidade,RGB *pixel_desej, RGB *pixel_saida){
    int r,g,b;

    r = pixel_saida->r - pixel_desej->r;
    g = pixel_saida->g - pixel_desej->g;
    b = pixel_saida->b - pixel_desej->b;

    if(r < 0) r *= -1;
    if(g < 0) g *= -1;
    if(b < 0) b *= -1;

    proximidade->r = r;
    proximidade->g = g;
    proximidade->b = b;
}

// Carrega uma imagem para a struct Img
void load(char *name, Img *pic)
{
    int chan;
    pic->img = (RGB *)SOIL_load_image(name, &pic->width, &pic->height, &chan, SOIL_LOAD_RGB);
    if (!pic->img)
    {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
        exit(1);
    }
    printf("Load: %d x %d x %d\n", pic->width, pic->height, chan);
}

// Verifica se o algoritmo foi aplicado corretamente:
// Ordena os pixels da imagem origem e de saída por R, G e B;
// depois compara uma com a outra: devem ser iguais
void valida()
{
    int ok = 1;
    int size = pic[ORIGEM].width * pic[ORIGEM].height;
    // Aloca memória para os dois arrays
    RGB *aux1 = malloc(size * 3);
    RGB *aux2 = malloc(size * 3);
    // Copia os pixels originais
    memcpy(aux1, pic[ORIGEM].img, size * 3);
    memcpy(aux2, pic[SAIDA].img, size * 3);
    // Mostra primeiros 8 pixels de ambas as imagens
    // antes de ordenar (teste)
    for (int i = 0; i < 8; i++)
        printf("[%02X %02X %02X] ", aux1[i].r, aux1[i].g, aux1[i].b);
    printf("\n");
    for (int i = 0; i < 8; i++)
        printf("[%02X %02X %02X] ", aux2[i].r, aux2[i].g, aux2[i].b);
    printf("\n");
    printf("Validando...\n");
    // Ordena ambos os arrays
    qsort(aux1, size, sizeof(RGB), cmp);
    qsort(aux2, size, sizeof(RGB), cmp);
    // Mostra primeiros 8 pixels de ambas as imagens
    // depois de ordenar
    for (int i = 0; i < 8; i++)
        printf("[%02X %02X %02X] ", aux1[i].r, aux1[i].g, aux1[i].b);
    printf("\n");
    for (int i = 0; i < 8; i++)
        printf("[%02X %02X %02X] ", aux2[i].r, aux2[i].g, aux2[i].b);
    printf("\n");
    for (int i = 0; i < size; i++)
    {
        if (aux1[i].r != aux2[i].r ||
            aux1[i].g != aux2[i].g ||
            aux1[i].b != aux2[i].b)
        {
            // Se pelo menos um dos pixels for diferente, o algoritmo foi aplicado incorretamente
            printf("*** INVÁLIDO na posição %d ***: %02X %02X %02X -> %02X %02X %02X\n",
                   i, aux1[i].r, aux1[i].g, aux1[i].b, aux2[i].r, aux2[i].g, aux2[i].b);
            ok = 0;
            break;
        }
    }
    // Libera memória dos arrays ordenados
    free(aux1);
    free(aux2);
    if (ok)
        printf(">>>> TRANSFORMAÇÃO VÁLIDA <<<<<\n");
}

// Funcao de comparacao para qsort: ordena por R, G, B (desempate nessa ordem)
int cmp(const void *elem1, const void *elem2)
{
    RGB *ptr1 = (RGB *)elem1;
    RGB *ptr2 = (RGB *)elem2;
    unsigned char r1 = ptr1->r;
    unsigned char r2 = ptr2->r;
    unsigned char g1 = ptr1->g;
    unsigned char g2 = ptr2->g;
    unsigned char b1 = ptr1->b;
    unsigned char b2 = ptr2->b;
    int r = 0;
    if (r1 < r2)
        r = -1;
    else if (r1 > r2)
        r = 1;
    else if (g1 < g2)
        r = -1;
    else if (g1 > g2)
        r = 1;
    else if (b1 < b2)
        r = -1;
    else if (b1 > b2)
        r = 1;
    return r;
}

//
// Funções de callback da OpenGL
//
// SÓ ALTERE SE VOCÊ TIVER ABSOLUTA CERTEZA DO QUE ESTÁ FAZENDO!
//

// Gerencia eventos de teclado
void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
    {
        // ESC: libera memória e finaliza
        free(pic[0].img);
        free(pic[1].img);
        free(pic[2].img);
        exit(1);
    }
    if (key >= '1' && key <= '3')
        // 1-3: seleciona a imagem correspondente (origem, desejada e saída)
        sel = key - '1';
    // V para validar a solução
    if (key == 'v')
        valida();
    glutPostRedisplay();
}

// Callback de redesenho da tela
void draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Preto
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Para outras cores, veja exemplos em /etc/X11/rgb.txt

    glColor3ub(255, 255, 255); // branco

    // Ativa a textura corresponde à imagem desejada
    glBindTexture(GL_TEXTURE_2D, tex[sel]);
    // E desenha um retângulo que ocupa toda a tela
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(0, 0);

    glTexCoord2f(1, 0);
    glVertex2f(pic[sel].width, 0);

    glTexCoord2f(1, 1);
    glVertex2f(pic[sel].width, pic[sel].height);

    glTexCoord2f(0, 1);
    glVertex2f(0, pic[sel].height);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Exibe a imagem
    glutSwapBuffers();
}
