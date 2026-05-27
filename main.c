#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>                                            //geometrik sekiller icin
#define FPS 60                                                                      //saniyede kare yenilenme hizi
#define SCREEN_WIDTH 640                                                            //oyun penceresi yatay genislik
#define SCREEN_HEIGHT 520                                                           //oyun penceresi dikey genislik
#define STEP_SIZE 40                                                                //yon tuslariyla kurbaganin kac pixel hareket edecegi
#define LANE_COUNT 13                                                               //serit sayisi
#define LANE_HEIGHT 40                                                              //serit yuksekligi
#define VEHICLES_PER_LANE 3                                                         //her seritteki arac sayisi
#define TOTAL_VEHICLES (LANE_COUNT*VEHICLES_PER_LANE)                               //toplam arac sayisi
struct Frog {                                                                       //kurbaganin ozellikleri
    float x;                                                                        //yatay konumu
    float y;                                                                        //dikey konumu
    int lives;                                                                      //kalan can sayisi
    int size;                                                                       //pixel boyutu
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
int main() {
    if (!al_init())                                                                 //allegro ana motor
        return -1;                                                                  //baslatilamazsa -1 dondur
    if (!al_install_keyboard())                                                     //klavyeyi etkinlestirir
        return -1;                                          
    if (!al_init_primitives_addon())                                                //geometrik sekiller icin (kare kurbaga)
        return -1;                                     
    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_OPENGL);                    //ekran kartimla uyusmazlik oldugu icin standart DirectX yerine OpenGL kullanımını zotunlu tuttum
    ALLEGRO_TIMER *timer= al_create_timer(1.0/FPS);                                 //oyun hizini sabitler
    ALLEGRO_EVENT_QUEUE *event_queue= al_create_event_queue();                      //hareketleri siraya alir
    ALLEGRO_DISPLAY *display= al_create_display(SCREEN_WIDTH,SCREEN_HEIGHT);        //pencereyi olusturan satir
    if(!timer||!event_queue||!display)                                              //yukaridakiler yapilamazsa oyunu kapatir
        return -1;
    al_register_event_source(event_queue,al_get_display_event_source(display));     //pencereyi kapatma hareketini siraya alir
    al_register_event_source(event_queue,al_get_timer_event_source(timer));         //zamanlayicinin sinyallerini siraya alir
    al_register_event_source(event_queue,al_get_keyboard_event_source());           //klavye hareketlerini siraya alir
    struct Frog frog;                                                               //kurbagayi nesne olarak uretir
    frog.size= 30;                                                                  //boyutu 30 pixel
    frog.x= (SCREEN_WIDTH/2)-(frog.size/2);                                         //x ekseninde pencerenin tam ortasinda
    frog.y= SCREEN_HEIGHT-LANE_HEIGHT+(LANE_HEIGHT-frog.size)/2;                    //y ekseninde kurbaga baslangic seridinin tam ortasinda
    frog.lives= 3;                                                                  //baslangicta 3 cani var
    Vehicle vehicles[TOTAL_VEHICLES];                                               //arac dizisi
    int v=0;                                                                        //arac sayaci
    for (int i=0;i<LANE_COUNT;i++) {                                                //serit kontrol dongusu
        if (lanes[i].type != LANE_ROAD)                                             //asfalt seridi degilse
            continue;                                                               //atla
        for (int j=0;j<VEHICLES_PER_LANE;j++) {                                     //her seritteki araba sayisi dongusu
            vehicles[v].lane=i;                                                     //hangi seritte
            vehicles[v].direction=lanes[i].direction;                               //seridin yonunu kullan
            vehicles[v].speed=lanes[i].speed;                                       //seridin hizini kullan
            vehicles[v].y=i*LANE_HEIGHT+(LANE_HEIGHT-30)/2;                         //y ekseninde seridin ortasinda
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
    bool game_running= true;                                                        //oyun dongusunu acik tutar
    bool redraw= true;                                                              //cizim kontrolu yapar
    al_start_timer(timer);                                                          //zamanlayiciyi baslatir
    while (game_running) {                                                          //oyun acikken sürekli dongu
        ALLEGRO_EVENT event;                                                        //olay bilgisini tutmak icin degisken
        al_wait_for_event(event_queue, &event);                                     //yeni hareket komutu bekler ve geleni 'event' degiskenine kaydeder
        if (event.type == ALLEGRO_EVENT_TIMER) {                                    //zamanlayicidan sinyal bekler
            redraw = true;                                                          //gelince goruntu yeniler
            for (int i=0;i<total_vehicles;i++) {                                    //her arac icin dongu
                vehicles[i].x += vehicles[i].direction*vehicles[i].speed;           //arac hareketi
                if (vehicles[i].direction==+1&&vehicles[i].x>SCREEN_WIDTH)          //pencerenin sagindan dısari cikinca
                    vehicles[i].x=-vehicles[i].width;                               //solundan iceri gir
                if (vehicles[i].direction==-1&&vehicles[i].x+vehicles[i].width<0)   //pencerenin solundan cikinca
                    vehicles[i].x=SCREEN_WIDTH;                                     //sagindan iceri gir
            }
        }
        else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)                         //pencerede x'e basilirsa
            game_running = false;                                                   //oyunu kapatir
        else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (event.keyboard.keycode) {                                       //basilan klavye tusuna gore hareketi saglayan yapi
                case ALLEGRO_KEY_UP:
                    if (frog.y - STEP_SIZE >= 0) {                                  //pencerenin ust sinirini asmiyorsa
                        frog.y -= STEP_SIZE;                                        //kurbaga yukari hareket eder
                    }
                        break;
                case ALLEGRO_KEY_DOWN:
                    if (frog.y + STEP_SIZE < SCREEN_HEIGHT) {                       //pencere alt sinirini asmiyorsa
                        frog.y += STEP_SIZE;                                        //kurbaga asagi hareket eder
                    }
                        break;
                case ALLEGRO_KEY_LEFT:
                    if (frog.x - STEP_SIZE >= 0) {                                  //pencere sol sinirini asmiyorsa
                        frog.x -= STEP_SIZE;                                        //kurbaga sola hareket eder
                    }
                        break;
                case ALLEGRO_KEY_RIGHT:
                    if (frog.x + STEP_SIZE < SCREEN_WIDTH) {                        //pencere sag sinirini asmiyorsa
                        frog.x += STEP_SIZE;                                        //kurbaga saga hareket eder
                    }
                        break;
                case ALLEGRO_KEY_ESCAPE:                                            //ESC tusuna basilirsa
                    game_running = false;                                           //oyunu sonlandirir
                    break;
            }
        }
        if (redraw && al_is_event_queue_empty(event_queue)) {                       //sirada bekleyen olay yoksa cizimi baslatir
            redraw = false;
            al_clear_to_color(al_map_rgb(0,0,0));                                   //pencereyi tamamen siyaha boyayarak onceki karenin kalintilarini temizler
            for(int i=0;i<LANE_COUNT;i++) {                                         //serit sayisi kadar dongu
                float y=i*LANE_HEIGHT;                                              //seritin numarasina gore cizilecegi pixel yuksekligi
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
                        color=al_map_rgb(51,153,255);                               //mavi
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
            al_draw_filled_rectangle(                                               //kurbagayi cizer
                frog.x, frog.y,
                frog.x + frog.size, frog.y + frog.size, al_map_rgb(0,102,0)         //koyu yesil
            );
            al_flip_display();                                                      //cizilen kurbagayi gosterir
        }
    }
    al_destroy_display(display);                                                    //oyun kapatilinca pencere hafizasini temizler
    al_destroy_timer(timer);                                                        //oyun kapatilinca zamanlayici hafizasini temizler
    al_destroy_event_queue(event_queue);                                            //oyun kapatilinca sira hafizasini temizler
    return 0;
}