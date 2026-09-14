#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <pins_config.h>
#include <st7701_lcd.h>
#include <lvgl.h>
#include <time.h>

#define DRAW_BUF_SIZE (480 * 80 * 2)

st7701_lcd lcd = st7701_lcd(LCD_RST);

const String SSID = "ZC LTE";
const String PASSWORD = "11223344";
const IPAddress IP = IPAddress(0, 0, 0, 0);
const int PORT = 5000;

WiFiUDP udp;

static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
static void lcd_setup(st7701_lcd &screen);
static void lvgl_setup(lv_display_t *&disp_ref, void *&draw_buf_ref);
static void create_card(lv_obj_t *parrent, const char *title, lv_obj_t *&value_ref);
static void update_telemetry(const char *csv_data);
static void wifi_udp_setup(WiFiUDP &udp_res);
static void update_time();

lv_obj_t *val_temp;
lv_obj_t *val_hum;
lv_obj_t *val_press;
lv_obj_t *val_time;
lv_display_t *disp;
void *draw_buf;
lv_obj_t *main_cont;

void setup()
{
  lcd_setup(lcd);

  lvgl_setup(disp, draw_buf);

  // Main container handling whole window
  main_cont = lv_obj_create(lv_screen_active());
  lv_obj_set_size(main_cont, 440, 760);
  lv_obj_center(main_cont);

  // pionizing cards using flexbox
  lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // widget tree
  create_card(main_cont, "Aktualny czas", val_time);
  create_card(main_cont, "Temperatura", val_temp);
  create_card(main_cont, "Wilgotnosc", val_hum);
  create_card(main_cont, "Cisnienie", val_press);

  Serial.begin(9600);
  Serial.println("System startup...");
  delay(2000);

  wifi_udp_setup(udp);
}

void loop()
{
  int packetSize = udp.parsePacket();
  if (packetSize)
  {
    char Buffer[255];
    udp.read(Buffer, 255);
    Buffer[packetSize] = '\0';
    Serial.println(Buffer);
    update_telemetry(Buffer);
  }
  update_time();
  lv_timer_handler();
  delay(10);
}

static void update_time(){
  static unsigned long last_time_update = 0;
  if (millis() - last_time_update > 1000)
  {
    struct tm timeinfo;
    // getLocalTime próbuje odczytać czas, drugi parametr (0) oznacza brak czekania
    if (getLocalTime(&timeinfo, 0)) 
    {
      char time_str[64];
      // Formatowanie: Godzina:Minuta:Sekunda, a pod spodem Dzień.Miesiąc.Rok
      strftime(time_str, sizeof(time_str), "%H:%M:%S\n%d.%m.%Y", &timeinfo);
      lv_label_set_text(val_time, time_str);
    }
    last_time_update = millis();
  }
}

static void wifi_udp_setup(WiFiUDP &udp_res)
{
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected.");
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
  udp_res.begin(IP, PORT);
}

static void lcd_setup(st7701_lcd &screen)
{
  screen.begin();
  // screen.fillScreen(0xF800);
}

static void update_telemetry(const char *csv_data)
{
  // 1. Zamieniamy surową tablicę znaków na obiekt String
  String data = String(csv_data);

  // 2. Szukamy pozycji przecinków w tekście
  int idx1 = data.indexOf(',');
  int idx2 = data.indexOf(',', idx1 + 1);

  if (idx1 > 0 && idx2 > idx1)
  {
    // 3. Wycinamy fragmenty tekstu i od razu konwertujemy je na wartości ułamkowe
    float temperature = data.substring(0, idx1).toFloat();
    float humidity = data.substring(idx1 + 1, idx2).toFloat();
    float pressure = data.substring(idx2 + 1).toFloat();

    // 4. Składamy gotowy tekst. String(zmienna, 2) wymusza 2 miejsca po przecinku,
    // korzystając z tego samego niezawodnego mechanizmu co Serial Monitor.
    String temp_out = String(temperature, 2) + " °C";
    String hum_out = String(humidity, 2) + " %";
    String press_out = String(pressure, 2) + " Pa";

    // 5. Wysyłamy surowy tekst (c_str) do przygotowanych etykiet LVGL
    lv_label_set_text(val_temp, temp_out.c_str());
    lv_label_set_text(val_hum, hum_out.c_str());
    lv_label_set_text(val_press, press_out.c_str());
  }
  else
  {
    Serial.println("Error: CSV line incomplete - udp error");
  }
}

/*
static void update_telemetry(const char *csv_data)
{
  float temperature = 0.0f;
  float humidity = 0.0f;
  float pressure = 0.0f;

  if (sscanf(csv_data, "%f,%f,%f", &temperature, &humidity, &pressure) == 3)
  {
    lv_label_set_text_fmt(val_temp, "%.2f °C", temperature);
    lv_label_set_text_fmt(val_hum, "%.2f %%", humidity);
    lv_label_set_text_fmt(val_press, "%.2f Pa", pressure);
  }
  else
  {
    Serial.println("Error: CSV line incomplete - udp error");
  }
}
*/

static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
  uint16_t width = area->x2 - area->x1 + 1;
  uint16_t height = area->y2 - area->y1 + 1;

  // pixel package for lcd screen
  lcd.draw16bitbergbbitmap(area->x1, area->y1, width, height, (uint16_t *)px_map);

  // callback for lvgl
  lv_display_flush_ready(disp);
}

static void lvgl_setup(lv_display_t *&disp_ref, void *&draw_buf_ref)
{
  lv_init();
  lv_tick_set_cb(lv_tick_get_cb_t(millis));
  disp_ref = lv_display_create(LCD_H_RES, LCD_V_RES);
  lv_display_set_flush_cb(disp_ref, my_disp_flush);
  draw_buf_ref = heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_INTERNAL);
  lv_display_set_buffers(disp_ref, draw_buf_ref, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
}

static void create_card(lv_obj_t *parrent, const char *title, lv_obj_t *&value_ref)
{
  lv_obj_t *card = lv_obj_create(parrent);
  lv_obj_set_width(card, lv_pct(90));
  lv_obj_set_height(card, 150);

  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *label_title = lv_label_create(card);
  lv_label_set_text(label_title, title);
  lv_obj_set_style_text_font(label_title, &lv_font_montserrat_32, 0);

  value_ref = lv_label_create(card);
  lv_label_set_text(value_ref, "--.--");
  lv_obj_set_style_text_font(value_ref, &lv_font_montserrat_24, 0);
}