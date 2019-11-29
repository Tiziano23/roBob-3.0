#pragma once

#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include "libraries/utils.h"

#include "devices.h"
#include "spi_interface_master.h"
#include "assets/gui_icons.h"

uint8_t WIDTH = 128;
uint8_t HEIGHT = 32;
Adafruit_SSD1306 d(WIDTH, HEIGHT, &Wire, -1);

class MenuItem
{
private:
    char *label;
    void_function action;
    uint8_t *icon = icons::null;

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
};

class Menu
{
protected:
    String id;
    Array<MenuItem> items;
    int length = 0;
    int selectedItemIndex = 0;

public:
    Menu() {}
    Menu(char id_[], Array<MenuItem> items_)
    {
        id = String(id_);
        items = items_;
        length = (int)items.size();
    }
    Menu(char id_[])
    {
        id = String(id_);
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
        items.add(item);
        length = (int)items.size();
    }
    void setSelectedItemIndex(int index)
    {
        selectedItemIndex = index;
    }
    int getSelectedItemIndex()
    {
        return selectedItemIndex;
    }
    void execSelectedItemAction()
    {
        items[selectedItemIndex].execAction();
    }
    virtual void draw() {}
    virtual bool selectNextItem() {}
    virtual bool selectPreviousItem() {}
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
private:
    int scrollLeft = 0;
    float smoothScrollLeft = 0;

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
};

class Gui
{
private:
    Array<Menu *> menus;
    Menu *activeMenu;

    bool waitForInput(Keyboard *k, bool animation)
    {
        while (true)
        {
            if (k->pressedOnce(0))
            {
                if (animation)
                    drawActionAborted();
                return false;
            }
            if (k->pressedOnce(2))
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
        menus.add(menu);
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

    void execSelectedItemAction()
    {
        activeMenu->execSelectedItemAction();
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
    T numberDialog(T n, T min, T max, T increment, Keyboard *k, void (*update)(T) = [](T) {})
    {
        T newN = n;
        T inc = increment;
        k->update();
        while (!k->pressedOnce(1))
        {
            k->update();

            if (k->pressedRepeat(0))
            {
                newN -= inc;
                inc += increment;
            }
            else if (k->pressedRepeat(2))
            {
                newN += inc;
                inc += increment;
            }

            if (!k->pressed(0) && !k->pressed(2))
            {
                inc = increment;
            }

            newN = constrain(newN, min, max);

            update(newN);
            d.clearDisplay();
            d.setTextSize(2);
            d.setCursor(59, 2);
            d.print(newN);
            d.setCursor(20, 18);
            d.print("-");
            d.setCursor(108, 18);
            d.print("+");
            d.display();
        }
        drawActionCompleted();
        return newN;
    }
    void colorCalibrationWizard(SPIMasterInterface *spi, Keyboard *k)
    {
        d.clearDisplay();
        d.drawBitmap(8, 24, icons::cross_8, 8, 8, WHITE);
        d.drawBitmap(112, 24, icons::tick, 8, 8, WHITE);
        d.setTextSize(1);
        d.setCursor(0, 0);
        d.setTextWrap(true);
        d.print("Put both sensors     above aluminium");
        d.display();
        if (waitForInput(k, false))
        {
            spi->execAction(CAL_COLOR);
            drawLoadingBar("Calibrating", 500);
        }
        else
        {
            spi->execAction(CAL_COLOR_ABORT);
            drawActionAborted();
            return;
        }
        clearRect(0, 0, 128, 24);
        d.print("Put both sensors on  a white surface");
        d.display();
        if (waitForInput(k, false))
        {
            spi->execAction(CAL_COLOR);
            drawLoadingBar("Calibrating", 500);
        }
        else
        {
            spi->execAction(CAL_COLOR_ABORT);
            drawActionAborted();
            return;
        }
        clearRect(0, 0, 128, 24);
        d.print("Put both sensors on  a black surface");
        d.display();
        if (waitForInput(k, false))
        {
            spi->execAction(CAL_COLOR);
            drawLoadingBar("Calibrating", 500);
        }
        else
        {
            spi->execAction(CAL_COLOR_ABORT);
            drawActionAborted();
        }
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
};