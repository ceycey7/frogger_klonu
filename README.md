# Frogger Klonu
Allegro5 kütüphanesi kullanılarak C dilinde geliştirilmiş klasik Frogger oyununun basit klonu.
## Oynanış
Kurbağayı trafikten geçirerek nehri aşıp hedeflere ulaştır. 3 hedef noktasının tamamlanmasıyla yeni seviyeye geçilir.
## Kontroller
| Tuş |     Eylem      |
|---|---|
|  ↑  | Yukarı hareket |
|  ↓  | Aşağı hareket  |
|  ←  | Sola hareket   |
|  →  | Sağa hareket   |
| ESC | Oyundan çık    | 
## Özellikler
- 13 şeritli oyun alanı (asfalt, su, güvenli bölge ve hedef şeridi)
- Farklı hız ve yönlerde araçlar (araba:orta hız, kamyon:düşük hız, motor:yüksek hız)
- Kütük ve kaplumbağa platformları
- Platform üzerinde sürüklenme mekaniği
- Suya düşünce boğulma animasyonu
- 5 can sistemi
- 60 saniyelik süre sayacı
- skor sistemi (ileri gitme:10p, hedefe ulaşma:50p, seviye atlama:1000p)
- seviye sistemi (her seviyede hız %20 artar)
- HUD (can, skor ve level gösterimi)
- Game Over ekranı
## Gereksinimler
- GCC
- Allegro5
- CMake 3.10+
## Derleme ve Çalıştırma
mkdir build
cd build
cmake ..
make
./frogger_klonu
## Geliştirme Süreci
Proje 1 haftalık süreçte geliştirilmiştir. Tüm commit geçmişi GitHub üzerinden takip edilebilir.