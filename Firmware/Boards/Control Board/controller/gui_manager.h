#pragma once

#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include "libraries/math.h"
#include "libraries/array.h"
#include "libraries/spi/spi_interface_master.h"

#include "assets/gui_icons.h"
#include "devices.h"

enum NumberFormat
{
    Real2,
    Real3,
    Integer,
    Percentual
};

const uint8_t WIDTH = 128;
const uint8_t HEIGHT = 32;
Adafruit_SSD1306 d(WIDTH, HEIGHT, &Wire, -1);

class MenuItem
{
public:
    MenuItem() {}
    MenuItem(char label[], void_function action) : label(label), action(action) {}
    MenuItem(char label[], uint8_t icon[], void_function action) : label(label), icon(icon), action(action) {}

    char *getLabel()
    {
        return label;
    }
    void setLabel(char label_[])
    {
        label = label_;
    }

    uint8_t *getIcon()
    {
        return icon;
    }
    void setIcon(uint8_t icon_[])
    {
        icon = icon_;
    }

    bool isEnabled()
    {
        return enabled;
    }
    void enable()
    {
        enabled = true;
    }
    void disable()
    {
        enabled = false;
    }
    void setState(bool enabled_)
    {
        enabled = enabled_;
    }

    void_function getAction()
    {
        return action;
    }
    void setAction(void_function action_)
    {
        action = action_;
    }
    void execAction()
    {
        action();
    }

private:
    char *label;
    bool enabled = true;
    void_function action;
    uint8_t *icon = icons::null;
};

class Menu
{
public:
    Menu() {}
    Menu(String id, Array<MenuItem> items) : id(id), items(items)
    {
        length = (int)items.size();
    }
    Menu(String id) : id(id)
    {
        length = 0;
    }
    String getId()
    {
        return id;
    }
    int getLength()
    {
        return length;
    }
    void addItem(MenuItem item)
    {
        items.push(item);
        length = (int)items.size();
    }
    MenuItem &getItem(int id)
    {
        return items[id];
    }
    void setSelectedItemIndex(int index)
    {
        selectedItemIndex = index;
    }
    int getSelectedItemIndex()
    {
        return selectedItemIndex;
    }
    bool execSelectedItemAction()
    {
        if (getSelectedItem().isEnabled())
        {
            items[selectedItemIndex].execAction();
            return true;
        }
        else
            return false;
    }
    virtual void draw() {}
    virtual bool selectNextItem() {}
    virtual bool selectPreviousItem() {}

protected:
    String id;
    Array<MenuItem> items;
    int length = 0;
    int selectedItemIndex = 0;

    MenuItem &getSelectedItem()
    {
        return items[selectedItemIndex];
    }
};

class ListMenu : public Menu
{
private:
    int scrollTop = 0;
    float smoothScrollTop = 0;

public:
    ListMenu() {}
    ListMenu(char id[]) : Menu(id) {}
    ListMenu(char id[], Array<MenuItem> items) : Menu(id, items) {}

    void draw()
    {
        if (abs(scrollTop - smoothScrollTop) > 1)
        {
            smoothScrollTop += (scrollTop - smoothScrollTop) / 4;
        }
        else
        {
            smoothScrollTop = scrollTop;
        }
        d.clearDisplay();
        d.setTextSize(1);
        d.drawBitmap(28, 2 - smoothScrollTop, icons::left_arrow, 12, 10, WHITE);
        for (int i = 0; i < length; i++)
        {
            int yPos = i * 14 - smoothScrollTop;
            if (yPos > -14 && yPos < 32)
            {
                d.drawRect(0, yPos + 1, 117, 12, WHITE);
                d.setCursor(2, yPos + 3);
                d.print(items[i].getLabel());
            }
        }
        d.fillCircle(111, (selectedItemIndex * 14 + 6) - smoothScrollTop, 2, WHITE);
        d.drawRect(119, 1, 9, 30, WHITE);
        d.fillRect(121, 3 + 27 * (smoothScrollTop / (length * 14)), 5, 27 * ((float)HEIGHT) / (length * 14), WHITE);
        d.display();
    }
    bool selectNextItem()
    {
        if (selectedItemIndex < (length - 1))
        {
            selectedItemIndex++;
            scrollTop = fmin(selectedItemIndex * 14, (length - 1) * 14 - 18);
            return true;
        }
        else
            return false;
    }
    bool selectPreviousItem()
    {
        if (selectedItemIndex > 0)
        {
            selectedItemIndex--;
            scrollTop = fmin(selectedItemIndex * 14, (length - 1) * 14 - 18);
            return true;
        }
        else
            return false;
    }
};

class MainMenu : public Menu
{
public:
    MainMenu() {}
    MainMenu(char id[]) : Menu(id) {}
    MainMenu(char id[], Array<MenuItem> items) : Menu(id, items) {}

    void draw()
    {
        if (abs(scrollLeft - smoothScrollLeft) > 1)
        {
            smoothScrollLeft += (scrollLeft - smoothScrollLeft) / 4;
        }
        else
        {
            smoothScrollLeft = scrollLeft;
        }
        d.clearDisplay();
        for (int i = 0; i < length; i++)
        {
            float x = (WIDTH * i) + 43 - smoothScrollLeft;
            if (x > -32 && x < WIDTH)
            {
                d.fillRect(x, 0, 42, 32, WHITE);
                d.fillRect(x + 2, 2, 38, 28, BLACK);
                d.drawBitmap(x + 13, 2, items[selectedItemIndex].getIcon(), 16, 16, WHITE);
                d.setTextSize(1);
                d.setCursor(x + 7, 20);
                d.print(items[selectedItemIndex].getLabel());
            }
        }
        if (selectedItemIndex > 0)
            d.fillTriangle(2, 16, 10, 12, 10, 20, WHITE);
        if (selectedItemIndex < (length - 1))
            d.fillTriangle(126, 16, 120, 12, 120, 20, WHITE);
        d.display();
    }
    bool selectNextItem()
    {
        if (selectedItemIndex < (length - 1))
        {
            selectedItemIndex++;
            scrollLeft = WIDTH * selectedItemIndex;
            return true;
        }
        else
            return false;
    }
    bool selectPreviousItem()
    {
        if (selectedItemIndex > 0)
        {
            selectedItemIndex--;
            scrollLeft = WIDTH * selectedItemIndex;
            return true;
        }
        else
            return false;
    }

private:
    int scrollLeft = 0;
    float smoothScrollLeft = 0;
};

class Gui
{
public:
    void init()
    {
        d.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        d.setTextSize(1);
        d.setTextColor(WHITE);
        clearDisplay();
        boot();
        drawActiveMenu();
    }
    void boot()
    {
        printMessage("ro_BOB 2.0", 6, 12, 2);
        // repeatFor(1000, [](int deltaTime) {
        //     double brightness = sin((TWO_PI / 500) * (float)deltaTime) + 1.0 / 2.0;
        //     d.ssd1306_command(SSD1306_SETCONTRAST);
        //     d.ssd1306_command(brightness * 255);
        //     d.ssd1306_command(SSD1306_SETVCOMDETECT);
        //     d.ssd1306_command(brightness * 40);
        //     d.ssd1306_command(SSD1306_SETPRECHARGE);
        //     d.ssd1306_command(brightness * 241);
        // });
        // d.ssd1306_command(SSD1306_SETCONTRAST);
        // d.ssd1306_command(0xFF);
        // d.ssd1306_command(SSD1306_SETVCOMDETECT);
        // d.ssd1306_command(0x40);
        // d.ssd1306_command(SSD1306_SETPRECHARGE);
        // d.ssd1306_command(0x22);
        clearDisplay();
        delay(500);
    }

    void addMenu(Menu *menu)
    {
        menus.push(menu);
    }

    Menu *getActiveMenu()
    {
        return activeMenu;
    }
    void setActiveMenu(char id[])
    {
        for (int i = 0; i < menus.size(); i++)
        {
            if (String(id) == menus[i]->getId())
            {
                activeMenu = menus[i];
            }
        }
    }

    void drawActiveMenu()
    {
        activeMenu->draw();
    }
    bool execSelectedItemAction()
    {
        return activeMenu->execSelectedItemAction();
    }
    bool selectNextItem()
    {
        return activeMenu->selectNextItem();
    }
    bool selectPreviousItem()
    {
        return activeMenu->selectPreviousItem();
    }

    template <class T>
    T numberDialog(
        T n, T min, T max, T step, Keyboard &k, NumberFormat nf, void (*update)(T) = [](T) {})
    {
        float v = step, a = step * 0.0001;
        T inc = 0;
        T newVal = n;

        k.update();
        while (!k.pressedOnce(MIDDLE))
        {
            k.update();

            if (!k.pressed(LEFT) && !k.pressed(RIGHT))
            {
                v = step;
                inc = 0;
            }
            else if (!(k.pressed(LEFT) && k.pressed(RIGHT)))
            {
                if (k.pressedRepeat(LEFT))
                {
                    v += a;
                    inc += v;
                    newVal -= inc;
                }
                if (k.pressedRepeat(RIGHT))
                {
                    v += a;
                    inc += v;
                    newVal += inc;
                }
            }

            newVal = constrain(newVal, min, max);
            update(newVal);

            d.clearDisplay();
            d.setTextSize(2);

            switch (nf)
            {
            case Real2:
                d.setCursor(49, 2);
                d.print((double)newVal, 2);
                break;
            case Real3:
                d.setCursor(44, 2);
                d.print((double)newVal, 3);
                break;
            case Integer:
                d.setCursor(49, 2);
                d.print((int)newVal);
                break;
            case Percentual:
                d.setCursor(49, 2);
                d.print((int)(newVal * 100));
                break;
            }

            d.setCursor(20, 18);
            d.print("-");
            d.setCursor(108, 18);
            d.print("+");

            d.display();
        }
        update(newVal);
        drawActionCompleted();
        return newVal;
    }
    void colorCalibrationGui(Keyboard& k, SPIMasterInterface &spi, uint8_t action, int calibrationTime)
    {
        d.clearDisplay();
        d.drawBitmap(8, 24, icons::cross_8, 8, 8, WHITE);
        d.drawBitmap(112, 24, icons::tick, 8, 8, WHITE);
        d.setTextSize(1);
        d.setCursor(0, 0);
        d.setTextWrap(true);
        d.print("Place both sensors above the color");
        d.display();
        if (waitForInput(k))
        {
            spi.execAction(action);
            drawLoadingBar("Calibrating", calibrationTime);
        }
        else
            drawActionAborted();
    }

    void drawLoadingBar(const char message[], int duration)
    {
        d.clearDisplay();
        d.setTextSize(1);
        d.setCursor(8, 8);
        d.print(message);
        d.drawRect(8, 20, 112, 8, WHITE);
        unsigned long startTime = millis();
        while (millis() - startTime < duration)
        {
            int perc = (millis() - startTime) / (duration / 100);
            d.fillRect(10, 22, perc * 1.1, 4, WHITE);
            d.display();
        }
        d.clearDisplay();
        drawActionCompleted();
    }
    void drawActionCompleted()
    {
        d.clearDisplay();
        d.drawTriangle(54, 16, 66, 24, 80, 8, WHITE);
        d.drawTriangle(54, 15, 66, 23, 80, 7, WHITE);
        d.drawTriangle(54, 14, 66, 22, 80, 6, WHITE);
        d.drawLine(54, 16, 80, 8, BLACK);
        d.drawLine(54, 15, 80, 7, BLACK);
        d.drawLine(54, 14, 80, 6, BLACK);
        d.display();
        delay(500);
    }
    void drawActionAborted()
    {
        d.clearDisplay();
        d.drawBitmap(56, 8, icons::cross_16, 16, 16, WHITE);
        d.display();
        delay(500);
    }
    void printColorData(bool left, bool right, bool aluminium)
    {
        d.clearDisplay();
        d.setTextSize(2);
        d.setCursor(0, 0);
        d.println("SX  DX  AL");
        d.print(left);
        d.print("   ");
        d.print(right);
        d.print("   ");
        d.println(aluminium);
        d.display();
    }

private:
    Array<Menu *> menus;
    Menu *activeMenu;

    bool waitForInput(Keyboard &k, bool animation = false)
    {
        while (!k.pressedOnce(MIDDLE))
        {
            k.update();
            if (k.pressedOnce(LEFT))
            {
                if (animation)
                    drawActionAborted();
                return false;
            }
            if (k.pressedOnce(RIGHT))
            {
                if (animation)
                    drawActionCompleted();
                return true;
            }
        }
    }
    void clearDisplay()
    {
        d.clearDisplay();
        d.display();
    }
    void printMessage(char message[], int x, int y, int size)
    {
        d.clearDisplay();
        d.setTextSize(size);
        d.setCursor(x, y);
        d.print(message);
        d.display();
    }
    void clearRect(int16_t x, int16_t y, int16_t w, int16_t h)
    {
        d.fillRect(x, y, w, h, BLACK);
    }
};