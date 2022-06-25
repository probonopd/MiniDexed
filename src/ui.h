//
// minidexed.h
//
// MiniDexed - Dexed FM synthesizer for bare metal Raspberry Pi
// Copyright (C) 2022  The MiniDexed Team
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _ui_h
#define _ui_h

#define U8X8_USE_PINS 1

#include <map>

#include "minidexed.h"
#include "config.h"
#include <circle/gpiomanager.h>

#include <U8g2lib.h>
#include <sensor/ky040.h>

class CPaint
{
public:
    CPaint(uint8_t sdi, uint8_t sda, uint8_t reset);
    ~CPaint();

    void Initialize();

    uint32_t GetBusClock();
    void SetBusClock(uint32_t clock_speed);

    void SetI2CAddress(uint8_t adr);
    
    void EnableUTF8Print();
    void DisableUTF8Print();

    unsigned Width();
    unsigned Height();

    void Clear();
      
    void SetPowerSave(uint8_t is_enable);
      
    void SetFlipMode(uint8_t mode);

    void SetContrast(uint8_t value);
    
    bool Begin();
    void BeginSimple();

    void SetMaxClipWindow();
    void SetClipWindow(uint8_t clip_x0, uint8_t clip_y0, uint8_t clip_x1, uint8_t clip_y1);
    
    void SendBuffer();
    void ClearBuffer();
    
    void FirstPage();
    uint8_t NextPage();
    
    uint8_t* GetBufferPtr();
    uint8_t GetBufferTileHeight();
    uint8_t GetBufferTileWidth();
    uint8_t GetPageCurrTileRow();
    void SetPageCurrTileRow(uint8_t row);
    uint8_t GetBufferCurrTileRow();
    void SetBufferCurrTileRow(uint8_t row);
    
    void SetBufferAutoClear(uint8_t mode);
    
    void UpdateDisplayArea(uint8_t  tx, uint8_t ty, uint8_t tw, uint8_t th);

    void UpdateDisplay();

    void RefreshDisplay();

    void SetDrawColor(uint8_t color_index);
    uint8_t GetDrawColor();
    void DrawPixel(uint8_t x, uint8_t y);
    void DrawHLine(uint8_t x, uint8_t y, uint8_t w);
    void DrawVLine(uint8_t x, uint8_t y, uint8_t h);
    void DrawHVLine(uint8_t x, uint8_t y, uint8_t len, uint8_t dir);

    void DrawFrame(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
    void DrawRFrame(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r);
    void DrawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
    void DrawRBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r);

    void DrawButtonUTF8(uint8_t x, uint8_t y, uint8_t flags, uint8_t width, uint8_t padding_h, uint8_t padding_v, const char* text);

    void DrawCircle(uint8_t x0, uint8_t y0, uint8_t rad, uint8_t opt = U8G2_DRAW_ALL);
    void DrawDisc(uint8_t x0, uint8_t y0, uint8_t rad, uint8_t opt = U8G2_DRAW_ALL);
    void DrawEllipse(uint8_t x0, uint8_t y0, uint8_t rx, uint8_t ry, uint8_t opt = U8G2_DRAW_ALL);
    void DrawFilledEllipse(uint8_t x0, uint8_t y0, uint8_t rx, uint8_t ry, uint8_t opt = U8G2_DRAW_ALL);

    void DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

    void SetBitmapMode(uint8_t is_transparent);
    void DrawBitmap(uint8_t x, uint8_t y, uint8_t cnt, uint8_t h, const uint8_t* bitmap);
    void DrawXBM(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* bitmap);
    void DrawXBMP(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* bitmap);
    
    void DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
      
    void SetFont(const uint8_t* font);
    void SetFontMode(uint8_t is_transparent);
    void SetFontDirection(uint8_t dir);

    int8_t GetAscent();
    int8_t GetDescent();
    
    void SetFontPosBaseline();
    void SetFontPosBottom();
    void SetFontPosTop();
    void SetFontPosCenter();

    void SetFontRefHeightText();
    void SetFontRefHeightExtendedText();
    void SetFontRefHeightAll();
    
    uint8_t DrawGlyph(uint8_t x, uint8_t y, uint16_t encoding);
    uint8_t DrawStr(uint8_t x, uint8_t y, const char* s);
    uint8_t DrawUTF8(uint8_t x, uint8_t y, const char* s);
    uint8_t DrawExtUTF8(uint8_t x, uint8_t y, uint8_t to_left, const uint16_t *kerning_table, const char* s); 
      
    uint8_t GetStrWidth(const char* s);
    uint8_t GetUTF8Width(const char* s);
    
    uint8_t UISelectionList(const char* title, uint8_t start_pos, const char* sl);
    uint8_t UIMessage(const char* title1, const char* title2, const char* title3, const char* buttons);
    uint8_t UIInputValue(const char* title, const char* pre, uint8_t* value, uint8_t lo, uint8_t hi, uint8_t digits, const char* post);

    void LCDHome();
    void LCDClear();
    void LCDNoDisplay();
    void LCDDisplay();
    void LCDSetCursor(uint8_t x, uint8_t y);
    uint8_t LCDGetCursorX();
    uint8_t LCDGetCursorY();
 
    void SleepOn();
    void SleepOff();
    void SetColorIndex(uint8_t color_index);
    uint8_t GetColorIndex();
    int8_t GetFontAscent();
    int8_t GetFontDescent();
    int8_t GetMaxCharHeight();
    int8_t GetMaxCharWidth();

private:
    U8G2_SH1122_256X64_F_HW_I2C m_display;
};

#define NUM_CONTROLS 4

class CControl
{
public:
    CControl(CMiniDexed *pMiniDexed, CGPIOManager *pGPIOManager, CConfig *pConfig);
    ~CControl();

    bool Initialize();

    CKY040& operator[](size_t index);

private:
    CMiniDexed* m_pMiniDexed;
    CGPIOManager* m_pGPIOManager;
    CConfig* m_pConfig;
    CKY040** m_ppRotaryEncoders;
};

class CScreenManager;

class CScreen
{
    friend class CScreenManager;

public:
    CScreen(unsigned id, const char* name, unsigned nPages = 1);
    virtual ~CScreen();

    static void Activate(CScreen& nxtScreen);
    void Activate();
    
    unsigned ScrollLeftPage();
    unsigned ScrollRightPage();
    unsigned CurrentScrollPage();
    unsigned ScrollToPage(unsigned pageNum);

    void SetDirty(bool isDirty);
    void Dirty(bool isDirty);
    bool IsDirty();

    void Display(CPaint& paint);

protected:
    virtual void Paint(CPaint& paint) = 0;

private:
    const unsigned m_id;
    const char* m_name;
    const unsigned m_nPages;
    unsigned m_currentScrollPage;
    bool m_isDirty;
};

class CScreenManager
{
private:
    std::map<unsigned, CScreen*> m_screenMap;

    CScreenManager();

public:
    static CScreenManager& GetInstance();

    CScreenManager(CScreenManager const&) = delete;
    void operator=(CScreenManager const&) = delete;
    
    ~CScreenManager();

    void Register(CScreen* pScreen);

    CScreen& operator[](unsigned index);

    CScreen& Activate(unsigned screenId);
};

#endif
