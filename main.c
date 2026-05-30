#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>                                            //geometrik sekiller icin
#include <allegro5/allegro_font.h>                                                  //yazi icin
#define FPS 60                                                                      //saniyede kare yenilenme hizi
#define SCREEN_WIDTH 640                                                            //oyun penceresi yatay genislik
#define SCREEN_HEIGHT 570                                                           //oyun penceresi dikey genislik
#define STEP_SIZE 40                                                                //yon tuslariyla kurbaganin kac pixel hareket edecegi
#define LANE_COUNT 13                                                               //serit sayisi
#define LANE_HEIGHT 40                                                              //serit yuksekligi
#define VEHICLES_PER_LANE 3                                                         //her seritteki arac sayisi
#define TOTAL_VEHICLES (LANE_COUNT*VEHICLES_PER_LANE)                               //toplam arac sayisi
#define PLATFORMS_PER_LANE 3                                                        //her su seridindeki platform(kaplumbaga veya kutuk) sayisi
#define GOAL_COUNT 5                                                                //hedef sayisi
#define HUD_HEIGHT 50                                                               //hud seridi yuksekligi
struct Frog {                                                                       //kurbaganin ozellikleri
    float x;                                                                        //yatay konumu
    float y;                                                                        //dikey konumu
    int lives;                                                                      //kalan can sayisi
    int size;                                                                       //pixel boyutu
    bool drowning;                                                                  //batma animasyonu aktif mi
    int drown_timer;                                                                //batma animasyonu suresi
};
typedef enum {
    LANE_START,                                                                     //baslangic seridi
    LANE_SAFE,                                                                      //hedef seridi
    LANE_ROAD,                                                                      //asfalt seridi
    LANE_WATER,                                                                     //su seridi
    LANE_MIDDLE,                                                                    //ortadaki uzerinde beklenebilen serit
} LaneType;
typedef struct {                                                                    //seridim ozellikleri
    LaneType type;                                                                  //serit turu
    int direction;                                                                  //hareket yonu
    float speed;                                                                    //hareket hizi
} Lane;
Lane lanes[LANE_COUNT] = {                                                          //serit dizisi
    {LANE_SAFE, 0,0.0f},                                                            //hedef seridi bolgesi
    {LANE_WATER, +1,1.5f},                                                          //su seridi, saga dogru, orta hiz
    {LANE_WATER, -1,1.5f},                                                          //su seridi, sola dogru, orta hiz
    {LANE_WATER, +1,1.5f},                                                          //su seridi, saga dogru, orta hiz
    {LANE_WATER, +1,1.0f},                                                          //su seridi, saga dogru, yavas
    {LANE_WATER, -1,2.0f},                                                          //su seridi, sola dogru, hizli
    {LANE_MIDDLE, 0,0.0f},                                                          //ortadaki beklenebilen serit
    {LANE_ROAD, -1,1.0f},                                                           //asfalt seridi, sola dogru, yavas
    {LANE_ROAD, +1,3.0f},                                                           //asfalt seridi, saga dogru, cok hizli
    {LANE_ROAD, -1,1.5f},                                                           //asfalt seridi, sola dogru, orta hizli
    {LANE_ROAD, +1,1.5f},                                                           //asfalt seridi, saga dogru, orta hizli
    {LANE_ROAD, -1,1.5f},                                                           //asfalt seridi, sola dogru, orta hizli
    {LANE_START, 0,0.0f},                                                           //baslangic seridi
};
typedef enum {
    VEHICLE_CAR,                                                                    //araba
    VEHICLE_TRUCK,                                                                  //kamyon
    VEHICLE_BIKE,                                                                   //motor
} VehicleType;
typedef struct {                                                                    //aracin ozellikleri
    VehicleType type;                                                               //arac turu
    float x;                                                                        //yatay konumu
    float y;                                                                        //dikey konumu
    float width;                                                                    //genisligi
    int direction;                                                                  //yonu
    float speed;                                                                    //hizi
    int lane;                                                                       //bulundugu serit                                                         
} Vehicle;
typedef enum {
    PLATFORM_LOG,                                                                   //kutuk
    PLATFORM_TURTLE,                                                                //kaplumbaga
} PlatformType;
typedef struct {                                                                    //platformun ozellikleri
    PlatformType type;                                                              //platform turu
    float x;                                                                        //yatay
    float y;                                                                        //dikey
    float width;                                                                    //genislik
    int direction;                                                                  //yonu
    float speed;                                                                    //hizi
    int lane;                                                                       //bulundugu serit
} Platform;
typedef struct {                                                                    //hedef ozellikleri
    float x;                                                                        //yatay konum
    bool filled;                                                                    //doluluk durumu
} Goal;
int main() {
    if (!al_init())                                                                 //allegro ana motor
        return -1;                                                                  //baslatilamazsa -1 dondur
    if (!al_install_keyboard())                                                     //klavyeyi etkinlestirir
        return -1;                                          
    if (!al_init_primitives_addon())                                                //geometrik sekiller icin (kare kurbaga)
        return -1;                                     
    if (!al_init_font_addon())                                                      //yazi eklentisini baslatir
        return -1;
    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_OPENGL);                    //ekran kartimla uyusmazlik oldugu icin standart DirectX yerine OpenGL kullanımını zotunlu tuttum
    ALLEGRO_TIMER *timer= al_create_timer(1.0/FPS);                                 //oyun hizini sabitler
    ALLEGRO_EVENT_QUEUE *event_queue= al_create_event_queue();                      //hareketleri siraya alir
    ALLEGRO_DISPLAY *display= al_create_display(SCREEN_WIDTH,SCREEN_HEIGHT);        //pencereyi olusturan satir
    ALLEGRO_FONT *font=al_create_builtin_font();                                    //yerlesik fontu yukler
    if (!font)
        return -1;
    if(!timer||!event_queue||!display)                                              //yukaridakiler yapilamazsa oyunu kapatir
        return -1;
    al_register_event_source(event_queue,al_get_display_event_source(display));     //pencereyi kapatma hareketini siraya alir
    al_register_event_source(event_queue,al_get_timer_event_source(timer));         //zamanlayicinin sinyallerini siraya alir
    al_register_event_source(event_queue,al_get_keyboard_event_source());           //klavye hareketlerini siraya alir
    struct Frog frog;                                                               //kurbagayi nesne olarak uretir
    frog.size= 20;                                                                  //boyutu 20 pixel
    frog.x= (SCREEN_WIDTH/2)-(frog.size/2);                                         //x ekseninde pencerenin tam ortasinda
    frog.y= SCREEN_HEIGHT-LANE_HEIGHT+(LANE_HEIGHT-frog.size)/2;                    //y ekseninde kurbaga baslangic seridinin tam ortasinda
    frog.lives= 5;                                                                  //baslangicta 5 cani var
    frog.drowning=false;                                                            //baslangicta batmiyor
    frog.drown_timer=0;                                                             //batma sayaci sifir
    int invincible_timer=0;                                                         //her turlu carpismada sadece 1 can gitmesi icin dokunulmazlik sayaci
    int level=1;                                                                    //baslangic seviyesi
    int time_limit=60*FPS;                                                          //60 sn (frame cinsinden)
    int time_left=time_limit;                                                       //kalan sure
    Goal goals[GOAL_COUNT];                                                         //hedef dizisi
    for (int i=0;i<GOAL_COUNT;i++) {                                                //hedeflerin dongusu
        goals[i].filled=false;                                                      //baslangicta hepsi bos
        goals[i].x=i*(SCREEN_WIDTH/GOAL_COUNT)+(SCREEN_WIDTH/GOAL_COUNT-30)/2;      //esit araliklar
    }
    Vehicle vehicles[TOTAL_VEHICLES];                                               //arac dizisi
    int v=0;                                                                        //arac sayaci
    for (int i=0;i<LANE_COUNT;i++) {                                                //serit kontrol dongusu
        if (lanes[i].type != LANE_ROAD)                                             //asfalt seridi degilse
            continue;                                                               //atla
        for (int j=0;j<VEHICLES_PER_LANE;j++) {                                     //her seritteki araba sayisi dongusu
            vehicles[v].lane=i;                                                     //hangi seritte
            vehicles[v].direction=lanes[i].direction;                               //seridin yonunu kullan
            vehicles[v].speed=lanes[i].speed;                                       //seridin hizini kullan
            vehicles[v].y=HUD_HEIGHT+i*LANE_HEIGHT+(LANE_HEIGHT-30)/2;              //y ekseninde seridin ortasinda
            if (lanes[i].speed >= 3.0f)                                             //cok hizli seritteyse
                vehicles[v].type=VEHICLE_BIKE;                                      //arac motor
            else if (lanes[i].speed <= 1.0f)                                        //yavas seritteyse
                vehicles[v].type=VEHICLE_TRUCK;                                     //arac kamyon
            else                                                                    //orta hizli seritteyse
                vehicles[v].type=VEHICLE_CAR;                                       //arac araba
            if (vehicles[v].type==VEHICLE_TRUCK)                                    //kamyonsa
                vehicles[v].width=80.0f;                                            //genis
            else if (vehicles[v].type==VEHICLE_BIKE)                                //motorsa
                vehicles[v].width=25.0f;                                            //dar
            else                                                                    //arabaysa
                vehicles[v].width=40.0f;                                            //normal genislik
            vehicles[v].x = j*(SCREEN_WIDTH/VEHICLES_PER_LANE);                     //araclar arasi uzaklik esit
            v++;                                                                    //sonraki araca gec
        }
    }
    int total_vehicles=v;                                                           //gercek arac sayisi
    PlatformType water_lane_types[] = {                                             //su seritlerindeki platform turleri
        PLATFORM_LOG,                                                               //kutuk
        PLATFORM_TURTLE,                                                            //kaplumbaga
        PLATFORM_LOG,                                                               //kutuk
        PLATFORM_LOG,                                                               //kutuk
        PLATFORM_TURTLE,                                                            //kaplumbaga
    };
    Platform platforms[LANE_COUNT*PLATFORMS_PER_LANE];                              //platform dizisi
    int p=0;                                                                        //platform sayaci
    int water_lane=0;                                                               //su seridi sayaci
    for (int i=0;i<LANE_COUNT;i++) {                                                //serit kontrol dongusu
        if (lanes[i].type != LANE_WATER)                                            //su seridi degilse
            continue;                                                               //atla
        for (int j=0;j<PLATFORMS_PER_LANE;j++) {                                    //her seritteki platform dongusu
            platforms[p].lane=i;                                                    //hangi seritte
            platforms[p].direction=lanes[i].direction;                              //seridin yonunu kullan
            platforms[p].speed=lanes[i].speed;                                      //seridin hizini kullan
            platforms[p].y=HUD_HEIGHT+i*LANE_HEIGHT+(LANE_HEIGHT-30)/2;              //y ekseninde bulundugu seridi ortaliyor
            platforms[p].type=water_lane_types[water_lane];                         //siraya gore turunu belirle
            platforms[p].width=80.0f;                                               //genislik (2 kaplumbaga=1 kutuk)
            platforms[p].x=j*(SCREEN_WIDTH/PLATFORMS_PER_LANE);                     //platformlar aarsi mesafe esit
            p++;                                                                    //sonraki platforma gec
        }
        water_lane++;                                                               //sonraki su seridine gec
    }
    int total_platforms=p;                                                          //gercek platform sayisi
    bool game_running= true;                                                        //oyun dongusunu acik tutar
    bool redraw= true;                                                              //cizim kontrolu yapar
    al_start_timer(timer);                                                          //zamanlayiciyi baslatir
    while (game_running) {                                                          //oyun acikken sürekli dongu
        ALLEGRO_EVENT event;                                                        //olay bilgisini tutmak icin degisken
        al_wait_for_event(event_queue, &event);                                     //yeni hareket komutu bekler ve geleni 'event' degiskenine kaydeder
        if (event.type == ALLEGRO_EVENT_TIMER) {                                    //zamanlayicidan sinyal bekler
            redraw = true;                                                          //gelince goruntu yeniler
            if (time_left>0) {                                                      //sure bitmemisse
                time_left--;                                                        //sayaci azaltir
            }       
            else {                                                                  //sure bitmisse
                frog.lives--;                                                       //cani azaltir
                frog.x=(SCREEN_WIDTH/2)-(frog.size/2);                              //baslangic x konumuna doner
                frog.y=SCREEN_HEIGHT-LANE_HEIGHT+(LANE_HEIGHT-frog.size)/2;         //baslangic y konumuna doner
                time_left=time_limit;                                               //sureyi sifirlar
                invincible_timer=30;                                                //dokunulmazlik suresi
            }
            for (int i=0;i<total_vehicles;i++) {                                    //her arac icin dongu
                vehicles[i].x += vehicles[i].direction*vehicles[i].speed;           //arac hareketi
                if (vehicles[i].direction==+1&&vehicles[i].x>SCREEN_WIDTH)          //pencerenin sagindan disari cikinca
                    vehicles[i].x=-vehicles[i].width;                               //solundan iceri girer
                if (vehicles[i].direction==-1&&vehicles[i].x+vehicles[i].width<0)   //pencerenin solundan cikinca
                    vehicles[i].x=SCREEN_WIDTH;                                     //sagindan iceri girer
            }
            for (int i=0;i<total_platforms;i++) {                                   //her platform icin dongu
                platforms[i].x += platforms[i].direction*platforms[i].speed;        //platform hareketi
                if (platforms[i].direction==+1&&platforms[i].x>SCREEN_WIDTH)        //pencerenin sagindan disari cikinca
                    platforms[i].x= -platforms[i].width;                            //solundan iceri girer
                if (platforms[i].direction==-1&&platforms[i].x+platforms[i].width<0)//pencerenin solundan disari cikinca
                    platforms[i].x=SCREEN_WIDTH;                                    //sagindan iceri girer
            }
            int frog_lane= (int)((frog.y-HUD_HEIGHT)/LANE_HEIGHT);                  //kurbaga hangi seritte
            if (lanes[frog_lane].type==LANE_WATER) {                                //serit su seridi mi
                bool on_platform=false;                                             //platform uzerinde mi
                for (int i=0;i<total_platforms;i++) {                               //her platformu kontrol dongusu
                    if (platforms[i].lane==frog_lane) {                             //aynı seritte mi
                        if (platforms[i].type==PLATFORM_LOG) {                      //kutukse tek alan
                            on_platform=frog.x+frog.size>platforms[i].x&&           //kurbaganin sagi kutugun solunu geciyor mu
                            frog.x<platforms[i].x+platforms[i].width;               //kurbaganın solu kutugun sagini geciyor mu
                        }
                        else {                                                      //kaplumbagaysa iki alan ayni sekilde kontrol
                            on_platform=(frog.x+frog.size>platforms[i].x&&          //sol kaplumbaga
                            frog.x<platforms[i].x+35)||
                            (frog.x+frog.size>platforms[i].x+45&&                   //sag kaplumbaga
                            frog.x<platforms[i].x+80);
                        }
                        if (on_platform) {                                          //platform uzerindeyse
                            frog.x+=platforms[i].direction*platforms[i].speed;      //birlikte suruklen
                            if (frog.x<0||frog.x+frog.size>SCREEN_WIDTH) {          //ekrandan disari cıkınca
                                frog.lives--;                                       //canını azalt
                                frog.x=(SCREEN_WIDTH/2)-(frog.size/2);              //baslangic x konumuna don
                                frog.y=SCREEN_HEIGHT-LANE_HEIGHT+(LANE_HEIGHT-frog.size)/2; //baslangic y konumuna don
                                invincible_timer=30;                                //dokunulmazlik suresi
                            }
                            break;
                        }
                    }
                }
            }
            if (frog.drowning) {                                                    //batiyorsa
                frog.drown_timer--;                                                 //sayaci azaltir
                if (frog.drown_timer<=0) {                                          //animasyon bittiyse
                    frog.drowning=false;                                            //batmayi bitirir
                    frog.lives--;                                                   //cani azaltir
                    frog.x=(SCREEN_WIDTH/2)-(frog.size/2);                          //baslangic x konumu
                    frog.y=SCREEN_HEIGHT-LANE_HEIGHT+(LANE_HEIGHT-frog.size)/2;     //baslangic y konumu
                    invincible_timer=30;                                            //dokunulmazlik suresi
                }
            }
            if (lanes[frog_lane].type==LANE_WATER&&invincible_timer==0&&!frog.drowning) {   //su seridindeyse, dokunulmaz degilse ve 
                bool on_platform=false;                                             //platform uzerinde mi
                for (int i=0;i<total_platforms;i++) {                               //her platform icin kontrol dongusu
                    if (platforms[i].lane==frog_lane) {                             //ayni seritte mi
                        if (platforms[i].type==PLATFORM_LOG) {                      //kutukse
                            on_platform=frog.x+frog.size>platforms[i].x&&
                            frog.x<platforms[i].x+platforms[i].width;
                        }
                        else {                                                      //kaplumbagaysa
                            on_platform=(frog.x+frog.size>platforms[i].x&& 
                            frog.x<platforms[i].x+35)|| 
                            (frog.x+frog.size>platforms[i].x+45&&
                            frog.x<platforms[i].x+80);
                        }
                        if (on_platform)                                            //platform bulununca
                            break;                                                  //dur
                    }
                }
                if (!on_platform) {                                                 //platform uzerinde degilse
                    //frog.lives--; 
                    //frog.x=(SCREEN_WIDTH/2)-(frog.size/2);
                    //frog.y=SCREEN_HEIGHT-LANE_HEIGHT+(LANE_HEIGHT-frog.size)/2;
                    //invincible_timer=30;
                    frog.drowning=true;                                             //batma animasyonunu baslatir
                    frog.drown_timer=30;                                            //30 frame batma animasyonu
                }
            }
            if (invincible_timer>0) {                                               //dokunulmazlik suresi devam ediyorsa
                invincible_timer--;                                                 //sayaci azaltir
            }
            else {                                                                  //dokunulmazlik suresi bitmisse
                for (int i=0;i<total_vehicles;i++) {                                //carpisma kontrolu
                    if (frog.x<vehicles[i].x+vehicles[i].width&& 
                        frog.x+frog.size>vehicles[i].x&&  
                        frog.y<vehicles[i].y+30&&  
                        frog.y+frog.size>vehicles[i].y) {  
                        frog.lives--;                                               //canı 1 azaltir
                        frog.x=(SCREEN_WIDTH/2)-(frog.size/2);                      //baslangic x konumuna doner
                        frog.y=SCREEN_HEIGHT-LANE_HEIGHT+(LANE_HEIGHT-frog.size)/2; //baslangic y konumuna doner
                        invincible_timer=30;                                        //1 saniye dokunulmaz kalir
                        break;                                                      //tek seferde tek can kaybi
                    }
                }
            }
            if (lanes[frog_lane].type==LANE_SAFE) {                                 //hedef seridindeyse
                for (int i=0;i<GOAL_COUNT;i++) {                                    //her hedef icin kontrol donusu
                    if (!goals[i].filled&&frog.x+frog.size>goals[i].x&&             //dolu degilse ve kurbaga dogru yerdeyse
                    frog.x<goals[i].x+30) { 
                        goals[i].filled=true;                                       //hedefi doldurur
                        frog.x=(SCREEN_WIDTH/2)-(frog.size/2);                      //baslangic x konumuna doner
                        frog.y=SCREEN_HEIGHT-LANE_HEIGHT+(LANE_HEIGHT-frog.size)/2; //baslangic y konumuna doner
                        invincible_timer=30;                                        //dokunulmazlik suresi
                        break;
                    }
                }
                bool all_filled=true;                                               //doluluk kontrolu icin
                for (int i=0;i<GOAL_COUNT;i++) {                                    //butun hedefleri kontrol dongusu
                    if (!goals[i].filled) {                                         //bos hedef varsa
                        all_filled=false;                                           //hepsi dolu durumu yanlıs olur
                            break;
                    }
                }
                if (all_filled) {                                                   //hedeflerin hepsi dolduysa
                    level++;                                                        //seviyeyi arttırır
                    for (int i=0;i<GOAL_COUNT;i++)                                  //yeni seviyedeki her hedef icin
                        goals[i].filled=false;                                      //yerlerini tekrar bosaltir
                    for (int i=0;i<total_vehicles;i++)                              //her arac icin dongu
                        vehicles[i].speed+=vehicles[i].speed*0.2f;                  //hizi %20 arttirir
                    for (int i=0;i<total_platforms;i++)                             //her platform icin dongu
                        platforms[i].speed+=platforms[i].speed*0.2f;                //hizi %20 arttirir
                    time_left=time_limit;                                           //yeni seviyede kalan sureyi sifirlar
                }
            }   
        }   
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)                         //pencerede x'e basilirsa
            game_running = false;                                                   //oyunu kapatir
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (invincible_timer==0&&!frog.drowning) {                              //dokunulmazlik suresi bitmisse
                switch (event.keyboard.keycode) {                                   //basilan klavye tusuna gore hareketi saglayan yapi
                    case ALLEGRO_KEY_UP:
                        if (frog.y - STEP_SIZE >= 0) {                              //pencerenin ust sinirini asmiyorsa
                            frog.y -= STEP_SIZE;                                    //kurbaga yukari hareket eder
                        }
                            break;
                    case ALLEGRO_KEY_DOWN:
                        if (frog.y + STEP_SIZE < SCREEN_HEIGHT) {                   //pencere alt sinirini asmiyorsa
                            frog.y += STEP_SIZE;                                    //kurbaga asagi hareket eder
                        }
                            break;
                    case ALLEGRO_KEY_LEFT:
                        if (frog.x - STEP_SIZE >= 0) {                              //pencere sol sinirini asmiyorsa
                            frog.x -= STEP_SIZE;                                    //kurbaga sola hareket eder
                        }
                            break;
                    case ALLEGRO_KEY_RIGHT:
                        if (frog.x + STEP_SIZE < SCREEN_WIDTH) {                    //pencere sag sinirini asmiyorsa
                            frog.x += STEP_SIZE;                                    //kurbaga saga hareket eder
                        }
                            break;
                    case ALLEGRO_KEY_ESCAPE:                                        //ESC tusuna basilirsa
                        game_running = false;                                       //oyunu sonlandirir
                        break;
                }
            }
        }
        if (redraw && al_is_event_queue_empty(event_queue)) {                       //sirada bekleyen olay yoksa cizimi baslatir
            redraw = false;
            al_clear_to_color(al_map_rgb(0,0,0));                                   //pencereyi tamamen siyaha boyayarak onceki karenin kalintilarini temizler
            al_draw_filled_rectangle(                                               //hud arka planini cizer
                0, 0, SCREEN_WIDTH, HUD_HEIGHT,
                al_map_rgb(0,0,0)                                                   //siyah
            );
            for(int i=0;i<LANE_COUNT;i++) {                                         //serit sayisi kadar dongu
                float y=HUD_HEIGHT+i*LANE_HEIGHT;                                   //seritin numarasina gore cizilecegi pixel yuksekligi
                ALLEGRO_COLOR color;                                                //renk icin degisken
                switch (lanes[i].type) {                                            //serit turune gore;
                    case LANE_START:                                                //baslangic seridi ise
                        color=al_map_rgb(255,180,0);                                //acik turuncu
                        break;
                    case LANE_SAFE:                                                 //hedef seridi ise
                        color=al_map_rgb(0,255,0);                                  //acik yesil
                        break;
                    case LANE_ROAD:                                                 //asfalt seridi ise
                        color=al_map_rgb(128,128,128);                              //gri
                        break;
                    case LANE_WATER:                                                //su seridi ise
                        color=al_map_rgb(153,204,255);                              //mavi
                        break;
                    case LANE_MIDDLE:                                               //ortada beklenebilen serit ise
                        color=al_map_rgb(153,0,153);                                //mor
                        break;
                    default:                                                        //hicbiri degil ise
                        color=al_map_rgb(255,180,0);                                //acik turuncu
                        break;
                }
                al_draw_filled_rectangle(0,y,SCREEN_WIDTH,y+LANE_HEIGHT,color);     //seritleri cizer
                al_draw_line(0,y+LANE_HEIGHT,SCREEN_WIDTH,y+LANE_HEIGHT,al_map_rgb(255,180,0),1.0f);    //seritler arasina 1 pixellik acik turuncu cizgi cizer
            }
            for (int i=0;i<GOAL_COUNT;i++) {                                        //hedefler kontrol dongusu
                if (goals[i].filled) {                                              //hedef dolduysa
                    al_draw_filled_rectangle(                                       //cerceve cizer
                        goals[i].x, HUD_HEIGHT+5, goals[i].x+30,
                        HUD_HEIGHT+35, al_map_rgb(0,80,0)                           //cok koyu yesil
                    );
                    al_draw_filled_rectangle(                                       //hedefe ulasmis kurbagayi cizer
                        goals[i].x+(30-frog.size)/2, HUD_HEIGHT+5+(30-frog.size)/2,
                        goals[i].x+(30+frog.size)/2, HUD_HEIGHT+5+(30+frog.size)/2, 
                        al_map_rgb(0,102,0)                                         //koyu yesil
                    );
                }
                else {                                                              //hedef bossa
                    al_draw_filled_rectangle(                                       //bos hedef cercevesi
                        goals[i].x, HUD_HEIGHT+5, goals[i].x+30,
                        HUD_HEIGHT+35, al_map_rgb(0,180,0)                          //acik yesil
                    );
                }
            }                            
            for (int i=0;i<total_vehicles;i++) {                                    //her arac icin dongu
                ALLEGRO_COLOR vcolor;                                               //arac rengi degiskeni
                if (vehicles[i].type==VEHICLE_TRUCK)                                //kamyonsa
                    vcolor=al_map_rgb(255,230,0);                                   //sari
                else if (vehicles[i].type==VEHICLE_BIKE)                            //motorsa
                    vcolor=al_map_rgb(255,100,0);                                   //turuncu
                else                                                                //arabaysa
                    vcolor=al_map_rgb(220,50,0);                                    //kirmizi
                al_draw_filled_rectangle(                                           //araclari cizer  
                    vehicles[i].x, vehicles[i].y, 
                    vehicles[i].x+vehicles[i].width, vehicles[i].y+30,              //genisligi turune gore cizer
                    vcolor                                                          //turune gore boyar
                );
            }
            for (int i=0;i<total_platforms;i++) {                                   //her platform icin dongu
                if (platforms[i].type==PLATFORM_LOG) {                              //kutukse
                    al_draw_filled_rectangle(                                       //kutuk cizer
                        platforms[i].x, platforms[i].y,
                        platforms[i].x+platforms[i].width, platforms[i].y+30,
                        al_map_rgb(139,90,43)                                       //kahverengi
                    );
                }
                else {                                                              //kaplumbagaysa ikili cizecek
                    al_draw_filled_rectangle(                                       //birinci kaplumbagayi cizer
                        platforms[i].x, platforms[i].y,
                        platforms[i].x+35, platforms[i].y+30, 
                        al_map_rgb(0,204,204)                                       //su yesili
                    );
                    al_draw_filled_rectangle(                                       //ikinci kaplumbagayi cizer
                        platforms[i].x+45, platforms[i].y,
                        platforms[i].x+80, platforms[i].y+30, 
                        al_map_rgb(0,153,153)                                       //koyu su yesil
                    );
                }
            }
            if (frog.drowning) {                                                    //batiyorsa
                int size=frog.size*frog.drown_timer/30;                             //zamanla kuculur
                al_draw_filled_rectangle(
                    frog.x+(frog.size-size)/2, frog.y+(frog.size-size)/2,
                    frog.x+(frog.size+size)/2, frog.y+(frog.size+size)/2,
                    al_map_rgb(0,51,153)                                            //lacivert (batarken)
                );
            }
            else if (invincible_timer==0||invincible_timer%10<5) {                  //her 10 framede bir yanip soner
                al_draw_filled_rectangle(                                           //kurbagayi cizer
                    frog.x, frog.y,
                    frog.x + frog.size, frog.y + frog.size, al_map_rgb(0,102,0)     //koyu yesil
                );
            }
            char time_str[32];                                                      //sure yazisi icin karakter dizisi
            sprintf(time_str,"TIME: %d",time_left/FPS); 
            al_draw_text(font,al_map_rgb(0,0,0),10, SCREEN_HEIGHT-20, 
        ALLEGRO_ALIGN_LEFT, time_str);                                              //sol alta yazi yazar
            al_flip_display();                                                      //cizilen kurbagayi gosterir
        }
    }
    al_destroy_font(font);                                                          //oyun kapatilince fontu temizler
    al_destroy_display(display);                                                    //oyun kapatilinca pencere hafizasini temizler
    al_destroy_timer(timer);                                                        //oyun kapatilinca zamanlayici hafizasini temizler
    al_destroy_event_queue(event_queue);                                            //oyun kapatilinca sira hafizasini temizler
    return 0;
}