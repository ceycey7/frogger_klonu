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
struct Frog {                                                                       //kurbaganin ozellikleri
    float x;                                                                        //yatay konumu
    float y;                                                                        //dikey konumu
    int lives;                                                                      //kalan can sayisi
    int size;                                                                       //pixel boyutu
};
int main(){
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
    frog.y= SCREEN_HEIGHT-LANE_HEIGHT+(LANE_HEIGHT-frog.size)/2;                    //y ekseninde kurbaga baslangic seridinin tam ortasinda                                 //y ekseninde pencerenin en altinda
    frog.lives= 3;                                                                  //baslangicta 3 cani var
    bool game_running= true;                                                        //oyun dongusunu acik tutar
    bool redraw= true;                                                              //cizim kontrolu yapar
    al_start_timer(timer);                                                          //zamanlayiciyi baslatir
    while (game_running) {                                                          //oyun acikken sürekli dongu
        ALLEGRO_EVENT event;  
        al_wait_for_event(event_queue, &event);                                     //yeni hareket komutu bekler ve geleni 'event' degiskenine kaydeder
        if (event.type == ALLEGRO_EVENT_TIMER)                                      //zamanlayicidan sinyal bekler
            redraw = true;                                                          //gelince goruntu yeniler
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
                        color=al_map_rgb(255,180,0);                                //turuncu
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
                        color=al_map_rgb(255,180,0);                                //turuncu
                        break;
                }
                al_draw_filled_rectangle(0,y,SCREEN_WIDTH,y+LANE_HEIGHT,color);     //seritleri cizer
                al_draw_line(0,y+LANE_HEIGHT,SCREEN_WIDTH,y+LANE_HEIGHT,al_map_rgb(255,180,0),1.0f);    //seritler arasina 1 pixellik turuncu cizgi cizer
            }                            
            al_draw_filled_rectangle(                                               //kurbagayi cizer, koyu yesil
                frog.x, frog.y, frog.x + frog.size, frog.y + frog.size, al_map_rgb(0,102,0)
            );
            al_flip_display();                                                      //cizilen kurbagayi gosterir
        }
    }
    al_destroy_display(display);                                                    //oyun kapatilinca pencere hafizasini temizler
    al_destroy_timer(timer);                                                        //oyun kapatilinca zamanlayici hafizasini temizler
    al_destroy_event_queue(event_queue);                                            //oyun kapatilinca sira hafizasini temizler
    return 0;
}