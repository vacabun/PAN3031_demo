#include <U8g2lib.h>

#define FONT u8g2_font_bitcasual_tr
#define MAX_DISPLAY_W 14
#define MAX_DISPLAY_H 6

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 14, 2, U8X8_PIN_NONE);

char display_memory[MAX_DISPLAY_H][MAX_DISPLAY_W + 1] = {' '};
int dis_x = 0;
int dis_y = 0;

void setup()
{
    u8g2.begin();
    u8g2.enableUTF8Print(); // enable UTF8 support for the Arduino print() function
    u8g2.setFont(FONT);     //
    Serial.begin(115200);   // opens serial port, sets data rate to 115200 bps

    for (int i = 0; i < MAX_DISPLAY_H; i++)
    {
        display_memory[i][MAX_DISPLAY_W] = '\0';
    }
}

void display_memory_add_1_char(char c)
{
    
    if (c == '\n')
    {
        
        if (++dis_x >= MAX_DISPLAY_H)
        {
            dis_x = MAX_DISPLAY_H - 1;
            for (int i = 0; i < MAX_DISPLAY_H - 1; i++)
            {
                memcpy(display_memory[i], display_memory[i + 1], MAX_DISPLAY_W);
            }
            for (int i = 0; i < MAX_DISPLAY_W; i++)
            {
                display_memory[MAX_DISPLAY_H - 1][i] = ' ';
            }
        }
        display_memory[dis_x][dis_y] = '_';
    }
    else if (c == '\r')
    {
        display_memory[dis_x][dis_y] = ' ';
        dis_y = 0;
    }
    else
    {
        display_memory[dis_x][dis_y] = c;
        if (++dis_y >= MAX_DISPLAY_W)
        {
            dis_y = 0;
            if (++dis_x >= MAX_DISPLAY_H)
            {
                dis_x = MAX_DISPLAY_H - 1;
                for (int i = 0; i < MAX_DISPLAY_H - 1; i++)
                {
                    memcpy(display_memory[i], display_memory[i + 1], MAX_DISPLAY_W);
                }
                for (int i = 0; i < MAX_DISPLAY_W; i++)
                {
                    display_memory[MAX_DISPLAY_H - 1][i] = ' ';
                }
            }
        }

        display_memory[dis_x][dis_y] = '_';
    }
    
}
void loop()
{
    while (Serial.available() > 0)
    {
        char c = Serial.read();
        display_memory_add_1_char(c);
    }
    u8g2.firstPage();
    do
    {
        for (int i = 0; i < MAX_DISPLAY_H; i++)
        {
            
            for(int j = 0; j< MAX_DISPLAY_W;j++){
              u8g2.setCursor(j*9, 14+9*i);
              u8g2.print(display_memory[i][j]);
              }
            
        }
    } while (u8g2.nextPage());
}
