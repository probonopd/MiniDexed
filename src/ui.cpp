#include "ui.h"

#include <assert.h>

LOGMODULE ("ui");

/* CPaint */
CPaint::CPaint(uint8_t sdi, uint8_t sda, uint8_t reset) : m_display(U8G2_R0, reset, sdi, sda)
{
}

CPaint::~CPaint()
{
    this->m_display.clearDisplay();
}

void CPaint::Initialize()
{
    this->m_display.initDisplay();
}

uint32_t CPaint::GetBusClock()
{
    return this->m_display.getBusClock();
}

void CPaint::SetBusClock(uint32_t clock_speed)
{
    this->m_display.setBusClock(clock_speed);
}

void CPaint::SetI2CAddress(uint8_t adr)
{
    this->m_display.setI2CAddress(adr);
}

void CPaint::EnableUTF8Print()
{
    this->m_display.enableUTF8Print();
}

void CPaint::DisableUTF8Print()
{
    this->m_display.disableUTF8Print();
}

unsigned CPaint::Width()
{
    return this->m_display.getDisplayWidth();
}

unsigned CPaint::Height()
{
    return this->m_display.getDisplayHeight();
}

void CPaint::Clear()
{
    this->m_display.clearDisplay();
}

void CPaint::SetPowerSave(uint8_t is_enable)
{
	this->m_display.setPowerSave(is_enable);
}
    
void CPaint::SetFlipMode(uint8_t mode)
{
	this->m_display.setFlipMode(mode);
}

void CPaint::SetContrast(uint8_t value)
{
	this->m_display.setContrast(value);
}

bool CPaint::Begin()
{
	this->m_display.begin();
}

void CPaint::BeginSimple()
{
	this->m_display.beginSimple();
}

void CPaint::SetMaxClipWindow()
{
	this->m_display.setMaxClipWindow();
}

void CPaint::SetClipWindow(uint8_t clip_x0, uint8_t clip_y0, uint8_t clip_x1, uint8_t clip_y1)
{
	this->m_display.setClipWindow(clip_x0, clip_y0, clip_x1, clip_y1);
}

void CPaint::SendBuffer()
{
	this->m_display.sendBuffer();
}
void CPaint::ClearBuffer()
{
	this->m_display.clearBuffer();
}

void CPaint::FirstPage()
{
	this->m_display.firstPage();
}
uint8_t CPaint::NextPage()
{
	this->m_display.nextPage();
}

uint8_t* CPaint::GetBufferPtr()
{
	return this->m_display.getBufferPtr();
}

uint8_t CPaint::GetBufferTileHeight()
{
	return this->m_display.getBufferTileHeight();
}

uint8_t CPaint::GetBufferTileWidth()
{
	return this->m_display.getBufferTileWidth();
}

uint8_t CPaint::GetPageCurrTileRow()
{
	return this->m_display.getPageCurrTileRow();
}

void CPaint::SetPageCurrTileRow(uint8_t row)
{
	this->m_display.setPageCurrTileRow(row);
}

uint8_t CPaint::GetBufferCurrTileRow()
{
	return this->m_display.getBufferCurrTileRow();
}

void CPaint::SetBufferCurrTileRow(uint8_t row)
{
	this->m_display.setBufferCurrTileRow(row);
}

void CPaint::SetBufferAutoClear(uint8_t mode)
{
	this->m_display.setAutoPageClear(mode);
}

void CPaint::UpdateDisplayArea(uint8_t  tx, uint8_t ty, uint8_t tw, uint8_t th)
{
	this->m_display.updateDisplayArea(tx, ty, tw, th);
}

void CPaint::UpdateDisplay()
{
	this->m_display.updateDisplay();
}

void CPaint::RefreshDisplay()
{
	this->m_display.refreshDisplay();
}

void CPaint::SetDrawColor(uint8_t color_index)
{
	this->m_display.setDrawColor(color_index);
}

uint8_t CPaint::GetDrawColor()
{
	this->m_display.getDrawColor();
}

void CPaint::DrawPixel(uint8_t x, uint8_t y)
{
	this->m_display.drawPixel(x, y);
}

void CPaint::DrawHLine(uint8_t x, uint8_t y, uint8_t w)
{
	this->m_display.drawHLine(x, y, w);
}

void CPaint::DrawVLine(uint8_t x, uint8_t y, uint8_t h)
{
	this->m_display.drawVLine(x, y, h);
}

void CPaint::DrawHVLine(uint8_t x, uint8_t y, uint8_t len, uint8_t dir)
{
	this->m_display.drawHVLine(x, y, len, dir);
}

void CPaint::DrawFrame(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	this->m_display.drawFrame(x, y, w, h);
}

void CPaint::DrawRFrame(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r)
{
	this->m_display.drawRFrame(x, y, w, h, r);
}

void CPaint::DrawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	this->m_display.drawBox(x, y, w, h);
}

void CPaint::DrawRBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t r)
{
	this->m_display.drawRBox(x, y, w, h, r);
}

void CPaint::DrawButtonUTF8(uint8_t x, uint8_t y, uint8_t flags, uint8_t width, uint8_t padding_h, uint8_t padding_v, const char* text)
{
	this->m_display.drawButtonUTF8(x, y, flags, width, padding_h, padding_v, text);
}

void CPaint::DrawCircle(uint8_t x0, uint8_t y0, uint8_t rad, uint8_t opt)
{
	this->m_display.drawCircle(x0, y0, rad, opt);
}

void CPaint::DrawDisc(uint8_t x0, uint8_t y0, uint8_t rad, uint8_t opt)
{
	this->m_display.drawDisc(x0, y0, rad, opt);
}

void CPaint::DrawEllipse(uint8_t x0, uint8_t y0, uint8_t rx, uint8_t ry, uint8_t opt)
{
	this->m_display.drawEllipse(x0, y0, rx, ry, opt);
}

void CPaint::DrawFilledEllipse(uint8_t x0, uint8_t y0, uint8_t rx, uint8_t ry, uint8_t opt)
{
	this->m_display.drawFilledEllipse(x0, y0, rx, ry, opt);
}

void CPaint::DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	this->m_display.drawLine(x1, y1, x2, y2);
}

void CPaint::SetBitmapMode(uint8_t is_transparent)
{
	this->m_display.setBitmapMode(is_transparent);
}

void CPaint::DrawBitmap(uint8_t x, uint8_t y, uint8_t cnt, uint8_t h, const uint8_t* bitmap)
{
	this->m_display.drawBitmap(x, y, cnt, h, bitmap);
}

void CPaint::DrawXBM(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* bitmap)
{
	this->m_display.drawXBM(x, y, w, h, bitmap);
}

void CPaint::DrawXBMP(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* bitmap)
{
	this->m_display.drawXBMP(x, y, w, h, bitmap);
}

void CPaint::DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	this->m_display.drawTriangle(x0, y0, x1, y1, x2, y2);
}
    
void CPaint::SetFont(const uint8_t* font)
{
	this->m_display.setFont(font);
}

void CPaint::SetFontMode(uint8_t is_transparent)
{
	this->m_display.setFontMode(is_transparent);
}

void CPaint::SetFontDirection(uint8_t dir)
{
	this->m_display.setFontDirection(dir);
}

int8_t CPaint::GetAscent()
{
	return this->m_display.getAscent();
}

int8_t CPaint::GetDescent()
{
	return this->m_display.getDescent();
}

void CPaint::SetFontPosBaseline()
{
	this->m_display.setFontPosBaseline();
}

void CPaint::SetFontPosBottom()
{
	this->m_display.setFontPosBottom();
}

void CPaint::SetFontPosTop()
{
	this->m_display.setFontPosTop();
}

void CPaint::SetFontPosCenter()
{
	this->m_display.setFontPosCenter();
}

void CPaint::SetFontRefHeightText()
{
	this->m_display.setFontRefHeightText();
}

void CPaint::SetFontRefHeightExtendedText()
{
	this->m_display.setFontRefHeightExtendedText();
}

void CPaint::SetFontRefHeightAll()
{
	this->m_display.setFontRefHeightAll();
}

uint8_t CPaint::DrawGlyph(uint8_t x, uint8_t y, uint16_t encoding)
{
	this->m_display.drawGlyph(x, y, encoding);
}

uint8_t CPaint::DrawStr(uint8_t x, uint8_t y, const char* s)
{
	this->m_display.drawStr(x, y, s);
}

uint8_t CPaint::DrawUTF8(uint8_t x, uint8_t y, const char* s)
{
	this->m_display.drawUTF8(x, y, s);
}

uint8_t CPaint::DrawExtUTF8(uint8_t x, uint8_t y, uint8_t to_left, const uint16_t *kerning_table, const char* s)
{
	this->m_display.drawExtUTF8(x, y, to_left, kerning_table, s);
} 
    
uint8_t CPaint::GetStrWidth(const char* s)
{
	return this->m_display.getStrWidth(s);
}

uint8_t CPaint::GetUTF8Width(const char* s)
{
	return this->m_display.getUTF8Width(s);
}

uint8_t CPaint::UISelectionList(const char* title, uint8_t start_pos, const char* sl)
{
	this->m_display.userInterfaceSelectionList(title, start_pos, sl);
}

uint8_t CPaint::UIMessage(const char* title1, const char* title2, const char* title3, const char* buttons)
{
	this->m_display.userInterfaceMessage(title1, title2, title3, buttons);
}

uint8_t CPaint::UIInputValue(const char* title, const char* pre, uint8_t* value, uint8_t lo, uint8_t hi, uint8_t digits, const char* post)
{
	return this->m_display.userInterfaceInputValue(title, pre, value, lo, hi, digits, post);
}

void CPaint::LCDHome()
{
	this->m_display.home();
}

void CPaint::LCDClear()
{
	this->m_display.clear();
}

void CPaint::LCDNoDisplay()
{
	this->m_display.noDisplay();
}

void CPaint::LCDDisplay()
{
	this->m_display.display();
}

void CPaint::LCDSetCursor(uint8_t x, uint8_t y)
{
	this->m_display.setCursor(x, y);
}

uint8_t CPaint::LCDGetCursorX()
{
	this->m_display.getCursorX();
}

uint8_t CPaint::LCDGetCursorY()
{
	this->m_display.getCursorY();
}

void CPaint::SleepOn()
{
	this->m_display.sleepOn();
}

void CPaint::SleepOff()
{
	this->m_display.sleepOff();
}

void CPaint::SetColorIndex(uint8_t color_index)
{
	this->m_display.setColorIndex(color_index);
}

uint8_t CPaint::GetColorIndex()
{
	this->m_display.getColorIndex();
}

int8_t CPaint::GetFontAscent()
{
	this->m_display.getFontAscent();
}

int8_t CPaint::GetFontDescent()
{
	this->m_display.getFontDescent();
}

int8_t CPaint::GetMaxCharHeight()
{
	this->m_display.getMaxCharHeight();
}

int8_t CPaint::GetMaxCharWidth()
{
	this->m_display.getMaxCharWidth();
}

/* CControl */
CControl::CControl(CMiniDexed *pMiniDexed, CGPIOManager *pGPIOManager, CConfig *pConfig) : 
    m_pMiniDexed(pMiniDexed),
    m_pGPIOManager(pGPIOManager),
    m_pConfig(pConfig),
    m_ppRotaryEncoders(nullptr)
{
}

CControl::~CControl()
{
    if(this->m_ppRotaryEncoders)
    {
        for(int i = 0; i < NUM_CONTROLS; i++)
        {
            delete this->m_ppRotaryEncoders[i];
        }

        delete []this->m_ppRotaryEncoders;
    }
}

bool CControl::Initialize()
{
    assert(this->m_pConfig);

    if(this->m_pConfig->GetEncoderEnabled()) 
    {
        this->m_ppRotaryEncoders = new CKY040*[4];
        for(int i = 0; i < NUM_CONTROLS; i++) {
            this->m_ppRotaryEncoders[i] = new CKY040(
                this->m_pConfig->GetEncoderPinClock(),
                this->m_pConfig->GetEncoderPinData(),
                this->m_pConfig->GetEncoderPinSwitch(),
                this->m_pGPIOManager
            );
        }
    }

    return true;
}

CKY040& CControl::operator[](size_t index)
{
    return *this->m_ppRotaryEncoders[index % NUM_CONTROLS];
}

/* CScreen */
CScreen::CScreen(unsigned id, const char* name, unsigned nPages) : 
    m_id(id), 
	m_name(name),
    m_nPages(nPages), 
    m_currentScrollPage(0),
    m_isDirty(false)
{
}

CScreen::~CScreen()
{
}

void CScreen::Activate(CScreen& nxtScreen)
{
    nxtScreen.SetDirty(true);
}

void CScreen::Activate()
{
    CScreen::Activate(*this);
}

unsigned CScreen::ScrollLeftPage()
{
    unsigned nxPage = (this->m_currentScrollPage + 1) % this->m_nPages;
    this->Dirty(this->m_currentScrollPage != nxPage);
    this->m_currentScrollPage = nxPage;

    return this->m_currentScrollPage;
}

unsigned CScreen::ScrollRightPage()
{
    unsigned nxPage = (this->m_currentScrollPage + this->m_nPages - 1) % this->m_nPages;
    this->Dirty(this->m_currentScrollPage != nxPage);
    this->m_currentScrollPage = nxPage;

    return this->m_currentScrollPage;
}

unsigned CScreen::CurrentScrollPage()
{
    return this->m_currentScrollPage;
}

unsigned CScreen::ScrollToPage(unsigned pageNum)
{
    unsigned nxPage = pageNum % this->m_nPages;
    this->Dirty(this->m_currentScrollPage != nxPage);
    this->m_currentScrollPage = nxPage;

    return this->m_currentScrollPage;
}

void CScreen::SetDirty(bool isDirty)
{
    this->m_isDirty = isDirty;
}

void CScreen::Dirty(bool isDirty)
{
    this->m_isDirty |= isDirty;
}

bool CScreen::IsDirty()
{
    return this->m_isDirty;
}

void CScreen::Display(CPaint& paint)
{
    if(this->IsDirty())
    {
        this->Paint(paint);
    }
}


CScreenManager& CScreenManager::GetInstance()
{
	static CScreenManager inst;

	return inst;
}

CScreenManager::CScreenManager() : 
	m_screenMap()
{
}

CScreenManager::~CScreenManager()
{
}

void CScreenManager::Register(CScreen* pScreen)
{
	this->m_screenMap[pScreen->m_id] = pScreen;
}

CScreen& CScreenManager::operator[](unsigned index)
{
	return *(this->m_screenMap[index]);
}

CScreen& CScreenManager::Activate(unsigned screenId)
{
	CScreen& nxtScreen = (*this)[screenId];
	CScreen::Activate(nxtScreen);
	
	return nxtScreen;
}
