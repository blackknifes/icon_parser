#ifndef __ZICON__
#define __ZICON__
#include <Windows.h>
#include <vector>

#pragma pack(push)
#pragma pack(1)


typedef struct
{
    BYTE        bWidth;
    BYTE        bHeight;
    BYTE        bColorCount;
    BYTE        bReserved;
    WORD        wPlanes;
    WORD        wBitCount;
    DWORD       dwBytesInRes;
    WORD        wId;						//×ÊÔ´id
} ICONDIRENTRY_RES, *LPICONDIRENTRY_RES;

typedef struct
{
    WORD				idReserved;
    WORD				idType;
    WORD				idCount;
    ICONDIRENTRY_RES	idEntries[1];
} ICONDIR_RES, *LPICONDIR_RES;

typedef struct
{
    BYTE        bWidth;
    BYTE        bHeight;
    BYTE        bColorCount;
    BYTE        bReserved;
    WORD        wPlanes;
    WORD        wBitCount;
    DWORD       dwBytesInRes;
    DWORD       dwImageOffset;
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
    WORD           idReserved;
    WORD           idType;
    WORD           idCount;
    ICONDIRENTRY   idEntries[1];
} ICONDIR, *LPICONDIR;

typedef struct
{
    BITMAPINFOHEADER	icHeader;
    RGBQUAD				icColors[1];
    BYTE				icXOR[1];
    BYTE				icAND[1];
    BYTE				icData[1];
} ICONIMAGE, *LPICONIMAGE;
#pragma pack(pop)

class ZIcon
{
public:
    enum IconType
    {
        Cursor = 0,
        Icon = 1
    };

    bool loadFromResource(HINSTANCE hInstance, int id, bool isCursor = false);
    bool loadFromMemory(const void* data);
    bool loadFromFile(const char* path);
    bool loadFromFile(const wchar_t* path);

    bool saveToFile(const char* path) const;
    bool saveToFile(const wchar_t* path) const;

    std::vector<unsigned char> saveToMemory() const;
private:
    IconType m_iconType;
    std::vector<ICONDIRENTRY*> m_entryList;
    std::vector<void*> m_imageList;
};
#endif