/*
 * ============================================================
 *  AERO-CONCEPT  -  Application complete (Allegro 5)
 *
 *  DOSSIER :
 *    avion.exe
 *    comptes.txt
 *    sprites/pilot_walk1.png .. pilot_walk4.png
 *
 *  COMPILATION :
 *    gcc aero_concept.c -o avion -lallegro -lallegro_main
 *        -lallegro_font -lallegro_ttf -lallegro_primitives
 *        -lallegro_image -lm
 * ============================================================
 */
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define SCR_W        1100
#define SCR_H         650
#define G_CONST       9.81f
#define COMPTES_FILE "comptes.txt"
#define MAX_COMPTES   200
/* 8 frames smooth + 1 pull + 1 front */
#define SMOOTH_COUNT   8
#define SPR_PULL       8
#define SPR_FRONT      9
#define SPRITE_COUNT   10
#define SPRITE_SPEED    6
#define NB_AVIONS      40
#define PHASE_WALK     80
#define PHASE_PULL    170
#define PHASE_TURN    200

/* ======= STRUCTURES ======= */
typedef struct { char user[64]; char pass[64]; } Compte;

typedef struct {
    char  nom[40];
    float masse,surface,clmax,rho,cd0,k,poussee;
    int   existe;
} Avion;

typedef enum {
    PAGE_MENU,PAGE_GESTION,PAGE_GESTION_LISTE,
    PAGE_GESTION_CUSTOM,PAGE_GESTION_VOIR,
    PAGE_ANALYSE,PAGE_DECOLLAGE,PAGE_AIDE,PAGE_VISIONNEUSE,
    PAGE_CHASSE
} Page;

typedef struct {
    Page  page; Avion avion;
    int   liste_scroll,liste_sel,inp_active;
    char  inp_nom[40],inp_masse[20],inp_surface[20],inp_clmax[20];
    char  inp_rho[20],inp_cd0[20],inp_k[20],inp_poussee[20];
    int   anim_frame; bool anim_running,anim_possible;
    float anim_Vs;
    char  msg[100]; bool msg_ok; int msg_timer;
    /* Visionneuse 3D */
    float vis_yaw, vis_pitch;
    bool  vis_dragging;
    float vis_drag_x, vis_drag_y;
    int   vis_hovered_part; /* index de la partie survolee, -1 sinon */
} AppState;

typedef struct {
    float x,y,w,h; char label[50];
    int r,g,b,rb,gb,bb;
} Btn;

typedef enum { SCREEN_LOGIN,SCREEN_REGISTER } ScreenMode;

typedef struct {
    char user[64],pass[64],ru[64],rp[64],rp2[64];
    int  active; bool err,rerr,rok; char msg[100];
    ScreenMode mode;
} FormLogin;

/* ======= BASE AVIONS ======= */
static Avion avions_pred[NB_AVIONS]={
    {"Robin DR400",     750, 16.0f,1.50f,1.225f,0.025f,0.045f,1200,1},
    {"Cessna 172",     1100, 16.2f,1.40f,1.225f,0.030f,0.050f,1800,1},
    {"Piper PA28",     1500, 16.8f,1.50f,1.225f,0.028f,0.048f,2200,1},
    {"Diamond DA40",    900, 12.5f,1.60f,1.225f,0.026f,0.043f,1400,1},
    {"Beechcraft Bonanza",1700,17.1f,1.45f,1.225f,0.029f,0.047f,2500,1},
    {"Cirrus SR22",    1400, 16.2f,1.52f,1.225f,0.027f,0.045f,2300,1},
    {"Mooney M20",     1300, 15.8f,1.48f,1.225f,0.028f,0.046f,2100,1},
    {"Piper J3 Cub",    500, 14.2f,1.60f,1.225f,0.024f,0.040f,1000,1},
    {"Cessna 150",      650, 15.0f,1.55f,1.225f,0.025f,0.042f,1100,1},
    {"Extra 300",       800, 11.5f,1.70f,1.225f,0.027f,0.045f,1600,1},
    {"Fouga Magister", 1800, 16.5f,1.48f,1.225f,0.030f,0.048f,2400,1},
    {"Stinson 108",    1600, 16.8f,1.50f,1.225f,0.029f,0.047f,2200,1},
    {"Cessna 180",     1200, 17.0f,1.47f,1.225f,0.032f,0.050f,2000,1},
    {"Piper PA32",     2000, 20.1f,1.40f,1.225f,0.031f,0.049f,2600,1},
    {"Beechcraft Sundowner",1550,17.5f,1.46f,1.225f,0.028f,0.046f,2300,1},
    {"Grumman Tiger",   950, 14.8f,1.55f,1.225f,0.027f,0.044f,1300,1},
    {"Socata TB10",    1350, 15.9f,1.51f,1.225f,0.028f,0.045f,2100,1},
    {"Socata TB20",    1450, 16.7f,1.50f,1.225f,0.029f,0.046f,2200,1},
    {"Cessna 210",     1900, 17.8f,1.45f,1.225f,0.032f,0.050f,2700,1},
    {"Piper Arrow",    1250, 16.2f,1.49f,1.225f,0.029f,0.046f,2000,1},
    {"Zenith CH701",    650, 12.1f,1.60f,1.225f,0.024f,0.042f, 950,1},
    {"Flight Design CT",620, 12.5f,1.55f,1.225f,0.025f,0.043f,1000,1},
    {"Cessna 182",     1350, 16.9f,1.48f,1.225f,0.031f,0.048f,2100,1},
    {"Piper PA24",     1700, 18.2f,1.45f,1.225f,0.030f,0.048f,2500,1},
    {"Lancair 360",     950, 11.9f,1.70f,1.225f,0.027f,0.044f,1700,1},
    {"Van s RV7",       700, 12.0f,1.62f,1.225f,0.025f,0.042f,1200,1},
    {"Kitfox Series7",  800, 12.7f,1.60f,1.225f,0.026f,0.043f,1350,1},
    {"Cessna 205",     1450, 17.5f,1.46f,1.225f,0.030f,0.046f,2100,1},
    {"Piper PA14",      900, 14.8f,1.53f,1.225f,0.027f,0.044f,1500,1},
    {"Piper PA20",     1150, 15.9f,1.50f,1.225f,0.028f,0.045f,1800,1},
    {"Ercoupe 415",     780, 12.8f,1.56f,1.225f,0.025f,0.042f,1200,1},
    {"Taylorcraft BC12",680, 13.1f,1.59f,1.225f,0.024f,0.041f,1000,1},
    {"Wright Stagger",  980, 15.2f,1.52f,1.225f,0.028f,0.044f,1500,1},
    {"Siai Marchetti", 1850, 17.0f,1.46f,1.225f,0.031f,0.047f,2600,1},
    {"Piper PA46",     1800, 19.0f,1.45f,1.225f,0.030f,0.047f,2600,1},
    {"Gippsland GA8",  1100, 16.7f,1.47f,1.225f,0.029f,0.046f,1800,1},
    {"Champagne Reno",  760, 13.5f,1.58f,1.225f,0.026f,0.044f,1100,1},
    {"Mercury Village", 850, 13.2f,1.57f,1.225f,0.026f,0.043f,1300,1},
    {"Beechcraft Skylark",1150,16.0f,1.48f,1.225f,0.030f,0.045f,1900,1},
    {"Piper Arrow II", 1300, 16.4f,1.49f,1.225f,0.029f,0.046f,2050,1}
};

/* ======= COMPTES ======= */
static int    nb_comptes=0;
static Compte comptes[MAX_COMPTES];

static void charger_comptes(void){
    FILE *f; char line[140],*sep; nb_comptes=0;
    f=fopen(COMPTES_FILE,"r"); if(!f) return;
    while(nb_comptes<MAX_COMPTES){
        if(!fgets(line,sizeof(line),f)) break;
        line[strcspn(line,"\r\n")]='\0';
        if(strlen(line)<3) continue;
        sep=strchr(line,':'); if(!sep) continue; *sep='\0';
        strncpy(comptes[nb_comptes].user,line,63);
        strncpy(comptes[nb_comptes].pass,sep+1,63);
        nb_comptes++;
    }
    fclose(f);
}
static void sauvegarder_comptes(void){
    int i; FILE *f=fopen(COMPTES_FILE,"w"); if(!f) return;
    for(i=0;i<nb_comptes;i++) fprintf(f,"%s:%s\n",comptes[i].user,comptes[i].pass);
    fclose(f);
}
static bool verifier(const char *u,const char *p){
    int i;
    for(i=0;i<nb_comptes;i++)
        if(!strcmp(comptes[i].user,u)&&!strcmp(comptes[i].pass,p)) return true;
    return false;
}
static bool existe_compte(const char *u){
    int i;
    for(i=0;i<nb_comptes;i++) if(!strcmp(comptes[i].user,u)) return true;
    return false;
}
static bool creer_compte(const char *u,const char *p){
    if(nb_comptes>=MAX_COMPTES||existe_compte(u)) return false;
    strncpy(comptes[nb_comptes].user,u,63);
    strncpy(comptes[nb_comptes].pass,p,63);
    nb_comptes++; sauvegarder_comptes(); return true;
}

/* ======= CALCULS ======= */
static float vit_decrochage(Avion a){
    float W=a.masse*G_CONST;
    return sqrtf((2*W)/(a.rho*a.surface*a.clmax));
}
static float calc_trainee(Avion a,float V){
    float W,CL,CD;
    if(V<1.0f) V=1.0f;
    W=a.masse*G_CONST;
    CL=(2*W)/(a.rho*a.surface*V*V);
    if(CL>a.clmax) CL=a.clmax;
    CD=a.cd0+a.k*CL*CL;
    return 0.5f*a.rho*a.surface*CD*V*V;
}

/* ======= UTILITAIRES ======= */
static bool in_rect(float mx,float my,float x,float y,float w,float h)
{ return mx>=x&&mx<=x+w&&my>=y&&my<=y+h; }

static void draw_bg(int tick){
    int i,y; float t,gy,ry,off;
    for(y=0;y<SCR_H;y++){
        t=(float)y/SCR_H;
        al_draw_line(0,y,SCR_W,y,al_map_rgb((int)(8+t*10),(int)(12+t*16),(int)(28+t*28)),1);
    }
    srand(42);
    for(i=0;i<100;i++){
        float b; int sx=rand()%SCR_W,sy=rand()%(int)(SCR_H*0.72f);
        b=0.4f+0.6f*sinf((tick+i*19)*0.04f);
        al_draw_filled_circle(sx,sy,1,al_map_rgb((int)(110+145*b),(int)(110+145*b),(int)(110+145*b)));
    }
    al_draw_filled_circle(960,68,46,al_map_rgb(228,222,196));
    al_draw_filled_circle(984,54,38,al_map_rgb(8,12,28));
    al_draw_filled_ellipse(185,105,80,24,al_map_rgba(35,50,85,155));
    al_draw_filled_ellipse(235,93,58,20,al_map_rgba(35,50,85,155));
    al_draw_filled_ellipse(690,115,85,26,al_map_rgba(35,50,85,155));
    gy=SCR_H*0.79f;
    al_draw_filled_rectangle(0,gy,SCR_W,SCR_H,al_map_rgb(12,16,30));
    ry=gy+5;
    al_draw_filled_rectangle(0,ry,SCR_W,ry+28,al_map_rgb(24,28,40));
    off=fmodf(tick*2.0f,70.0f);
    for(i=-70;i<SCR_W+70;i+=70)
        al_draw_filled_rectangle(i+off,ry+11,i+off+36,ry+18,al_map_rgb(188,158,38));
    for(i=0;i<SCR_W;i+=44){
        float g=0.5f+0.5f*sinf((tick+i)*0.1f);
        al_draw_filled_circle(i,ry-4,3,al_map_rgba(202,162,38,(int)(155*g)));
    }
    al_draw_filled_rectangle(36,gy-98,208,gy,al_map_rgb(9,12,24));
    al_draw_filled_triangle(36,gy-98,122,gy-142,208,gy-98,al_map_rgb(9,12,24));
    al_draw_filled_rectangle(62,gy-80,100,gy-50,al_map_rgba(58,112,202,48));
    al_draw_filled_rectangle(112,gy-80,150,gy-50,al_map_rgba(58,112,202,48));
    al_draw_filled_rectangle(845,gy-122,888,gy,al_map_rgb(9,12,24));
    al_draw_filled_rectangle(830,gy-145,902,gy-122,al_map_rgb(14,20,42));
    al_draw_filled_rectangle(840,gy-135,892,gy-124,al_map_rgba(58,162,208,68));
    if(sinf(tick*0.14f)>0) al_draw_filled_circle(866,gy-149,5,al_map_rgb(208,38,38));
}

static void draw_btn(ALLEGRO_FONT *fn,Btn b,float mx,float my){
    bool hov=in_rect(mx,my,b.x,b.y,b.w,b.h);
    int  alpha=hov?255:200;
    al_draw_filled_rounded_rectangle(b.x+6,b.y+6,b.x+b.w+6,b.y+b.h+6,10,10,al_map_rgba(0,0,0,80));
    al_draw_filled_rounded_rectangle(b.x,b.y,b.x+b.w,b.y+b.h,10,10,al_map_rgba(b.r,b.g,b.b,alpha));
    al_draw_rounded_rectangle(b.x,b.y,b.x+b.w,b.y+b.h,10,10,al_map_rgb(b.rb,b.gb,b.bb),2.0f);
    al_draw_text(fn,al_map_rgb(255,255,255),b.x+b.w/2,b.y+b.h/2-5,ALLEGRO_ALIGN_CENTRE,b.label);
}

static void draw_field(ALLEGRO_FONT *fn,const char *label,const char *buf,
                        int fid,int active,float x0,float fy,float pw,bool is_pass){
    char disp[64]; int k; float cw; ALLEGRO_COLOR bc;
    al_draw_text(fn,al_map_rgb(170,185,230),x0+20,fy-20,ALLEGRO_ALIGN_LEFT,label);
    al_draw_filled_rounded_rectangle(x0+16,fy,x0+pw-16,fy+44,8,8,al_map_rgb(6,12,32));
    bc=(active==fid)?al_map_rgb(90,160,255):al_map_rgb(38,55,108);
    al_draw_rounded_rectangle(x0+16,fy,x0+pw-16,fy+44,8,8,bc,2.0f);
    memset(disp,0,sizeof(disp));
    if(is_pass){ int l=strlen(buf); for(k=0;k<l&&k<63;k++) disp[k]='*'; }
    else strncpy(disp,buf,63);
    al_draw_text(fn,al_map_rgb(228,235,255),x0+32,fy+13,ALLEGRO_ALIGN_LEFT,disp);
    if(active==fid){
        cw=al_get_text_width(fn,disp);
        al_draw_filled_rectangle(x0+32+cw,fy+10,x0+34+cw,fy+34,al_map_rgb(90,160,255));
    }
}

static void draw_titre(ALLEGRO_FONT *fb,ALLEGRO_FONT *fn,const char *titre,const char *sous){
    float pw=700,x0=(SCR_W-pw)/2;
    al_draw_filled_rounded_rectangle(x0,18,x0+pw,80,12,12,al_map_rgba(16,38,115,220));
    al_draw_rounded_rectangle(x0,18,x0+pw,80,12,12,al_map_rgb(175,138,42),2.0f);
    al_draw_text(fb,al_map_rgb(212,172,48),SCR_W/2,28,ALLEGRO_ALIGN_CENTRE,titre);
    if(sous&&strlen(sous)>0)
        al_draw_text(fn,al_map_rgb(140,165,215),SCR_W/2,58,ALLEGRO_ALIGN_CENTRE,sous);
}

/* ======= LOGIN PANELS ======= */
static void draw_login_panel(float px,float py,ALLEGRO_FONT *fb,ALLEGRO_FONT *fn,const FormLogin *fs){
    float pw=400,ph=400,x0=px-pw/2,y0=py-ph/2,by2;
    al_draw_filled_rounded_rectangle(x0+12,y0+12,x0+pw+12,y0+ph+12,18,18,al_map_rgba(0,0,0,160));
    al_draw_filled_rounded_rectangle(x0,y0,x0+pw,y0+ph,18,18,al_map_rgba(12,18,52,250));
    al_draw_rounded_rectangle(x0,y0,x0+pw,y0+ph,18,18,al_map_rgba(80,140,255,220),2.5f);
    al_draw_filled_rounded_rectangle(x0,y0,x0+pw,y0+62,18,18,al_map_rgb(16,38,115));
    al_draw_filled_rectangle(x0,y0+44,x0+pw,y0+62,al_map_rgb(16,38,115));
    al_draw_line(x0+22,y0+63,x0+pw-22,y0+63,al_map_rgb(175,138,42),2.0f);
    al_draw_text(fb,al_map_rgb(212,172,48),px,y0+18,ALLEGRO_ALIGN_CENTRE,"AERO-CONCEPT");
    al_draw_text(fn,al_map_rgb(95,128,190),px,y0+76,ALLEGRO_ALIGN_CENTRE,"Acces systeme de vol");
    draw_field(fn,"Identifiant", fs->user,0,fs->active,x0,y0+112,pw,false);
    draw_field(fn,"Mot de passe",fs->pass,1,fs->active,x0,y0+204,pw,true);
    if(fs->err) al_draw_text(fn,al_map_rgb(235,65,65),px,y0+258,ALLEGRO_ALIGN_CENTRE,fs->msg);
    { float by1=y0+ph-106;
      al_draw_filled_rounded_rectangle(x0+10,by1+6,x0+pw-10,by1+52,10,10,al_map_rgba(0,0,0,80));
      al_draw_filled_rounded_rectangle(x0+8,by1,x0+pw-8,by1+46,10,10,al_map_rgb(25,80,210));
      al_draw_rounded_rectangle(x0+8,by1,x0+pw-8,by1+46,10,10,al_map_rgb(70,140,255),2.0f);
      al_draw_text(fn,al_map_rgb(255,255,255),px,by1+15,ALLEGRO_ALIGN_CENTRE,"SE CONNECTER"); }
    al_draw_line(x0+30,y0+ph-52,px-30,y0+ph-52,al_map_rgb(38,52,105),1);
    al_draw_text(fn,al_map_rgb(68,85,138),px,y0+ph-60,ALLEGRO_ALIGN_CENTRE,"ou");
    al_draw_line(px+30,y0+ph-52,x0+pw-30,y0+ph-52,al_map_rgb(38,52,105),1);
    by2=y0+ph-42;
    al_draw_filled_rounded_rectangle(x0+20,by2,x0+pw-20,by2+32,10,10,al_map_rgba(38,75,175,60));
    al_draw_rounded_rectangle(x0+20,by2,x0+pw-20,by2+32,10,10,al_map_rgb(75,128,228),2.0f);
    al_draw_text(fn,al_map_rgb(118,178,255),px,by2+9,ALLEGRO_ALIGN_CENTRE,"S'INSCRIRE");
}

static void draw_register_panel(float px,float py,ALLEGRO_FONT *fb,ALLEGRO_FONT *fn,const FormLogin *fs){
    float pw=400,ph=492,x0=px-pw/2,y0=py-ph/2,by2;
    al_draw_filled_rounded_rectangle(x0+12,y0+12,x0+pw+12,y0+ph+12,18,18,al_map_rgba(0,0,0,160));
    al_draw_filled_rounded_rectangle(x0,y0,x0+pw,y0+ph,18,18,al_map_rgba(10,22,34,250));
    al_draw_rounded_rectangle(x0,y0,x0+pw,y0+ph,18,18,al_map_rgba(45,195,105,220),2.5f);
    al_draw_filled_rounded_rectangle(x0,y0,x0+pw,y0+62,18,18,al_map_rgb(14,68,42));
    al_draw_filled_rectangle(x0,y0+44,x0+pw,y0+62,al_map_rgb(14,68,42));
    al_draw_line(x0+22,y0+63,x0+pw-22,y0+63,al_map_rgb(42,188,95),2.0f);
    al_draw_text(fb,al_map_rgb(75,228,128),px,y0+18,ALLEGRO_ALIGN_CENTRE,"INSCRIPTION");
    al_draw_text(fn,al_map_rgb(78,152,108),px,y0+76,ALLEGRO_ALIGN_CENTRE,"Creer un compte pilote");
    draw_field(fn,"Identifiant",          fs->ru, 2,fs->active,x0,y0+110,pw,false);
    draw_field(fn,"Mot de passe",          fs->rp, 3,fs->active,x0,y0+196,pw,true);
    draw_field(fn,"Confirmer mot de passe",fs->rp2,4,fs->active,x0,y0+282,pw,true);
    if(fs->rerr) al_draw_text(fn,al_map_rgb(228,62,62),px,y0+338,ALLEGRO_ALIGN_CENTRE,fs->msg);
    if(fs->rok)  al_draw_text(fn,al_map_rgb(48,222,98), px,y0+338,ALLEGRO_ALIGN_CENTRE,"Compte cree ! Connectez-vous.");
    { float by1=y0+ph-106;
      al_draw_filled_rounded_rectangle(x0+10,by1+6,x0+pw-10,by1+52,10,10,al_map_rgba(0,0,0,80));
      al_draw_filled_rounded_rectangle(x0+8,by1,x0+pw-8,by1+46,10,10,al_map_rgb(18,118,58));
      al_draw_rounded_rectangle(x0+8,by1,x0+pw-8,by1+46,10,10,al_map_rgb(50,205,100),2.0f);
      al_draw_text(fn,al_map_rgb(240,255,240),px,by1+15,ALLEGRO_ALIGN_CENTRE,"CREER MON COMPTE"); }
    by2=y0+ph-48;
    al_draw_filled_rounded_rectangle(x0+20,by2,x0+pw-20,by2+32,10,10,al_map_rgba(38,58,128,60));
    al_draw_rounded_rectangle(x0+20,by2,x0+pw-20,by2+32,10,10,al_map_rgb(55,82,152),2.0f);
    al_draw_text(fn,al_map_rgb(118,152,228),px,by2+9,ALLEGRO_ALIGN_CENTRE,"< Retour connexion");
}

static void draw_rope(float x1,float y1,float x2,float y2,float sag)
{
    int j; float mx2=(x1+x2)/2.0f, my2=(y1+y2)/2.0f+sag;
    for (j=0;j<28;j++){
        float t1=(float)j/28.0f, t2=(float)(j+1)/28.0f;
        float px1=(1-t1)*(1-t1)*x1+2*(1-t1)*t1*mx2+t1*t1*x2;
        float py1=(1-t1)*(1-t1)*y1+2*(1-t1)*t1*my2+t1*t1*y2;
        float px2=(1-t2)*(1-t2)*x1+2*(1-t2)*t2*mx2+t2*t2*x2;
        float py2=(1-t2)*(1-t2)*y1+2*(1-t2)*t2*my2+t2*t2*y2;
        al_draw_line(px1,py1,px2,py2,al_map_rgb(180,140,50),3);
    }
}

/* ======= ECRAN LOGIN ======= */
static bool ecran_login(ALLEGRO_DISPLAY *disp,ALLEGRO_EVENT_QUEUE *queue,
                         ALLEGRO_TIMER *timer,ALLEGRO_FONT *fb,ALLEGRO_FONT *fn,
                         ALLEGRO_BITMAP **sprites,bool has_spr,char *logged_user){
    int tick=0; bool running=true,logged_in=false;
    FormLogin fs;
    float pfx=SCR_W*0.19f, pfy=SCR_H*0.82f;
    float panx=SCR_W*0.60f, pany=SCR_H*0.50f;
    memset(&fs,0,sizeof(fs)); fs.active=0; fs.mode=SCREEN_LOGIN;

    while(running&&!logged_in){
        ALLEGRO_EVENT ev; al_wait_for_event(queue,&ev);

        if(ev.type==ALLEGRO_EVENT_KEY_CHAR){
            char *buf=NULL; int l;
            if(fs.mode==SCREEN_LOGIN){ if(fs.active==0) buf=fs.user; if(fs.active==1) buf=fs.pass; }
            else { if(fs.active==2) buf=fs.ru; if(fs.active==3) buf=fs.rp; if(fs.active==4) buf=fs.rp2; }
            if(ev.keyboard.keycode==ALLEGRO_KEY_BACKSPACE&&buf){ l=strlen(buf); if(l>0) buf[l-1]='\0'; }
            else if(ev.keyboard.keycode==ALLEGRO_KEY_TAB){
                if(fs.mode==SCREEN_LOGIN) fs.active=1-fs.active;
                else { fs.active++; if(fs.active>4) fs.active=2; }
            }
            else if(ev.keyboard.keycode==ALLEGRO_KEY_ENTER&&fs.mode==SCREEN_LOGIN){
                if(verifier(fs.user,fs.pass)){ strncpy(logged_user,fs.user,63); logged_in=true; }
                else { fs.err=true; strcpy(fs.msg,"Identifiant ou mot de passe incorrect"); memset(fs.pass,0,sizeof(fs.pass)); }
            }
            else if(buf&&ev.keyboard.unichar>=32){ l=strlen(buf); if(l<62){buf[l]=(char)ev.keyboard.unichar;buf[l+1]='\0';} fs.err=false;fs.rerr=false; }
        }

        if(ev.type==ALLEGRO_EVENT_MOUSE_BUTTON_DOWN&&tick>=PHASE_TURN){
            float mx=(float)ev.mouse.x,my=(float)ev.mouse.y;
            if(fs.mode==SCREEN_LOGIN){
                float pw=400,ph=400,x0=panx-pw/2,y0=pany-ph/2;
                if(in_rect(mx,my,x0+16,y0+112,pw-32,44)) fs.active=0;
                if(in_rect(mx,my,x0+16,y0+204,pw-32,44)) fs.active=1;
                if(in_rect(mx,my,x0+8,y0+ph-106,pw-16,46)){
                    if(verifier(fs.user,fs.pass)){ strncpy(logged_user,fs.user,63); logged_in=true; }
                    else { fs.err=true; strcpy(fs.msg,"Identifiant ou mot de passe incorrect"); memset(fs.pass,0,sizeof(fs.pass)); }
                }
                if(in_rect(mx,my,x0+20,y0+ph-42,pw-40,32)){
                    fs.mode=SCREEN_REGISTER; fs.active=2; fs.rerr=false; fs.rok=false;
                    memset(fs.ru,0,sizeof(fs.ru)); memset(fs.rp,0,sizeof(fs.rp)); memset(fs.rp2,0,sizeof(fs.rp2));
                }
            } else {
                float pw=400,ph=492,x0=panx-pw/2,y0=pany-ph/2;
                if(in_rect(mx,my,x0+16,y0+110,pw-32,44)) fs.active=2;
                if(in_rect(mx,my,x0+16,y0+196,pw-32,44)) fs.active=3;
                if(in_rect(mx,my,x0+16,y0+282,pw-32,44)) fs.active=4;
                if(in_rect(mx,my,x0+8,y0+ph-106,pw-16,46)){
                    if(strlen(fs.ru)<3){ fs.rerr=true; strcpy(fs.msg,"Identifiant trop court (min 3)"); }
                    else if(strlen(fs.rp)<4){ fs.rerr=true; strcpy(fs.msg,"Mot de passe trop court (min 4)"); }
                    else if(strcmp(fs.rp,fs.rp2)!=0){ fs.rerr=true; strcpy(fs.msg,"Mots de passe differents"); }
                    else if(existe_compte(fs.ru)){ fs.rerr=true; strcpy(fs.msg,"Identifiant deja utilise"); }
                    else if(creer_compte(fs.ru,fs.rp)){ fs.rerr=false; fs.rok=true; strncpy(fs.user,fs.ru,63); memset(fs.pass,0,sizeof(fs.pass)); }
                }
                if(in_rect(mx,my,x0+20,y0+ph-48,pw-40,32)){ fs.mode=SCREEN_LOGIN; fs.active=0; fs.rok=false; }
            }
        }
        if(ev.type==ALLEGRO_EVENT_DISPLAY_CLOSE) running=false;

        if(ev.type==ALLEGRO_EVENT_TIMER){
            float pilot_x, panel_x, scale;
            int   frame;
            float sag, hx, hy, pax2, pay2;

            tick++;

            /* Position pilote */
            if(tick<PHASE_WALK){
                float t=(float)tick/PHASE_WALK, ease=1.0f-(1.0f-t)*(1.0f-t);
                pilot_x=-220.0f+ease*(pfx+220.0f);
            } else { pilot_x=pfx; }

            /* Position panneau */
            if(tick<PHASE_WALK){
                panel_x=(float)SCR_W+500.0f;
            } else {
                float t2=(float)(tick-PHASE_WALK)/(PHASE_PULL-PHASE_WALK), ease2;
                if(t2>1.0f) t2=1.0f;
                ease2=1.0f-(1.0f-t2)*(1.0f-t2)*(1.0f-t2);
                panel_x=(float)SCR_W+500.0f-ease2*((float)SCR_W+500.0f-panx);
            }

            /* Selection frame */
            if(tick<PHASE_WALK){
                frame=(tick/SPRITE_SPEED)%SMOOTH_COUNT; scale=0.45f;
            } else if(tick<PHASE_PULL){
                frame=SPR_PULL; scale=0.45f;
            } else if(tick<PHASE_TURN){
                frame=0; scale=0.45f;
            } else {
                frame=SPR_FRONT; scale=0.50f;
            }

            draw_bg(tick);

            /* Panneau */
            if(tick>=PHASE_WALK){
                if(fs.mode==SCREEN_LOGIN) draw_login_panel(panel_x,pany,fb,fn,&fs);
                else draw_register_panel(panel_x,pany,fb,fn,&fs);
            }

            /* Pilote */
            if(has_spr&&sprites[frame]){
                float bw2=(float)al_get_bitmap_width(sprites[frame]);
                float bh2=(float)al_get_bitmap_height(sprites[frame]);
                float dw=bw2*scale, dh=bh2*scale;
                al_draw_scaled_bitmap(sprites[frame],0,0,bw2,bh2,pilot_x-dw/2,pfy-dh,dw,dh,0);
            }

            /* Corde */
            if(tick>=PHASE_WALK&&tick<PHASE_PULL+12){
                float spr_w=380.0f*scale, spr_h=440.0f*scale;
                float corner_x=pilot_x-spr_w/2.0f, corner_y=pfy-spr_h;
                hx=corner_x+0.57f*spr_w; hy=corner_y+0.61f*spr_h;
                pax2=panel_x-195.0f; pay2=pany;
                sag=(pax2-hx)*0.05f;
                if(sag<2) sag=2; if(sag>55) sag=55;
                draw_rope(hx,hy,pax2,pay2,sag);
            }

            if(tick>=PHASE_TURN)
                al_draw_text(fn,al_map_rgba(100,122,168,148),SCR_W/2,SCR_H-20,ALLEGRO_ALIGN_CENTRE,"TAB = champ suivant  |  ENTREE = valider");
            al_flip_display();
        }
    }

    if(logged_in){
        int i; char wmsg[80]; snprintf(wmsg,sizeof(wmsg),"BIENVENUE, %s !",logged_user);
        for(i=0;i<200;i++){
            float a=(i<60)?(float)i/60.0f:1.0f,prog=(float)i/200.0f; int av=(int)(a*255),s;
            al_draw_filled_rectangle(0,0,SCR_W,SCR_H,al_map_rgb(6,9,22));
            srand(99); for(s=0;s<120;s++){ int sx=rand()%SCR_W,sy=rand()%SCR_H; al_draw_filled_circle(sx,sy,1,al_map_rgba(182,196,240,(rand()%110)+68)); }
            al_draw_filled_circle(SCR_W/2,SCR_H/2,80+i/3,al_map_rgba(18,48,142,(int)(24*a)));
            al_draw_text(fb,al_map_rgba(205,165,48,av),SCR_W/2,SCR_H/2-28,ALLEGRO_ALIGN_CENTRE,wmsg);
            al_draw_text(fn,al_map_rgba(132,175,242,av),SCR_W/2,SCR_H/2+10,ALLEGRO_ALIGN_CENTRE,"Chargement du systeme AERO-CONCEPT...");
            al_draw_filled_rounded_rectangle(SCR_W/2-180,SCR_H/2+46,SCR_W/2-180+360*prog,SCR_H/2+64,4,4,al_map_rgb(26,82,212));
            al_draw_rounded_rectangle(SCR_W/2-180,SCR_H/2+46,SCR_W/2+180,SCR_H/2+64,4,4,al_map_rgb(52,92,192),1.5f);
            al_flip_display(); al_rest(1.0/60.0);
        }
    }
    (void)disp; return logged_in;
}

/* ======= MENU PAGES ======= */
static void valider_custom(AppState *st){
    float masse,surface,clmax,rho,cd0,k,poussee;
    if(!strlen(st->inp_nom)){strcpy(st->msg,"Nom manquant !");st->msg_ok=false;st->msg_timer=120;return;}
    masse=(float)atof(st->inp_masse);surface=(float)atof(st->inp_surface);
    clmax=(float)atof(st->inp_clmax);rho=(float)atof(st->inp_rho);
    cd0=(float)atof(st->inp_cd0);k=(float)atof(st->inp_k);poussee=(float)atof(st->inp_poussee);
    if(masse<=0||surface<=0||clmax<=0||rho<=0||cd0<=0||k<=0||poussee<=0){
        strcpy(st->msg,"Toutes les valeurs doivent etre > 0 !");st->msg_ok=false;st->msg_timer=120;return;
    }
    strncpy(st->avion.nom,st->inp_nom,39);
    st->avion.masse=masse;st->avion.surface=surface;st->avion.clmax=clmax;
    st->avion.rho=rho;st->avion.cd0=cd0;st->avion.k=k;st->avion.poussee=poussee;st->avion.existe=1;
    strcpy(st->msg,"Avion enregistre !");st->msg_ok=true;st->msg_timer=120;
}

static void draw_menu_page(ALLEGRO_FONT *fb,ALLEGRO_FONT *fn,AppState *st,float mx,float my,int tick){
    int i; float bw=380,bh=56,bx=(SCR_W-bw)/2,by=120,gap=66;
    const char *labels[6]={"  Gestion des donnees avion","  Analyse et calcul des performances","  Exportation des resultats","  Aide et informations","  Jeu de Chasse F-16","  Quitter"};
    int cols[6][3]={{25,80,210},{18,118,58},{140,80,20},{80,40,160},{160,30,100},{160,30,30}};
    int bords[6][3]={{70,140,255},{50,205,100},{210,140,50},{140,80,220},{220,60,140},{220,60,60}};
    Btn b;
    draw_titre(fb,fn,"AERO-CONCEPT","Analyse et simulation des performances");
    if(st->avion.existe){
        char info[80]; snprintf(info,sizeof(info),"Avion actif : %s  |  Masse : %.0f kg  |  Poussee : %.0f N",st->avion.nom,st->avion.masse,st->avion.poussee);
        al_draw_filled_rounded_rectangle(50,92,SCR_W-50,118,6,6,al_map_rgba(20,50,30,180));
        al_draw_text(fn,al_map_rgb(80,220,100),SCR_W/2,98,ALLEGRO_ALIGN_CENTRE,info);
    } else {
        al_draw_filled_rounded_rectangle(50,92,SCR_W-50,118,6,6,al_map_rgba(80,20,20,180));
        al_draw_text(fn,al_map_rgb(220,100,80),SCR_W/2,98,ALLEGRO_ALIGN_CENTRE,"Aucun avion selectionne - Allez dans Gestion des donnees");
    }
    for(i=0;i<6;i++){
        b.x=bx;b.y=by+i*gap;b.w=bw;b.h=bh;strncpy(b.label,labels[i],49);
        b.r=cols[i][0];b.g=cols[i][1];b.b=cols[i][2];b.rb=bords[i][0];b.gb=bords[i][1];b.bb=bords[i][2];
        draw_btn(fn,b,mx,my);
    }
    { float ax=fmodf(tick*1.5f,SCR_W+100)-50,ay=300+sinf(tick*0.04f)*20;
      al_draw_filled_ellipse(ax,ay,22,8,al_map_rgba(200,210,240,60));
      al_draw_filled_triangle(ax-22,ay,ax+22,ay,ax-5,ay-12,al_map_rgba(200,210,240,60)); }
    al_draw_text(fn,al_map_rgba(100,120,168,140),SCR_W/2,SCR_H-20,ALLEGRO_ALIGN_CENTRE,"Clic sur un bouton pour naviguer  |  ESC = retour");
}

static void draw_gestion_page(ALLEGRO_FONT *fb,ALLEGRO_FONT *fn,AppState *st,float mx,float my){
    float bw=340,bh=52,col=(SCR_W-bw)/2; Btn b;
    draw_titre(fb,fn,"GESTION DES DONNEES","Selectionnez une option");
    b.x=col;b.y=130;b.w=bw;b.h=bh;strcpy(b.label,"Choisir un avion predefined");b.r=25;b.g=80;b.b=210;b.rb=70;b.gb=140;b.bb=255;draw_btn(fn,b,mx,my);
    b.y=200;strcpy(b.label,"Entrer un avion personnalise");b.r=18;b.g=118;b.b=58;b.rb=50;b.gb=205;b.bb=100;draw_btn(fn,b,mx,my);
    b.y=270;strcpy(b.label,"Consulter les donnees");b.r=100;b.g=60;b.b=180;b.rb=160;b.gb=110;b.bb=240;draw_btn(fn,b,mx,my);
    b.y=340;strcpy(b.label,"Exporter les donnees");b.r=140;b.g=80;b.b=20;b.rb=210;b.gb=140;b.bb=50;draw_btn(fn,b,mx,my);
    b.y=430;strcpy(b.label,"< Retour menu principal");b.r=50;b.g=50;b.b=80;b.rb=80;b.gb=80;b.bb=120;draw_btn(fn,b,mx,my);
    if(st->avion.existe){ char info[80]; snprintf(info,sizeof(info),"Avion actif : %s",st->avion.nom);
        al_draw_filled_rounded_rectangle(col,500,col+bw,526,6,6,al_map_rgba(20,60,20,180));
        al_draw_text(fn,al_map_rgb(80,220,100),SCR_W/2,506,ALLEGRO_ALIGN_CENTRE,info); }
    (void)fb;
}

static void draw_liste_page(ALLEGRO_FONT *fb,ALLEGRO_FONT *fn,AppState *st,float mx,float my){
    int i; float lx=80,ly=100,lw=SCR_W-160,row=30; int vis=14; Btn bv,br;
    draw_titre(fb,fn,"CHOISIR UN AVION","Cliquez sur un avion puis Selectionner");
    al_draw_filled_rounded_rectangle(lx,ly,lx+lw,ly+vis*row+10,8,8,al_map_rgba(10,16,40,200));
    al_draw_rounded_rectangle(lx,ly,lx+lw,ly+vis*row+10,8,8,al_map_rgba(60,90,160,160),1.5f);
    for(i=0;i<vis&&(i+st->liste_scroll)<NB_AVIONS;i++){
        int idx=i+st->liste_scroll; float ry=ly+5+i*row;
        bool sel=(idx==st->liste_sel),hov=in_rect(mx,my,lx+4,ry,lw-8,row-2);
        char ligne[80];
        if(sel) al_draw_filled_rounded_rectangle(lx+4,ry,lx+lw-4,ry+row-2,4,4,al_map_rgba(25,80,210,180));
        else if(hov) al_draw_filled_rounded_rectangle(lx+4,ry,lx+lw-4,ry+row-2,4,4,al_map_rgba(40,55,100,120));
        snprintf(ligne,sizeof(ligne),"%2d.  %-22s   Masse: %5.0f kg   Poussee: %5.0f N",idx+1,avions_pred[idx].nom,avions_pred[idx].masse,avions_pred[idx].poussee);
        al_draw_text(fn,sel?al_map_rgb(255,255,255):al_map_rgb(180,195,230),lx+12,ry+8,ALLEGRO_ALIGN_LEFT,ligne);
    }
    if(st->liste_scroll>0) al_draw_text(fn,al_map_rgb(120,140,200),SCR_W/2,ly-18,ALLEGRO_ALIGN_CENTRE,"Molette pour scroller");
    if(st->liste_scroll+vis<NB_AVIONS) al_draw_text(fn,al_map_rgb(120,140,200),SCR_W/2,ly+vis*row+18,ALLEGRO_ALIGN_CENTRE,"Molette pour scroller");
    bv.x=SCR_W/2-200;bv.y=SCR_H-80;bv.w=180;bv.h=44;strcpy(bv.label,"Selectionner");bv.r=25;bv.g=80;bv.b=210;bv.rb=70;bv.gb=140;bv.bb=255;draw_btn(fn,bv,mx,my);
    br.x=SCR_W/2+20;br.y=SCR_H-80;br.w=180;br.h=44;strcpy(br.label,"< Retour");br.r=50;br.g=50;br.b=80;br.rb=80;br.gb=80;br.bb=120;draw_btn(fn,br,mx,my);
    (void)fb;
}

static void draw_custom_page(ALLEGRO_FONT *fb,ALLEGRO_FONT *fn,AppState *st,float mx,float my){
    float cx=120,cw=380,gap=62; Btn bv,br;
    draw_titre(fb,fn,"AVION PERSONNALISE","Remplissez les champs puis validez");
    draw_field(fn,"Nom de l avion",     st->inp_nom,    0,st->inp_active,cx,       120,     cw,false);
    draw_field(fn,"Masse (kg)",         st->inp_masse,  1,st->inp_active,cx,       120+gap, cw,false);
    draw_field(fn,"Surface alaire(m2)", st->inp_surface,2,st->inp_active,cx+cw+60, 120,     cw,false);
    draw_field(fn,"CLmax",              st->inp_clmax,  3,st->inp_active,cx+cw+60, 120+gap, cw,false);
    draw_field(fn,"Densite air(kg/m3)", st->inp_rho,    4,st->inp_active,cx,       120+gap*2,cw,false);
    draw_field(fn,"CD0",                st->inp_cd0,    5,st->inp_active,cx+cw+60, 120+gap*2,cw,false);
    draw_field(fn,"Facteur induit k",   st->inp_k,      6,st->inp_active,cx,       120+gap*3,cw,false);
    draw_field(fn,"Poussee (N)",        st->inp_poussee,7,st->inp_active,cx+cw+60, 120+gap*3,cw,false);
    al_draw_text(fn,al_map_rgb(120,140,180),cx,120+gap*4+10,ALLEGRO_ALIGN_LEFT,"TAB = champ suivant");
    bv.x=SCR_W/2-200;bv.y=SCR_H-80;bv.w=180;bv.h=44;strcpy(bv.label,"Valider");bv.r=18;bv.g=118;bv.b=58;bv.rb=50;bv.gb=205;bv.bb=100;draw_btn(fn,bv,mx,my);
    br.x=SCR_W/2+20;br.y=SCR_H-80;br.w=180;br.h=44;strcpy(br.label,"< Retour");br.r=50;br.g=50;br.b=80;br.rb=80;br.gb=80;br.bb=120;draw_btn(fn,br,mx,my);
    if(st->msg_timer>0){ ALLEGRO_COLOR mc=st->msg_ok?al_map_rgb(50,220,100):al_map_rgb(230,60,60); al_draw_text(fn,mc,SCR_W/2,SCR_H-105,ALLEGRO_ALIGN_CENTRE,st->msg); }
    (void)fb;
}

static void draw_voir_page(ALLEGRO_FONT *fb,ALLEGRO_FONT *fn,AppState *st,float mx,float my){
    float px=SCR_W/2-220,py=110,pw=440; Btn bsup,br;
    draw_titre(fb,fn,"DONNEES DE L AVION","");
    if(!st->avion.existe){
        al_draw_text(fn,al_map_rgb(230,60,60),SCR_W/2,300,ALLEGRO_ALIGN_CENTRE,"Aucun avion selectionne !");
    } else {
        float ph=8*36+68; int j;
        const char *keys[]={"Masse","Surface alaire","CLmax","Densite air","CD0","Facteur k","Poussee"};
        float vals[7]={st->avion.masse,st->avion.surface,st->avion.clmax,st->avion.rho,st->avion.cd0,st->avion.k,st->avion.poussee};
        const char *units[]={"kg","m2","","kg/m3","","","N"};
        char tmp[60];
        al_draw_filled_rounded_rectangle(px,py,px+pw,py+ph,12,12,al_map_rgba(12,18,50,220));
        al_draw_rounded_rectangle(px,py,px+pw,py+ph,12,12,al_map_rgba(70,120,220,180),2.0f);
        al_draw_filled_rounded_rectangle(px,py,px+pw,py+44,12,12,al_map_rgb(16,38,115));
        al_draw_filled_rectangle(px,py+28,px+pw,py+44,al_map_rgb(16,38,115));
        al_draw_text(fb,al_map_rgb(212,172,48),SCR_W/2,py+12,ALLEGRO_ALIGN_CENTRE,st->avion.nom);
        for(j=0;j<7;j++){
            float ry=py+58+j*36;
            if(j%2==0) al_draw_filled_rectangle(px+8,ry,px+pw-8,ry+32,al_map_rgba(20,30,60,80));
            snprintf(tmp,sizeof(tmp),"%.4g %s",vals[j],units[j]);
            al_draw_text(fn,al_map_rgb(150,170,220),px+20,ry+8,ALLEGRO_ALIGN_LEFT,keys[j]);
            al_draw_text(fn,al_map_rgb(228,235,255),px+pw-20,ry+8,ALLEGRO_ALIGN_RIGHT,tmp);
        }
        { float vs=vit_decrochage(st->avion),ry=py+58+7*36;
          al_draw_filled_rectangle(px+8,ry,px+pw-8,ry+32,al_map_rgba(20,60,20,100));
          snprintf(tmp,sizeof(tmp),"%.1f m/s  (%.1f km/h)",vs,vs*3.6f);
          al_draw_text(fn,al_map_rgb(80,220,100),px+20,ry+8,ALLEGRO_ALIGN_LEFT,"Vit. decrochage");
          al_draw_text(fn,al_map_rgb(80,220,100),px+pw-20,ry+8,ALLEGRO_ALIGN_RIGHT,tmp); }
    }
    bsup.x=SCR_W/2-200;bsup.y=SCR_H-80;bsup.w=180;bsup.h=44;strcpy(bsup.label,"Supprimer");bsup.r=160;bsup.g=30;bsup.b=30;bsup.rb=220;bsup.gb=60;bsup.bb=60;draw_btn(fn,bsup,mx,my);
    br.x=SCR_W/2+20;br.y=SCR_H-80;br.w=180;br.h=44;strcpy(br.label,"< Retour");br.r=50;br.g=50;br.b=80;br.rb=80;br.gb=80;br.bb=120;draw_btn(fn,br,mx,my);
    (void)fb;
}

static void draw_analyse_page(ALLEGRO_FONT *fb,ALLEGRO_FONT *fn,AppState *st,float mx,float my){
    float bw=340,bx=(SCR_W-bw)/2; Btn bd,br;
    draw_titre(fb,fn,"ANALYSE DES PERFORMANCES","");
    if(!st->avion.existe){
        al_draw_text(fn,al_map_rgb(230,60,60),SCR_W/2,280,ALLEGRO_ALIGN_CENTRE,"Aucun avion selectionne !");
    } else {
        char info[100]; float vs=vit_decrochage(st->avion),d=calc_trainee(st->avion,vs*1.3f);
        al_draw_filled_rounded_rectangle(bx-20,110,bx+bw+20,280,10,10,al_map_rgba(12,18,50,200));
        al_draw_rounded_rectangle(bx-20,110,bx+bw+20,280,10,10,al_map_rgba(60,100,200,150),1.5f);
        snprintf(info,sizeof(info),"Avion : %s",st->avion.nom);al_draw_text(fn,al_map_rgb(212,172,48),SCR_W/2,122,ALLEGRO_ALIGN_CENTRE,info);
        snprintf(info,sizeof(info),"Vitesse decrochage : %.1f m/s  (%.0f km/h)",vs,vs*3.6f);al_draw_text(fn,al_map_rgb(80,220,100),SCR_W/2,152,ALLEGRO_ALIGN_CENTRE,info);
        snprintf(info,sizeof(info),"Trainee a 1.3Vs : %.0f N",d);al_draw_text(fn,al_map_rgb(180,210,255),SCR_W/2,182,ALLEGRO_ALIGN_CENTRE,info);
        snprintf(info,sizeof(info),"Poussee moteur : %.0f N",st->avion.poussee);al_draw_text(fn,al_map_rgb(180,210,255),SCR_W/2,212,ALLEGRO_ALIGN_CENTRE,info);
        if(st->avion.poussee>calc_trainee(st->avion,vs)) al_draw_text(fn,al_map_rgb(50,220,80),SCR_W/2,248,ALLEGRO_ALIGN_CENTRE,"Decollage POSSIBLE");
        else al_draw_text(fn,al_map_rgb(230,60,60),SCR_W/2,248,ALLEGRO_ALIGN_CENTRE,"Decollage IMPOSSIBLE");
    }
    bd.x=bx;bd.y=310;bd.w=bw;bd.h=52;strcpy(bd.label,"Lancer l animation decollage");bd.r=25;bd.g=80;bd.b=210;bd.rb=70;bd.gb=140;bd.bb=255;draw_btn(fn,bd,mx,my);
    br.x=bx;br.y=380;br.w=bw;br.h=52;strcpy(br.label,"< Retour menu principal");br.r=50;br.g=50;br.b=80;br.rb=80;br.gb=80;br.bb=120;draw_btn(fn,br,mx,my);
    (void)fb;
}

/* â”€â”€ Dessine un vrai avion 2D en vol a la position (cx,cy), angle en radians â”€â”€ */
static void draw_avion_2d(float cx, float cy, float angle, ALLEGRO_COLOR col)
{
    /* Fuselage */
    float cos_a = cosf(angle), sin_a = sinf(angle);
    float lx = cx - 28*cos_a, ly = cy - 28*sin_a;
    float rx = cx + 28*cos_a, ry = cy + 28*sin_a;
    al_draw_line(lx,ly,rx,ry,col,7);
    /* Nez arrondi */
    al_draw_filled_circle(rx,ry,5,col);
    /* Aile principale */
    float wx1 = cx - 4*cos_a + 18*sin_a, wy1 = cy - 4*sin_a - 18*cos_a;
    float wx2 = cx - 4*cos_a - 18*sin_a, wy2 = cy - 4*sin_a + 18*cos_a;
    float wx3 = cx + 12*cos_a,            wy3 = cy + 12*sin_a;
    al_draw_filled_triangle(wx1,wy1,wx2,wy2,wx3,wy3,col);
    /* Derive (petite aile arriere) */
    float dx1 = lx + 6*cos_a + 9*sin_a,  dy1 = ly + 6*sin_a - 9*cos_a;
    float dx2 = lx + 6*cos_a - 4*sin_a,  dy2 = ly + 6*sin_a + 4*cos_a;
    float dx3 = lx + 14*cos_a,            dy3 = ly + 14*sin_a;
    al_draw_filled_triangle(dx1,dy1,dx2,dy2,dx3,dy3,col);
    /* Cockpit */
    al_draw_filled_circle(cx+10*cos_a,cy+10*sin_a,4,al_map_rgba(150,220,255,200));
    /* Trainee de vapeur (petit effet) */
    al_draw_line(lx,ly,lx-14*cos_a,ly-14*sin_a,al_map_rgba(255,255,255,60),3);
}

/*
 * Simule la physique reelle du decollage pas a pas.
 * Retourne la position (dist, alt) au step f parmi TOTAL.
 * La simulation depend entierement des parametres de l'avion.
 *
 * Phases :
 *  1) Roulage  : acceleration au sol, F_net = T - D - friction*W
 *  2) Rotation : a Vs, l'avion commence a monter
 *  3) Montee   : portance > poids => montee selon angle de montee
 */
static void sim_position(Avion a, float Vs, int f, int TOTAL,
                          float *out_dist, float *out_alt,
                          float *out_V,    float *out_angle)
{
    float W      = a.masse * G_CONST;
    float mu     = 0.02f;
    /* dt fixe a 0.5s - fonctionne bien avec CL borne */
    float dt = 0.5f;

    float V      = 5.0f;   /* vitesse initiale non nulle */
    float dist   = 0.0f;
    float alt    = 0.0f;
    float vz     = 0.0f;
    float angle  = 0.0f;
    int   i;

    for (i = 0; i < f && i < TOTAL; i++) {
        float L, D, Fneta, Fnetv;

        if (alt < 0.05f) {
            /* â”€â”€ Phase roulage â”€â”€ */
            D     = calc_trainee(a, V);
            Fneta = a.poussee - D - mu * W;
            if (Fneta < 0.0f) Fneta = 0.0f;
            V    += (Fneta / a.masse) * dt;

            /* Decollage quand V >= Vs */
            if (V >= Vs) {
                alt = 0.5f;
                vz  = V * 0.08f;   /* vitesse verticale initiale */
            }
            dist += V * dt;

        } else {
            /* â”€â”€ Phase montee â”€â”€ */
            /* Portance avec CL = 0.8*CLmax (montee efficace) */
            L     = 0.5f * a.rho * a.surface * (a.clmax * 0.80f) * V * V;
            D     = calc_trainee(a, V);

            /* Acceleration horizontale reduite (energie vers montee) */
            Fneta = (a.poussee - D) * 0.6f;
            V    += (Fneta / a.masse) * dt;
            if (V < Vs * 0.9f) V = Vs * 0.9f;  /* pas moins que 0.9*Vs */

            /* Acceleration verticale */
            Fnetv = L - W;
            vz   += (Fnetv / a.masse) * dt * 0.5f;
            /* Vitesse verticale bornee : min physique, max 20% de V */
            if (vz < V * 0.04f) vz = V * 0.04f;
            if (vz > V * 0.25f) vz = V * 0.25f;

            alt  += vz * dt;
            dist += V  * dt;

            /* Angle de montee reel */
            angle = atan2f(vz, V);
            if (angle > 0.45f) angle = 0.45f;
        }
    }
    *out_dist  = dist;
    *out_alt   = alt;
    *out_V     = V;
    *out_angle = angle;
}

static void draw_decollage_page(ALLEGRO_FONT *fb,ALLEGRO_FONT *fn,AppState *st,float mx,float my)
{
    float gx=80,gy=95,rw=SCR_W-160,rh=415;
    int   TOTAL=160;
    Btn   br;

    draw_titre(fb,fn,"ANIMATION DECOLLAGE","");

    if (!st->avion.existe || !st->anim_possible) {
        al_draw_text(fn,al_map_rgb(230,60,60),SCR_W/2,300,ALLEGRO_ALIGN_CENTRE,
                     "Decollage impossible ou aucun avion selectionne.");
        br.x=SCR_W/2-90;br.y=SCR_H-80;br.w=180;br.h=44;
        strcpy(br.label,"< Retour");br.r=50;br.g=50;br.b=80;br.rb=80;br.gb=80;br.bb=120;
        draw_btn(fn,br,mx,my); return;
    }

    /* â”€â”€ Zone graphique â”€â”€ */
    al_draw_filled_rounded_rectangle(gx,gy,gx+rw,gy+rh,8,8,al_map_rgba(6,10,24,220));
    al_draw_rounded_rectangle(gx,gy,gx+rw,gy+rh,8,8,al_map_rgba(50,80,160,180),1.5f);

    /* Sol (ligne verte) */
    float sol_y = gy + rh - 28;
    al_draw_filled_rectangle(gx+4, sol_y+2, gx+rw-4, sol_y+8, al_map_rgb(30,100,30));
    al_draw_line(gx+36, sol_y, gx+rw-10, sol_y, al_map_rgb(60,180,60), 1.5f);

    /* Axes */
    al_draw_line(gx+36, gy+10, gx+36, sol_y+2, al_map_rgb(70,85,120), 1.5f);
    al_draw_text(fn,al_map_rgb(100,120,180),gx+rw/2,gy+rh-10,ALLEGRO_ALIGN_CENTRE,"Distance (m)");
    al_draw_text(fn,al_map_rgb(100,120,180),gx+12,gy+rh/2-10,ALLEGRO_ALIGN_LEFT,"Alt");

    /* â”€â”€ Calcul echelle : on simule le vol complet pour trouver les max â”€â”€ */
    {
        float max_dist=1, max_alt=1;
        float d2,a2,v2,ang2;
        int   f;
        for (f=1; f<=TOTAL; f++) {
            sim_position(st->avion,st->anim_Vs,f,TOTAL,&d2,&a2,&v2,&ang2);
            if (d2>max_dist) max_dist=d2;
            if (a2>max_alt)  max_alt=a2;
        }
        if (max_alt < 1.0f) max_alt = 1.0f;

        float zone_w = rw - 50;
        float zone_h = rh - 50;

        /* â”€â”€ Trace la trajectoire â”€â”€ */
        for (f=1; f<=st->anim_frame && f<=TOTAL; f++) {
            float fd,fa,fv,fang;
            float fx,fy2;
            ALLEGRO_COLOR tc;
            float fD,fm;

            sim_position(st->avion,st->anim_Vs,f,TOTAL,&fd,&fa,&fv,&fang);
            fx  = gx+36 + (fd/max_dist)*zone_w;
            fy2 = sol_y  - (fa/max_alt)*zone_h;

            fD  = calc_trainee(st->avion, fv < 1 ? 1 : fv);
            fm  = st->avion.poussee - fD;

            if (fm>400)      tc=al_map_rgba(50,220,80,160);
            else if (fm>100) tc=al_map_rgba(220,200,50,160);
            else             tc=al_map_rgba(220,60,60,160);

            al_draw_filled_circle(fx, fy2, 2, tc);
        }

        /* â”€â”€ Position et dessin avion courant â”€â”€ */
        if (st->anim_frame <= TOTAL) {
            float dist,alt,V,angle;
            float ax2,ay2;
            float D,marge;
            ALLEGRO_COLOR pc;
            char  info[100];

            sim_position(st->avion,st->anim_Vs,st->anim_frame,TOTAL,
                         &dist,&alt,&V,&angle);

            ax2 = gx+36 + (dist/max_dist)*zone_w;
            ay2 = sol_y  - (alt/max_alt)*zone_h;

            D     = calc_trainee(st->avion, V < 1 ? 1 : V);
            marge = st->avion.poussee - D;

            if (marge>400)      pc=al_map_rgb(50,220,80);
            else if (marge>100) pc=al_map_rgb(220,200,50);
            else                pc=al_map_rgb(220,60,60);

            /* Dessin avion 2D avec angle de montee */
            draw_avion_2d(ax2, ay2, -angle, pc);

            /* Piste (marquages) */
            {
                int m;
                for (m=0; m<8; m++) {
                    float mx2 = gx+50 + m*60;
                    if (mx2 < ax2-20)
                        al_draw_filled_rectangle(mx2,sol_y+2,mx2+30,sol_y+6,
                                                 al_map_rgb(180,155,35));
                }
            }

            snprintf(info,sizeof(info),
                     "V=%.0f km/h  |  Alt=%.0f m  |  Marge T-D=%.0f N  |  Avion: %s",
                     V*3.6f, alt, marge, st->avion.nom);
            al_draw_text(fn,pc,SCR_W/2,gy+rh+8,ALLEGRO_ALIGN_CENTRE,info);

            /* Axes graduation dynamique */
            {
                char tmp[20];
                snprintf(tmp,sizeof(tmp),"%.0fm",max_dist);
                al_draw_text(fn,al_map_rgb(80,100,160),gx+rw-10,sol_y+4,ALLEGRO_ALIGN_RIGHT,tmp);
                snprintf(tmp,sizeof(tmp),"%.0fm",max_alt);
                al_draw_text(fn,al_map_rgb(80,100,160),gx+14,gy+18,ALLEGRO_ALIGN_LEFT,tmp);
            }
        }
    }

    /* Legende */
    al_draw_filled_circle(gx+60, gy+rh+30,5,al_map_rgb(50,220,80));
    al_draw_text(fn,al_map_rgb(150,170,200),gx+72, gy+rh+23,ALLEGRO_ALIGN_LEFT,"Marge bonne");
    al_draw_filled_circle(gx+210,gy+rh+30,5,al_map_rgb(220,200,50));
    al_draw_text(fn,al_map_rgb(150,170,200),gx+222,gy+rh+23,ALLEGRO_ALIGN_LEFT,"Marge faible");
    al_draw_filled_circle(gx+360,gy+rh+30,5,al_map_rgb(220,60,60));
    al_draw_text(fn,al_map_rgb(150,170,200),gx+372,gy+rh+23,ALLEGRO_ALIGN_LEFT,"Marge critique");

    br.x=SCR_W-200;br.y=SCR_H-70;br.w=160;br.h=40;
    strcpy(br.label,"< Retour");br.r=50;br.g=50;br.b=80;br.rb=80;br.gb=80;br.bb=120;
    draw_btn(fn,br,mx,my);
    (void)fb;
}

/* ======= PAGE AIDE ======= */
static void draw_aide_page(ALLEGRO_FONT *fb, ALLEGRO_FONT *fn, float mx, float my)
{
    float px = 80, py = 100, pw = SCR_W - 160;
    Btn br, bvis;

    draw_titre(fb, fn, "AIDE & INFORMATIONS", "Guide d utilisation d AERO-CONCEPT");

    al_draw_filled_rounded_rectangle(px, py, px+pw, py+320, 10, 10, al_map_rgba(10, 16, 48, 210));
    al_draw_rounded_rectangle(px, py, px+pw, py+320, 10, 10, al_map_rgba(60, 100, 200, 150), 1.5f);

    float tx = px + 24, ty = py + 14, ls = 28.0f;
    ALLEGRO_COLOR ch = al_map_rgb(212, 172, 48);
    ALLEGRO_COLOR ct = al_map_rgb(180, 200, 240);

    al_draw_text(fb, ch, tx, ty,               ALLEGRO_ALIGN_LEFT, "1. Gestion avion");
    al_draw_text(fn, ct, tx+16, ty+ls,         ALLEGRO_ALIGN_LEFT, "Selectionnez un avion predifini ou creez un avion personnalise.");

    al_draw_text(fb, ch, tx, ty+ls*2+8,        ALLEGRO_ALIGN_LEFT, "2. Analyse performances");
    al_draw_text(fn, ct, tx+16, ty+ls*3+8,     ALLEGRO_ALIGN_LEFT, "Calcule la vitesse de decrochage, la trainee et la faisabilite du decollage.");

    al_draw_text(fb, ch, tx, ty+ls*4+16,       ALLEGRO_ALIGN_LEFT, "3. Animation decollage");
    al_draw_text(fn, ct, tx+16, ty+ls*5+16,    ALLEGRO_ALIGN_LEFT, "Visualise la trajectoire de decollage simulee avec les parametres de l avion.");

    al_draw_text(fb, ch, tx, ty+ls*6+24,       ALLEGRO_ALIGN_LEFT, "4. Visionneuse 3D");
    al_draw_text(fn, ct, tx+16, ty+ls*7+24,    ALLEGRO_ALIGN_LEFT, "Rotation interactive de l avion. Survolez les points pour identifier les parties.");

    al_draw_text(fb, ch, tx, ty+ls*8+32,       ALLEGRO_ALIGN_LEFT, "5. Parametres aerodynamiques");
    al_draw_text(fn, ct, tx+16, ty+ls*9+32,    ALLEGRO_ALIGN_LEFT, "Masse(kg), Surface(m2), CLmax, Rho(kg/m3), CD0, k, Poussee(N).");

    /* Bouton visionneuse 3D */
    bvis.x = SCR_W/2 - 180; bvis.y = py + 358; bvis.w = 360; bvis.h = 46;
    strcpy(bvis.label, "Ouvrir la Visionneuse 3D");
    bvis.r=18; bvis.g=68; bvis.b=148; bvis.rb=50; bvis.gb=120; bvis.bb=220;
    draw_btn(fn, bvis, mx, my);

    /* Bouton retour */
    br.x = SCR_W/2 - 90; br.y = SCR_H - 58; br.w = 180; br.h = 44;
    strcpy(br.label, "< Retour menu");
    br.r=50; br.g=50; br.b=80; br.rb=80; br.gb=80; br.bb=120;
    draw_btn(fn, br, mx, my);

    (void)fb;
}

/* ======= VISIONNEUSE 3D - 24 SPRITES ROTATION ======= */

#define VIS_SPRITES    24   /* 4 rangees x 6 colonnes */
#define VIS_SPEED       4   /* ticks par frame auto-rotation */

static ALLEGRO_BITMAP *vis_spr[VIS_SPRITES];
static bool vis_loaded = false;

static void vis_charger_sprites(void)
{
    int i;
    char path[64];
    if(vis_loaded) return;
    for(i=0;i<VIS_SPRITES;i++){
        snprintf(path,sizeof(path),"sprites/spr_%02d.png",i+1);
        vis_spr[i] = al_load_bitmap(path);
    }
    vis_loaded = true;
}

/* Parties de l avion : position relative sur sprite de face (sprite 0) */
typedef struct {
    const char *nom;
    const char *desc;
    float rx, ry;  /* 0..1 sur l image */
} PartieAvion;

#define NB_PARTIES 10
static const PartieAvion parties[NB_PARTIES]={
    {"Nez",                    "Pointe avant du fuselage",              0.13f,0.50f},
    {"Cockpit",                "Poste de pilotage et verriere",         0.22f,0.42f},
    {"Fuselage",               "Corps principal de l avion",            0.50f,0.45f},
    {"Aile gauche",            "Surface portante - genere la portance", 0.55f,0.72f},
    {"Aile droite",            "Surface portante - genere la portance", 0.55f,0.28f},
    {"Reacteur gauche",        "Turboreacteur - genere la poussee",     0.62f,0.65f},
    {"Derive verticale",       "Stabilisateur vertical - lacet",        0.82f,0.25f},
    {"Empennage horiz. G.",    "Stabilisateur - controle tangage",      0.85f,0.58f},
    {"Empennage horiz. D.",    "Stabilisateur - controle tangage",      0.85f,0.38f},
    {"Winglet",                "Saumon d aile - reduit la trainee",     0.78f,0.18f},
};

static void draw_visionneuse(ALLEGRO_FONT *fb, ALLEGRO_FONT *fn,
                              AppState *st, float mx, float my)
{
    int    i;
    Btn    br;
    char   lbl[80];
    float  vx=30, vy=88, vw=SCR_W-270, vh=SCR_H-108;

    draw_titre(fb,fn,"VISIONNEUSE 3D AVION",
               "Clic + glisser pour tourner  |  Survolez un point pour identifier");

    vis_charger_sprites();

    /* Zone viewer */
    al_draw_filled_rounded_rectangle(vx,vy,vx+vw,vy+vh,14,14,al_map_rgba(4,8,24,240));
    al_draw_rounded_rectangle(vx,vy,vx+vw,vy+vh,14,14,al_map_rgb(45,75,155),1.5f);

    /* Index sprite selon yaw normalise 0..23 */
    float yaw = st->vis_yaw;
    /* Normaliser yaw en 0..2PI */
    while(yaw <  0)        yaw += 2*M_PI;
    while(yaw >= 2*M_PI)   yaw -= 2*M_PI;
    int idx = (int)(yaw / (2*M_PI) * VIS_SPRITES) % VIS_SPRITES;

    /* Pitch : modifier la luminosite et un leger zoom */
    float pitch  = st->vis_pitch;
    float bright = 0.6f + 0.4f * cosf(pitch);
    float pscale = 1.0f - 0.15f * fabsf(sinf(pitch));
    int   bv     = (int)(bright * 255);

    /* Dessin sprite */
    if(vis_spr[idx]){
        float bw = (float)al_get_bitmap_width(vis_spr[idx]);
        float bh = (float)al_get_bitmap_height(vis_spr[idx]);

        /* Ajuster a la zone en gardant ratio */
        float max_w = (vw-40)*pscale;
        float max_h = (vh-40)*pscale;
        float ratio = bw/bh;
        float dw = max_w, dh = max_w/ratio;
        if(dh > max_h){ dh=max_h; dw=max_h*ratio; }

        float cx2 = vx+vw/2.0f;
        float cy2 = vy+vh/2.0f - sinf(pitch)*(vh*0.12f);
        float dx   = cx2-dw/2.0f;
        float dy   = cy2-dh/2.0f;

        /* Ombre */
        al_draw_filled_ellipse(cx2, vy+vh-18, dw*0.38f, 8*pscale,
                               al_map_rgba(0,0,0,55));

        /* Sprite avec teinte */
        al_draw_tinted_scaled_bitmap(vis_spr[idx],
            al_map_rgb(bv,bv,bv),
            0,0,bw,bh, dx,dy,dw,dh, 0);

        /* Points parties */
        float best_d = 55.0f;
        int   best   = -1;
        float pts_x[NB_PARTIES], pts_y[NB_PARTIES];

        for(i=0;i<NB_PARTIES;i++){
            pts_x[i] = dx + parties[i].rx * dw;
            pts_y[i] = dy + parties[i].ry * dh;
            float d = sqrtf((mx-pts_x[i])*(mx-pts_x[i])+(my-pts_y[i])*(my-pts_y[i]));
            if(d < best_d &&
               pts_x[i]>vx+4 && pts_x[i]<vx+vw-4 &&
               pts_y[i]>vy+4 && pts_y[i]<vy+vh-4){
                best_d=d; best=i;
            }
        }
        st->vis_hovered_part = best;

        /* Dessiner les points */
        for(i=0;i<NB_PARTIES;i++){
            if(pts_x[i]<vx+4||pts_x[i]>vx+vw-4) continue;
            if(pts_y[i]<vy+4||pts_y[i]>vy+vh-4) continue;
            bool sel=(i==best);
            float r=sel?8.0f:4.5f;
            ALLEGRO_COLOR fc=sel?al_map_rgb(255,220,40):al_map_rgba(80,180,255,200);
            al_draw_filled_circle(pts_x[i],pts_y[i],r,fc);
            al_draw_circle(pts_x[i],pts_y[i],r+1.5f,al_map_rgba(0,0,0,150),1.5f);
            if(sel){
                al_draw_circle(pts_x[i],pts_y[i],r+6, al_map_rgba(255,220,40,90),2.0f);
                al_draw_circle(pts_x[i],pts_y[i],r+12,al_map_rgba(255,220,40,35),1.5f);
            }
        }

        /* Tooltip */
        if(best>=0){
            snprintf(lbl,sizeof(lbl),"%s",parties[best].nom);
            float tw=(float)al_get_text_width(fn,lbl)+22;
            float tx=pts_x[best]+14, ty=pts_y[best]-30;
            if(tx+tw>vx+vw-10) tx=pts_x[best]-tw-14;
            if(ty<vy+5)        ty=pts_y[best]+14;
            al_draw_filled_rounded_rectangle(tx+2,ty+2,tx+tw+2,ty+26,8,8,al_map_rgba(0,0,0,100));
            al_draw_filled_rounded_rectangle(tx,ty,tx+tw,ty+26,8,8,al_map_rgba(8,16,48,240));
            al_draw_rounded_rectangle(tx,ty,tx+tw,ty+26,8,8,al_map_rgb(255,220,40),2.0f);
            al_draw_text(fn,al_map_rgb(255,245,160),tx+11,ty+5,ALLEGRO_ALIGN_LEFT,lbl);
            al_draw_line(pts_x[best],pts_y[best],
                         tx+(tx>pts_x[best]?0:tw),ty+13,
                         al_map_rgba(255,220,40,190),1.5f);
            /* Description en bas */
            al_draw_text(fn,al_map_rgb(140,180,220),
                         vx+vw/2,vy+vh-22,ALLEGRO_ALIGN_CENTRE,
                         parties[best].desc);
        } else {
            al_draw_text(fn,al_map_rgba(100,130,175,140),
                         vx+vw/2,vy+vh-22,ALLEGRO_ALIGN_CENTRE,
                         "Survolez un point bleu pour identifier la partie");
        }

    } else {
        al_draw_text(fn,al_map_rgb(200,80,80),vx+vw/2,vy+vh/2,
                     ALLEGRO_ALIGN_CENTRE,
                     "Sprites manquants : placez avion3d/*.png dans sprites/");
        st->vis_hovered_part=-1;
    }

    /* Panneau droit : liste des parties */
    float px2=vx+vw+8, py2=vy, pw2=SCR_W-px2-8, ph2=vh;
    al_draw_filled_rounded_rectangle(px2,py2,px2+pw2,py2+ph2,10,10,al_map_rgba(8,14,38,230));
    al_draw_rounded_rectangle(px2,py2,px2+pw2,py2+ph2,10,10,al_map_rgba(50,80,160,160),1.5f);
    al_draw_text(fn,al_map_rgb(180,200,240),px2+pw2/2,py2+10,ALLEGRO_ALIGN_CENTRE,"PARTIES");
    al_draw_line(px2+8,py2+28,px2+pw2-8,py2+28,al_map_rgb(40,70,140),1);
    for(i=0;i<NB_PARTIES;i++){
        float ry2=py2+36+i*34;
        bool sel=(i==st->vis_hovered_part);
        if(sel)
            al_draw_filled_rounded_rectangle(px2+4,ry2-2,px2+pw2-4,ry2+28,4,4,al_map_rgba(30,100,60,180));
        ALLEGRO_COLOR tc=sel?al_map_rgb(50,220,120):al_map_rgb(150,175,220);
        al_draw_filled_circle(px2+14,ry2+12,4,sel?al_map_rgb(50,220,120):al_map_rgba(80,180,255,200));
        al_draw_text(fn,tc,px2+24,ry2+4,ALLEGRO_ALIGN_LEFT,parties[i].nom);
        if(sel)
            al_draw_text(fn,al_map_rgb(120,190,150),px2+14,ry2+18,
                         ALLEGRO_ALIGN_LEFT,parties[i].desc);
    }

    /* Indicateur angle */
    {
        float icx=vx+vw-36, icy=vy+vh-36, ir=22;
        al_draw_filled_circle(icx,icy,ir,al_map_rgba(10,20,50,180));
        al_draw_circle(icx,icy,ir,al_map_rgb(60,90,160),1.0f);
        float ax2=icx+cosf(-st->vis_yaw)*ir*0.75f;
        float ay2=icy+sinf(-st->vis_yaw)*ir*0.75f;
        al_draw_line(icx,icy,ax2,ay2,al_map_rgb(80,160,255),2.0f);
        al_draw_filled_circle(ax2,ay2,3,al_map_rgb(80,160,255));
        al_draw_text(fn,al_map_rgba(120,150,200,160),icx,icy+ir+4,ALLEGRO_ALIGN_CENTRE,"360");
    }

    /* Bouton retour */
    br.x=vx; br.y=SCR_H-58; br.w=180; br.h=40;
    strcpy(br.label,"< Retour aide");
    br.r=50;br.g=50;br.b=80;br.rb=80;br.gb=80;br.bb=120;
    draw_btn(fn,br,mx,my);

    al_draw_text(fn,al_map_rgba(100,125,170,140),
                 vx+vw/2,SCR_H-48,ALLEGRO_ALIGN_CENTRE,
                 "Glissez la souris pour faire pivoter l avion");
    (void)fb;
}

/* ======= JEU DE CHASSE ======= */
#define CHASSE_MAX_ENNEMIS   12
#define CHASSE_MAX_MISSILES  20
#define CHASSE_MAX_ETOILES   120
#define CHASSE_MAX_EXPLOSIONS 16
#define CHASSE_HP_MAX        100
#define CHASSE_VAGUES        5

typedef struct {
    float x,y,vx,vy;
    bool actif;
    int  type;   /* 0=chasseur 1=lourd 2=boss */
    int  hp,hp_max;
    float tir_cd;
    float angle;
} ChasseEnnemi;

typedef struct {
    float x,y,vx,vy;
    bool actif;
    int owner; /* 0=joueur 1=ennemi */
} ChasseMissile;

typedef struct {
    float x,y,r;
    int  timer;
    ALLEGRO_COLOR col;
} ChasseExplosion;

typedef struct {
    float x,y;
    float bright;
    float speed;
} ChasseEtoile;

typedef struct {
    float    px,py;          /* position joueur */
    float    pvx,pvy;
    int      hp;
    int      score;
    int      vague;
    int      ennemis_restants;
    bool     game_over;
    bool     victoire;
    bool     paused;
    int      invincible;     /* frames invincibilite apres degats */
    float    tir_cd;
    ChasseEnnemi    ennemis[CHASSE_MAX_ENNEMIS];
    ChasseMissile   missiles[CHASSE_MAX_MISSILES];
    ChasseExplosion explosions[CHASSE_MAX_EXPLOSIONS];
    ChasseEtoile    etoiles[CHASSE_MAX_ETOILES];
    ALLEGRO_BITMAP *f16_bmp;
    bool            f16_loaded;
    int             tick;
    int             wave_timer;
    float           nuage_x[6];
    float           nuage_y[6];
} ChasseState;

static void chasse_init_etoiles(ChasseState *cs){
    int i;
    srand(42);
    for(i=0;i<CHASSE_MAX_ETOILES;i++){
        cs->etoiles[i].x=(float)(rand()%SCR_W);
        cs->etoiles[i].y=(float)(rand()%SCR_H);
        cs->etoiles[i].bright=0.3f+(rand()%70)/100.0f;
        cs->etoiles[i].speed =0.2f+(rand()%30)/100.0f;
    }
}

static void chasse_add_explosion(ChasseState *cs,float x,float y,bool big){
    int i;
    for(i=0;i<CHASSE_MAX_EXPLOSIONS;i++){
        if(!cs->explosions[i].timer){
            cs->explosions[i].x=x; cs->explosions[i].y=y;
            cs->explosions[i].r=big?60.0f:30.0f;
            cs->explosions[i].timer=big?35:22;
            cs->explosions[i].col=big?al_map_rgb(255,140,20):al_map_rgb(255,200,80);
            return;
        }
    }
}

static void chasse_spawn_vague(ChasseState *cs){
    int i,n,type;
    float ey;
    cs->vague++;
    /* Nettoie ennemis */
    for(i=0;i<CHASSE_MAX_ENNEMIS;i++) cs->ennemis[i].actif=false;
    cs->wave_timer = 60*30; /* 30 secondes max par vague */
    n = 3 + cs->vague*2;
    if(n>CHASSE_MAX_ENNEMIS) n=CHASSE_MAX_ENNEMIS;
    cs->ennemis_restants=n;
    for(i=0;i<n;i++){
        ey = 60.0f + (float)(rand()%(SCR_H-120));
        type = (cs->vague>=4&&i==0)?2:(cs->vague>=2&&i<2)?1:0;
        cs->ennemis[i].x = SCR_W + 80.0f + i*120.0f;
        cs->ennemis[i].y = ey;
        cs->ennemis[i].vx= -(1.8f + cs->vague*0.3f);
        cs->ennemis[i].vy= 0;
        cs->ennemis[i].type = type;
        cs->ennemis[i].hp   = type==2?200:(type==1?60:25);
        cs->ennemis[i].hp_max= cs->ennemis[i].hp;
        cs->ennemis[i].tir_cd= (float)(rand()%80)+40.0f;
        cs->ennemis[i].angle = 0;
        cs->ennemis[i].actif = true;
    }
}

static void chasse_reset(ChasseState *cs){
    memset(cs->ennemis,  0, sizeof(cs->ennemis));
    memset(cs->missiles, 0, sizeof(cs->missiles));
    memset(cs->explosions,0,sizeof(cs->explosions));
    cs->px=150; cs->py=SCR_H/2;
    cs->pvx=0;  cs->pvy=0;
    cs->hp=CHASSE_HP_MAX;
    cs->score=0; cs->vague=0;
    cs->game_over=false; cs->victoire=false; cs->paused=false;
    cs->invincible=0; cs->tir_cd=0; cs->tick=0;
    chasse_init_etoiles(cs);
    /* nuages lumineux */
    int i; for(i=0;i<6;i++){cs->nuage_x[i]=(float)(rand()%SCR_W);cs->nuage_y[i]=40.0f+(rand()%200);}
    chasse_spawn_vague(cs);
}

static void chasse_fire_player(ChasseState *cs){
    int i;
    if(cs->tir_cd>0) return;
    for(i=0;i<CHASSE_MAX_MISSILES;i++){
        if(!cs->missiles[i].actif){
            cs->missiles[i].x=cs->px+55;
            cs->missiles[i].y=cs->py;
            cs->missiles[i].vx=14.0f;
            cs->missiles[i].vy=0;
            cs->missiles[i].owner=0;
            cs->missiles[i].actif=true;
            cs->tir_cd=12.0f;
            return;
        }
    }
}

static void chasse_fire_ennemi(ChasseState *cs, int ei){
    int i;
    ChasseEnnemi *e=&cs->ennemis[ei];
    float dx=cs->px-e->x, dy=cs->py-e->y;
    float len=sqrtf(dx*dx+dy*dy); if(len<1)len=1;
    float spd=5.5f;
    for(i=0;i<CHASSE_MAX_MISSILES;i++){
        if(!cs->missiles[i].actif){
            cs->missiles[i].x=e->x-20;
            cs->missiles[i].y=e->y;
            cs->missiles[i].vx=(dx/len)*spd;
            cs->missiles[i].vy=(dy/len)*spd;
            cs->missiles[i].owner=1;
            cs->missiles[i].actif=true;
            return;
        }
    }
}

static void chasse_update(ChasseState *cs, bool key_up, bool key_down,
                          bool key_left, bool key_right, bool key_fire){
    int i,j;
    if(cs->game_over||cs->victoire||cs->paused) return;
    cs->tick++;

    /* Etoiles parallaxe */
    for(i=0;i<CHASSE_MAX_ETOILES;i++){
        cs->etoiles[i].x -= cs->etoiles[i].speed;
        if(cs->etoiles[i].x<0) cs->etoiles[i].x=SCR_W;
    }

    /* Nuages */
    for(i=0;i<6;i++){
        cs->nuage_x[i]-=0.4f;
        if(cs->nuage_x[i]<-150) cs->nuage_x[i]=SCR_W+100;
    }

    /* Joueur mouvement */
    float acc=0.45f, frein=0.85f, maxv=5.5f;
    if(key_up)    cs->pvy-=acc;
    if(key_down)  cs->pvy+=acc;
    if(key_left)  cs->pvx-=acc;
    if(key_right) cs->pvx+=acc;
    if(!key_up&&!key_down)  cs->pvy*=frein;
    if(!key_left&&!key_right) cs->pvx*=frein;
    if(cs->pvx>maxv)  cs->pvx=maxv;
    if(cs->pvx<-maxv) cs->pvx=-maxv;
    if(cs->pvy>maxv)  cs->pvy=maxv;
    if(cs->pvy<-maxv) cs->pvy=-maxv;
    cs->px+=cs->pvx; cs->py+=cs->pvy;
    if(cs->px<30)       cs->px=30;
    if(cs->px>SCR_W-30) cs->px=SCR_W-30;
    if(cs->py<30)       cs->py=30;
    if(cs->py>SCR_H-30) cs->py=SCR_H-30;

    /* Tir joueur */
    if(key_fire) chasse_fire_player(cs);
    if(cs->tir_cd>0) cs->tir_cd--;
    if(cs->invincible>0) cs->invincible--;

    /* Mise a jour missiles */
    for(i=0;i<CHASSE_MAX_MISSILES;i++){
        if(!cs->missiles[i].actif) continue;
        cs->missiles[i].x+=cs->missiles[i].vx;
        cs->missiles[i].y+=cs->missiles[i].vy;
        if(cs->missiles[i].x<-20||cs->missiles[i].x>SCR_W+20||
           cs->missiles[i].y<-20||cs->missiles[i].y>SCR_H+20)
            cs->missiles[i].actif=false;

        /* Collision missile joueur -> ennemi */
        if(cs->missiles[i].owner==0){
            for(j=0;j<CHASSE_MAX_ENNEMIS;j++){
                if(!cs->ennemis[j].actif) continue;
                float dx=cs->missiles[i].x-cs->ennemis[j].x;
                float dy=cs->missiles[i].y-cs->ennemis[j].y;
                float r = cs->ennemis[j].type==2?55.0f:(cs->ennemis[j].type==1?35.0f:22.0f);
                if(dx*dx+dy*dy<r*r){
                    int dmg=cs->ennemis[j].type==2?8:15;
                    cs->ennemis[j].hp-=dmg;
                    cs->missiles[i].actif=false;
                    if(cs->ennemis[j].hp<=0){
                        chasse_add_explosion(cs,cs->ennemis[j].x,cs->ennemis[j].y,cs->ennemis[j].type==2);
                        cs->ennemis[j].actif=false;
                        cs->score+=cs->ennemis[j].type==2?500:(cs->ennemis[j].type==1?150:50);
                        cs->ennemis_restants--;
                    }
                    break;
                }
            }
        }
        /* Collision missile ennemi -> joueur */
        if(cs->missiles[i].owner==1&&cs->invincible==0){
            float dx=cs->missiles[i].x-cs->px;
            float dy=cs->missiles[i].y-cs->py;
            if(dx*dx+dy*dy<28.0f*28.0f){
                cs->hp-=12;
                cs->missiles[i].actif=false;
                cs->invincible=60;
                chasse_add_explosion(cs,cs->px,cs->py,false);
                if(cs->hp<=0){cs->hp=0;cs->game_over=true;}
            }
        }
    }

    /* Mise a jour ennemis */
    for(i=0;i<CHASSE_MAX_ENNEMIS;i++){
        if(!cs->ennemis[i].actif) continue;
        ChasseEnnemi *e=&cs->ennemis[i];

        /* Mouvement : sinusoidal pour chasseur, droit pour lourd, boss tourne */
        if(e->type==0){
            e->y+=sinf(cs->tick*0.04f+i)*1.2f;
            e->x+=e->vx;
        } else if(e->type==1){
            e->x+=e->vx*1.0f;  /* lourd avance aussi vite */
            e->y+=sinf(cs->tick*0.02f+i)*0.8f;
        } else {
            /* Boss : tourne ET avance lentement vers la gauche */
            e->angle+=0.018f;
            e->x = SCR_W*0.72f + cosf(e->angle)*180.0f + (cs->tick - cs->tick%1)*(-0.15f);
            if(e->x > SCR_W*0.85f) e->x = SCR_W*0.85f;
            e->y = SCR_H*0.5f  + sinf(e->angle)*140.0f;
        }

        /* Rebond bords */
        if(e->y<40)  e->y=40;
        if(e->y>SCR_H-40) e->y=SCR_H-40;
        if(e->x<-150){ e->actif=false; cs->ennemis_restants--; if(cs->ennemis_restants<0)cs->ennemis_restants=0; continue; }

        /* Tir ennemi */
        e->tir_cd--;
        if(e->tir_cd<=0){
            chasse_fire_ennemi(cs,i);
            e->tir_cd = e->type==2?25.0f:(e->type==1?55.0f:75.0f);
        }

        /* Collision physique joueur */
        if(cs->invincible==0){
            float dx=e->x-cs->px, dy=e->y-cs->py;
            float cr=e->type==2?60.0f:35.0f;
            if(dx*dx+dy*dy<cr*cr){
                cs->hp-=20; cs->invincible=90;
                chasse_add_explosion(cs,cs->px,cs->py,false);
                if(cs->hp<=0){cs->hp=0;cs->game_over=true;}
            }
        }
    }

    /* Explosions */
    for(i=0;i<CHASSE_MAX_EXPLOSIONS;i++)
        if(cs->explosions[i].timer>0) cs->explosions[i].timer--;

    /* Prochaine vague */
    if(cs->wave_timer>0) cs->wave_timer--;
    if((cs->ennemis_restants<=0||cs->wave_timer==0)&&!cs->game_over){
        if(cs->vague>=CHASSE_VAGUES) cs->victoire=true;
        else chasse_spawn_vague(cs);
    }
}

static void chasse_draw_f16(ChasseState *cs, float x, float y, float scale, bool flip){
    if(cs->f16_loaded&&cs->f16_bmp){
        float bw=(float)al_get_bitmap_width(cs->f16_bmp);
        float bh=(float)al_get_bitmap_height(cs->f16_bmp);
        float dw=bw*scale, dh=bh*scale;
        int flags=flip?ALLEGRO_FLIP_HORIZONTAL:0;
        al_draw_scaled_bitmap(cs->f16_bmp,0,0,bw,bh,x-dw/2,y-dh/2,dw,dh,flags);
    } else {
        /* Fallback : dessine un avion geometrique */
        ALLEGRO_COLOR c=al_map_rgb(180,200,220);
        float s=scale*40;
        if(flip){
            al_draw_filled_triangle(x-s,y, x+s*0.6f,y-s*0.35f, x+s*0.6f,y+s*0.35f,c);
            al_draw_filled_triangle(x-s*0.1f,y-s*0.5f, x+s*0.4f,y-s*0.5f, x+s*0.1f,y,c);
        } else {
            al_draw_filled_triangle(x+s,y, x-s*0.6f,y-s*0.35f, x-s*0.6f,y+s*0.35f,c);
            al_draw_filled_triangle(x+s*0.1f,y-s*0.5f, x-s*0.4f,y-s*0.5f, x-s*0.1f,y,c);
        }
    }
}

static void chasse_draw_ennemi(ChasseEnnemi *e){
    float x=e->x, y=e->y;
    ALLEGRO_COLOR body,wing,accent;
    float s;
    if(e->type==2){
        s=1.8f;
        body  =al_map_rgb(180,30,30);
        wing  =al_map_rgb(140,20,20);
        accent=al_map_rgb(255,80,80);
    } else if(e->type==1){
        s=1.2f;
        body  =al_map_rgb(120,60,180);
        wing  =al_map_rgb(90,40,140);
        accent=al_map_rgb(180,120,255);
    } else {
        s=0.85f;
        body  =al_map_rgb(50,140,200);
        wing  =al_map_rgb(30,100,160);
        accent=al_map_rgb(100,200,255);
    }
    /* Corps */
    al_draw_filled_triangle(x-50*s,y, x+20*s,y-14*s, x+20*s,y+14*s, body);
    /* Aile */
    al_draw_filled_triangle(x-10*s,y-10*s, x+15*s,y-32*s, x+15*s,y, wing);
    al_draw_filled_triangle(x-10*s,y+10*s, x+15*s,y+32*s, x+15*s,y, wing);
    /* Queue */
    al_draw_filled_triangle(x+10*s,y-8*s, x+22*s,y-22*s, x+22*s,y, accent);
    /* Cockpit */
    al_draw_filled_circle(x-18*s,y,6*s,al_map_rgba(100,220,255,200));
    /* Barre HP */
    if(e->hp<e->hp_max){
        float bw=50*s, bx2=x-bw/2, by2=y-38*s;
        al_draw_filled_rectangle(bx2,by2,bx2+bw,by2+6,al_map_rgba(40,0,0,180));
        al_draw_filled_rectangle(bx2,by2,bx2+bw*(float)e->hp/e->hp_max,by2+6,al_map_rgb(220,50,50));
    }
}

static void draw_chasse_page(ChasseState *cs, ALLEGRO_FONT *fb, ALLEGRO_FONT *fn,
                              bool key_up,bool key_dn,bool key_lf,bool key_rt,bool key_fire){
    int i;

    /* === FOND NUIT ETOILEE === */
    al_draw_filled_rectangle(0,0,SCR_W,SCR_H,al_map_rgb(4,6,18));

    /* Etoiles */
    for(i=0;i<CHASSE_MAX_ETOILES;i++){
        int bv=(int)(cs->etoiles[i].bright*255);
        al_draw_filled_circle(cs->etoiles[i].x,cs->etoiles[i].y,
            cs->etoiles[i].bright>0.7f?1.5f:1.0f,
            al_map_rgb(bv,bv,bv));
    }

    /* Nuages nocturnes subtils */
    for(i=0;i<6;i++){
        al_draw_filled_ellipse(cs->nuage_x[i],cs->nuage_y[i],
            80+i*20,22+i*5,al_map_rgba(30,40,80,28));
    }

    /* Lune */
    al_draw_filled_circle(SCR_W-90,60,36,al_map_rgb(240,240,200));
    al_draw_filled_circle(SCR_W-74,52,30,al_map_rgb(4,6,18));

    /* === MISSILES === */
    for(i=0;i<CHASSE_MAX_MISSILES;i++){
        if(!cs->missiles[i].actif) continue;
        ALLEGRO_COLOR mc=cs->missiles[i].owner==0?al_map_rgb(80,220,255):al_map_rgb(255,80,80);
        float mx2=cs->missiles[i].x, my2=cs->missiles[i].y;
        float tail= cs->missiles[i].owner==0 ? -12.0f : 12.0f;
        al_draw_line(mx2,my2,mx2+tail,my2,al_map_rgba(255,200,100,120),2);
        al_draw_filled_circle(mx2,my2,4,mc);
    }

    /* === ENNEMIS === */
    for(i=0;i<CHASSE_MAX_ENNEMIS;i++){
        if(!cs->ennemis[i].actif) continue;
        chasse_draw_ennemi(&cs->ennemis[i]);
    }

    /* === JOUEUR (F-16) === */
    bool blink=(cs->invincible>0&&(cs->tick/5)%2==0);
    if(!blink){
        chasse_draw_f16(cs,cs->px,cs->py,0.18f,false);
        /* Flamme reacteur */
        float fx=cs->px-52*0.18f;
        al_draw_filled_triangle(fx,cs->py-5, fx-14,cs->py, fx,cs->py+5,
            al_map_rgba(255,140+rand()%60,30,200));
    }

    /* === EXPLOSIONS === */
    for(i=0;i<CHASSE_MAX_EXPLOSIONS;i++){
        if(!cs->explosions[i].timer) continue;
        float prog=(float)cs->explosions[i].timer/35.0f;
        float rad=cs->explosions[i].r*(1.0f-prog*0.5f);
        al_draw_filled_circle(cs->explosions[i].x,cs->explosions[i].y,
            rad,al_map_rgba(255,180,40,(int)(prog*220)));
        al_draw_filled_circle(cs->explosions[i].x,cs->explosions[i].y,
            rad*0.5f,al_map_rgba(255,240,200,(int)(prog*200)));
    }

    /* === HUD === */
    /* Barre HP */
    float hpx=20,hpy=SCR_H-36,hpw=200,hph=18;
    al_draw_filled_rounded_rectangle(hpx,hpy,hpx+hpw,hpy+hph,4,4,al_map_rgba(0,0,0,160));
    float hpfill=hpw*(float)cs->hp/CHASSE_HP_MAX;
    ALLEGRO_COLOR hpcol = cs->hp>60?al_map_rgb(40,220,80):cs->hp>30?al_map_rgb(240,200,20):al_map_rgb(240,40,40);
    al_draw_filled_rounded_rectangle(hpx,hpy,hpx+hpfill,hpy+hph,4,4,hpcol);
    al_draw_rounded_rectangle(hpx,hpy,hpx+hpw,hpy+hph,4,4,al_map_rgba(120,180,255,160),1.5f);
    char shp[32]; snprintf(shp,sizeof(shp),"HP : %d",cs->hp);
    al_draw_text(fn,al_map_rgb(220,240,255),hpx+hpw/2,hpy+3,ALLEGRO_ALIGN_CENTRE,shp);

    /* Score & vague */
    char shud[80];
    snprintf(shud,sizeof(shud),"SCORE : %d",cs->score);
    al_draw_text(fb,al_map_rgb(220,200,80),SCR_W/2,8,ALLEGRO_ALIGN_CENTRE,shud);
    snprintf(shud,sizeof(shud),"VAGUE %d / %d",cs->vague,CHASSE_VAGUES);
    al_draw_text(fn,al_map_rgb(140,180,240),SCR_W-10,8,ALLEGRO_ALIGN_RIGHT,shud);
    al_draw_text(fn,al_map_rgba(100,130,180,160),SCR_W/2,SCR_H-18,ALLEGRO_ALIGN_CENTRE,
        "Fleches = deplacer  |  ESPACE = tirer  |  P = pause  |  ESC = quitter");

    /* Ennemis restants */
    snprintf(shud,sizeof(shud),"Ennemis : %d",cs->ennemis_restants>0?cs->ennemis_restants:0);
    al_draw_text(fn,al_map_rgb(255,120,120),10,8,ALLEGRO_ALIGN_LEFT,shud);

    /* PAUSE */
    if(cs->paused){
        al_draw_filled_rectangle(0,0,SCR_W,SCR_H,al_map_rgba(0,0,20,160));
        al_draw_text(fb,al_map_rgb(220,200,80),SCR_W/2,SCR_H/2-30,ALLEGRO_ALIGN_CENTRE,"PAUSE");
        al_draw_text(fn,al_map_rgb(180,200,240),SCR_W/2,SCR_H/2+10,ALLEGRO_ALIGN_CENTRE,"P = reprendre  |  ESC = quitter");
    }

    /* GAME OVER */
    if(cs->game_over){
        al_draw_filled_rectangle(0,0,SCR_W,SCR_H,al_map_rgba(80,0,0,180));
        al_draw_text(fb,al_map_rgb(255,60,60),SCR_W/2,SCR_H/2-50,ALLEGRO_ALIGN_CENTRE,"GAME OVER");
        snprintf(shud,sizeof(shud),"Score final : %d",cs->score);
        al_draw_text(fn,al_map_rgb(220,180,80),SCR_W/2,SCR_H/2,ALLEGRO_ALIGN_CENTRE,shud);
        al_draw_text(fn,al_map_rgb(160,200,240),SCR_W/2,SCR_H/2+40,ALLEGRO_ALIGN_CENTRE,"R = recommencer  |  ESC = menu");
    }

    /* VICTOIRE */
    if(cs->victoire){
        al_draw_filled_rectangle(0,0,SCR_W,SCR_H,al_map_rgba(0,60,0,160));
        al_draw_text(fb,al_map_rgb(80,255,120),SCR_W/2,SCR_H/2-50,ALLEGRO_ALIGN_CENTRE,"VICTOIRE !");
        snprintf(shud,sizeof(shud),"Score final : %d",cs->score);
        al_draw_text(fn,al_map_rgb(220,200,80),SCR_W/2,SCR_H/2,ALLEGRO_ALIGN_CENTRE,shud);
        al_draw_text(fn,al_map_rgb(160,200,240),SCR_W/2,SCR_H/2+40,ALLEGRO_ALIGN_CENTRE,"R = rejouer  |  ESC = menu");
    }

    (void)key_up;(void)key_dn;(void)key_lf;(void)key_rt;(void)key_fire;
    (void)fb;
}

/* ======= ECRAN MENU ======= */
static void ecran_menu(ALLEGRO_DISPLAY *disp,ALLEGRO_EVENT_QUEUE *queue,
                        ALLEGRO_TIMER *timer,ALLEGRO_FONT *fb,ALLEGRO_FONT *fn){
    AppState st; int tick=0; float mx=0,my=0; bool running=true;
    ChasseState cs; memset(&cs,0,sizeof(cs));
    bool k_up=false,k_dn=false,k_lf=false,k_rt=false,k_fire=false;
    bool chasse_initialized=false;
    memset(&st,0,sizeof(st)); st.page=PAGE_MENU; strcpy(st.inp_rho,"1.225");

    while(running){
        ALLEGRO_EVENT ev; al_wait_for_event(queue,&ev);
        if(ev.type==ALLEGRO_EVENT_MOUSE_AXES){
            mx=(float)ev.mouse.x;my=(float)ev.mouse.y;
            if(st.page==PAGE_GESTION_LISTE&&ev.mouse.dz!=0){
                st.liste_scroll-=ev.mouse.dz;
                if(st.liste_scroll<0)st.liste_scroll=0;
                if(st.liste_scroll>NB_AVIONS-14)st.liste_scroll=NB_AVIONS-14;
            }
            if(st.page==PAGE_VISIONNEUSE&&st.vis_dragging){
                st.vis_yaw   += (mx-st.vis_drag_x)*0.008f;
                st.vis_pitch += (my-st.vis_drag_y)*0.008f;
                if(st.vis_pitch> 1.2f) st.vis_pitch= 1.2f;
                if(st.vis_pitch<-1.2f) st.vis_pitch=-1.2f;
                st.vis_drag_x=mx; st.vis_drag_y=my;
            }
        }
        if(ev.type==ALLEGRO_EVENT_MOUSE_BUTTON_UP){
            if(st.page==PAGE_VISIONNEUSE) st.vis_dragging=false;
        }
        if(ev.type==ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
            mx=(float)ev.mouse.x;my=(float)ev.mouse.y;
            float bw=340,bx2=(SCR_W-bw)/2;
            if(st.page==PAGE_MENU){
                float by=120,gap=66;
                if(in_rect(mx,my,bx2,by+0*gap,bw,56)) st.page=PAGE_GESTION;
                if(in_rect(mx,my,bx2,by+1*gap,bw,56)) st.page=PAGE_ANALYSE;
                if(in_rect(mx,my,bx2,by+2*gap,bw,56)&&st.avion.existe){
                    FILE *f=fopen("donnees_avion.txt","w");
                    if(f){fprintf(f,"Nom : %s\nMasse : %.2f\nSurface : %.2f\nCLmax : %.2f\nRho : %.3f\nCD0 : %.3f\nk : %.3f\nPoussee : %.2f\n",st.avion.nom,st.avion.masse,st.avion.surface,st.avion.clmax,st.avion.rho,st.avion.cd0,st.avion.k,st.avion.poussee);fclose(f);strcpy(st.msg,"Exporte vers donnees_avion.txt !");st.msg_ok=true;}
                    else{strcpy(st.msg,"Erreur fichier !");st.msg_ok=false;}
                    st.msg_timer=180;
                }
                if(in_rect(mx,my,bx2,by+3*gap,bw,56)) st.page=PAGE_AIDE;
                if(in_rect(mx,my,bx2,by+4*gap,bw,56)) st.page=PAGE_CHASSE;
                if(in_rect(mx,my,bx2,by+5*gap,bw,56)) running=false;
            }
            else if(st.page==PAGE_GESTION){
                float col=(SCR_W-bw)/2;
                if(in_rect(mx,my,col,130,bw,52)) st.page=PAGE_GESTION_LISTE;
                if(in_rect(mx,my,col,200,bw,52)){memset(st.inp_nom,0,40);memset(st.inp_masse,0,20);memset(st.inp_surface,0,20);memset(st.inp_clmax,0,20);memset(st.inp_cd0,0,20);memset(st.inp_k,0,20);memset(st.inp_poussee,0,20);strcpy(st.inp_rho,"1.225");st.inp_active=0;st.page=PAGE_GESTION_CUSTOM;}
                if(in_rect(mx,my,col,270,bw,52)) st.page=PAGE_GESTION_VOIR;
                if(in_rect(mx,my,col,340,bw,52)&&st.avion.existe){FILE *f=fopen("donnees_avion.txt","w");if(f){fprintf(f,"Nom : %s\nMasse : %.2f\nSurface : %.2f\nCLmax : %.2f\nRho : %.3f\nCD0 : %.3f\nk : %.3f\nPoussee : %.2f\n",st.avion.nom,st.avion.masse,st.avion.surface,st.avion.clmax,st.avion.rho,st.avion.cd0,st.avion.k,st.avion.poussee);fclose(f);strcpy(st.msg,"Exporte !");st.msg_ok=true;}else{strcpy(st.msg,"Erreur !");st.msg_ok=false;}st.msg_timer=180;}
                if(in_rect(mx,my,col,430,bw,52)) st.page=PAGE_MENU;
            }
            else if(st.page==PAGE_GESTION_LISTE){
                int i; float lx=80,ly=100,lw=SCR_W-160,row=30;
                for(i=0;i<14;i++){int idx=i+st.liste_scroll;if(idx<NB_AVIONS&&in_rect(mx,my,lx+4,ly+5+i*row,lw-8,row-2))st.liste_sel=idx;}
                if(in_rect(mx,my,SCR_W/2-200,SCR_H-80,180,44)){st.avion=avions_pred[st.liste_sel];st.page=PAGE_GESTION;}
                if(in_rect(mx,my,SCR_W/2+20,SCR_H-80,180,44)) st.page=PAGE_GESTION;
            }
            else if(st.page==PAGE_GESTION_CUSTOM){
                float cx=120,cw=380,gap=62;
                if(in_rect(mx,my,cx,120,cw,38))st.inp_active=0;if(in_rect(mx,my,cx,120+gap,cw,38))st.inp_active=1;
                if(in_rect(mx,my,cx+cw+60,120,cw,38))st.inp_active=2;if(in_rect(mx,my,cx+cw+60,120+gap,cw,38))st.inp_active=3;
                if(in_rect(mx,my,cx,120+gap*2,cw,38))st.inp_active=4;if(in_rect(mx,my,cx+cw+60,120+gap*2,cw,38))st.inp_active=5;
                if(in_rect(mx,my,cx,120+gap*3,cw,38))st.inp_active=6;if(in_rect(mx,my,cx+cw+60,120+gap*3,cw,38))st.inp_active=7;
                if(in_rect(mx,my,SCR_W/2-200,SCR_H-80,180,44))valider_custom(&st);
                if(in_rect(mx,my,SCR_W/2+20,SCR_H-80,180,44))st.page=PAGE_GESTION;
            }
            else if(st.page==PAGE_GESTION_VOIR){
                if(in_rect(mx,my,SCR_W/2-200,SCR_H-80,180,44)){st.avion.existe=0;st.page=PAGE_GESTION;}
                if(in_rect(mx,my,SCR_W/2+20,SCR_H-80,180,44))st.page=PAGE_GESTION;
            }
            else if(st.page==PAGE_ANALYSE){
                float col=(SCR_W-bw)/2;
                if(in_rect(mx,my,col,310,bw,52)&&st.avion.existe){float Vs=vit_decrochage(st.avion);if(st.avion.poussee>calc_trainee(st.avion,Vs)){st.anim_Vs=Vs;st.anim_frame=0;st.anim_running=true;st.anim_possible=true;}else st.anim_possible=false;st.page=PAGE_DECOLLAGE;}
                if(in_rect(mx,my,(SCR_W-bw)/2,380,bw,52))st.page=PAGE_MENU;
            }
            else if(st.page==PAGE_DECOLLAGE){if(in_rect(mx,my,SCR_W-200,SCR_H-70,160,40))st.page=PAGE_ANALYSE;}
            else if(st.page==PAGE_AIDE){
                if(in_rect(mx,my,SCR_W/2-90,SCR_H-58,180,44)) st.page=PAGE_MENU;
                /* Bouton visionneuse */
                float px=80,py=100,pw=SCR_W-160;
                if(in_rect(mx,my,SCR_W/2-180,py+358,360,46)){
                    st.page=PAGE_VISIONNEUSE;
                    st.vis_yaw=0.4f; st.vis_pitch=-0.2f; st.vis_dragging=false;
                }
                (void)pw;
            }
            else if(st.page==PAGE_VISIONNEUSE){
                if(in_rect(mx,my,SCR_W/2-90,SCR_H-52,180,38)) st.page=PAGE_AIDE;
                else { st.vis_dragging=true; st.vis_drag_x=mx; st.vis_drag_y=my; }
            }
        }
        if(ev.type==ALLEGRO_EVENT_KEY_DOWN){
            if(st.page==PAGE_CHASSE){
                if(ev.keyboard.keycode==ALLEGRO_KEY_UP)    k_up=true;
                if(ev.keyboard.keycode==ALLEGRO_KEY_DOWN)  k_dn=true;
                if(ev.keyboard.keycode==ALLEGRO_KEY_LEFT)  k_lf=true;
                if(ev.keyboard.keycode==ALLEGRO_KEY_RIGHT) k_rt=true;
                if(ev.keyboard.keycode==ALLEGRO_KEY_SPACE) k_fire=true;
            }
        }
        if(ev.type==ALLEGRO_EVENT_KEY_UP){
            if(ev.keyboard.keycode==ALLEGRO_KEY_UP)    k_up=false;
            if(ev.keyboard.keycode==ALLEGRO_KEY_DOWN)  k_dn=false;
            if(ev.keyboard.keycode==ALLEGRO_KEY_LEFT)  k_lf=false;
            if(ev.keyboard.keycode==ALLEGRO_KEY_RIGHT) k_rt=false;
            if(ev.keyboard.keycode==ALLEGRO_KEY_SPACE) k_fire=false;
        }
        if(ev.type==ALLEGRO_EVENT_KEY_CHAR){
            if(st.page==PAGE_CHASSE){
                if(ev.keyboard.keycode==ALLEGRO_KEY_P) cs.paused=!cs.paused;
                if(ev.keyboard.keycode==ALLEGRO_KEY_R&&(cs.game_over||cs.victoire)) chasse_reset(&cs);
                if(ev.keyboard.keycode==ALLEGRO_KEY_ESCAPE){ st.page=PAGE_MENU; k_up=k_dn=k_lf=k_rt=k_fire=false; }
            } else {
            if(st.page==PAGE_GESTION_CUSTOM){
                char *fields[8]={st.inp_nom,st.inp_masse,st.inp_surface,st.inp_clmax,st.inp_rho,st.inp_cd0,st.inp_k,st.inp_poussee};
                char *buf=fields[st.inp_active]; int l=strlen(buf);
                if(ev.keyboard.keycode==ALLEGRO_KEY_BACKSPACE){if(l>0)buf[l-1]='\0';}
                else if(ev.keyboard.keycode==ALLEGRO_KEY_TAB){st.inp_active=(st.inp_active+1)%8;}
                else if(ev.keyboard.keycode==ALLEGRO_KEY_ENTER){valider_custom(&st);}
                else if(ev.keyboard.unichar>=32&&l<38){buf[l]=(char)ev.keyboard.unichar;buf[l+1]='\0';}
            }
            if(ev.keyboard.keycode==ALLEGRO_KEY_ESCAPE){if(st.page!=PAGE_MENU)st.page=PAGE_MENU;else running=false;}
            } /* end else not PAGE_CHASSE */
        }
        if(ev.type==ALLEGRO_EVENT_DISPLAY_CLOSE) running=false;
        if(ev.type==ALLEGRO_EVENT_TIMER){
            tick++;
            /* Init jeu de chasse quand on entre sur la page */
            if(st.page==PAGE_CHASSE&&!chasse_initialized){
                chasse_initialized=true;
                cs.f16_bmp=al_load_bitmap("sprites/f16.png");
                cs.f16_loaded=(cs.f16_bmp!=NULL);
                chasse_reset(&cs);
            }
            if(st.page!=PAGE_CHASSE) chasse_initialized=false;
            if(st.page==PAGE_DECOLLAGE&&st.anim_running){st.anim_frame++;if(st.anim_frame>160){st.anim_frame=160;st.anim_running=false;}}
            if(st.msg_timer>0)st.msg_timer--;
            /* Update jeu chasse */
            if(st.page==PAGE_CHASSE) chasse_update(&cs,k_up,k_dn,k_lf,k_rt,k_fire);
            /* Draw */
            if(st.page==PAGE_CHASSE){
                draw_chasse_page(&cs,fb,fn,k_up,k_dn,k_lf,k_rt,k_fire);
            } else {
            draw_bg(tick);
            switch(st.page){
                case PAGE_MENU:           draw_menu_page(fb,fn,&st,mx,my,tick); break;
                case PAGE_GESTION:        draw_gestion_page(fb,fn,&st,mx,my);   break;
                case PAGE_GESTION_LISTE:  draw_liste_page(fb,fn,&st,mx,my);     break;
                case PAGE_GESTION_CUSTOM: draw_custom_page(fb,fn,&st,mx,my);    break;
                case PAGE_GESTION_VOIR:   draw_voir_page(fb,fn,&st,mx,my);      break;
                case PAGE_ANALYSE:        draw_analyse_page(fb,fn,&st,mx,my);   break;
                case PAGE_DECOLLAGE:      draw_decollage_page(fb,fn,&st,mx,my); break;
                case PAGE_AIDE:           draw_aide_page(fb,fn,mx,my);          break;
                case PAGE_VISIONNEUSE:    draw_visionneuse(fb,fn,&st,mx,my);    break;
                default: break;
            }
            if(st.msg_timer>0){ALLEGRO_COLOR mc=st.msg_ok?al_map_rgb(50,220,100):al_map_rgb(230,60,60);al_draw_filled_rounded_rectangle(SCR_W/2-260,SCR_H-50,SCR_W/2+260,SCR_H-20,8,8,al_map_rgba(10,14,30,200));al_draw_text(fn,mc,SCR_W/2,SCR_H-42,ALLEGRO_ALIGN_CENTRE,st.msg);}
            } /* end else not PAGE_CHASSE */
            al_flip_display();
        }
    }
    if(cs.f16_bmp) al_destroy_bitmap(cs.f16_bmp);
    (void)disp;(void)timer;
}

/* ======= MAIN ======= */
int main(void){
    int i; bool ok; char logged_user[64];
    ALLEGRO_TIMER *timer; ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_DISPLAY *disp; ALLEGRO_FONT *fb,*fn;
    ALLEGRO_BITMAP *sprites[SPRITE_COUNT]; bool has_spr;
    const char *spaths[SPRITE_COUNT]={
        "sprites/pilot_smooth1.png","sprites/pilot_smooth2.png",
        "sprites/pilot_smooth3.png","sprites/pilot_smooth4.png",
        "sprites/pilot_smooth5.png","sprites/pilot_smooth6.png",
        "sprites/pilot_smooth7.png","sprites/pilot_smooth8.png",
        "sprites/pilot_pull.png","sprites/pilot_front.png"
    };

    charger_comptes();
    al_init(); al_init_font_addon(); al_init_ttf_addon();
    al_init_primitives_addon(); al_init_image_addon();
    al_install_keyboard(); al_install_mouse();

    timer=al_create_timer(1.0/60.0);
    queue=al_create_event_queue();
    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE);
    disp=al_create_display(SCR_W,SCR_H);
    al_set_window_title(disp,"Aero-Concept");
    fb=al_create_builtin_font();
    fn=al_create_builtin_font();

    has_spr=true;
    for(i=0;i<SPRITE_COUNT;i++){sprites[i]=al_load_bitmap(spaths[i]);if(!sprites[i])has_spr=false;}

    al_register_event_source(queue,al_get_display_event_source(disp));
    al_register_event_source(queue,al_get_timer_event_source(timer));
    al_register_event_source(queue,al_get_keyboard_event_source());
    al_register_event_source(queue,al_get_mouse_event_source());
    al_start_timer(timer);

    memset(logged_user,0,sizeof(logged_user));

    /* Etape 1 : Login */
    ok=ecran_login(disp,queue,timer,fb,fn,sprites,has_spr,logged_user);

    /* Etape 2 : Menu (si login OK) */
    if(ok) ecran_menu(disp,queue,timer,fb,fn);

    /* Nettoyage */
    for(i=0;i<SPRITE_COUNT;i++) if(sprites[i]) al_destroy_bitmap(sprites[i]);
    al_destroy_font(fb);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    { int vi; for(vi=0;vi<VIS_SPRITES;vi++) if(vis_spr[vi]) al_destroy_bitmap(vis_spr[vi]); }
    al_destroy_display(disp);
    return 0;
}