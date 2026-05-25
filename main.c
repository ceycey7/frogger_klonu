#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
int main() {
    if (!al_init()) {                                                   //allegronun baslatılmasında bir sorun olup olmadigini anlamak icin test
        fprintf(stderr,"failed to initialize allegro\n");
        return -1;
    }

    al_set_new_display_flags(ALLEGRO_WINDOWED | ALLEGRO_OPENGL);        //ekran kartimla bir uyusmazlik oldugu icin standart Direct3D yerine OpenGL motorunu zorunlu kildim
    ALLEGRO_DISPLAY* display= al_create_display(640,480);               //deneme amacli 'display' degiskeniyle basit bir oyun penceresi olusturma ve olculeri
    if(!display){                                                       //ayni sekilde ekranin olusturulmasinda meydana gelen bir sorun olup olmadigini anlamak icin test
        fprintf(stderr,"failed to create display\n");
    }
    al_clear_to_color(al_map_rgb(255,180,0));                           //yine deneme amacli arka plani siyah disinda(calisip calismadıgini anlamak icin) bir renge boyadim
    al_flip_display();                                                  //sectigimiz rengin ekranda gorulmesini saglar
    al_rest(4.0);                                                       //pencerenin otomatik olarak kapanmadan once ekranda bekleme suresi
    al_destroy_display(display);                                        //bekleme suresi sonunda pencere icin bellekte ayrilan alanı temizler
    return 0;
}