#include "ZIcon.h"

static unsigned char* LoadFile(const wchar_t* file)
{
    HANDLE hFile = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return nullptr;
    DWORD fileSize = 0;
    fileSize = GetFileSize(hFile, nullptr);

    unsigned char* pData = (unsigned char*)malloc(fileSize);
    ReadFile(hFile, pData, fileSize, &fileSize, nullptr);
    ::CloseHandle(hFile);
    return pData;
}

static void ReleaseFileData(unsigned char* pData)
{
    free(pData);
}

bool ZIcon::loadFromResource(HINSTANCE hInstance, int id, bool isCursor /*= false*/)
{
    HRSRC hSrc = FindResource(hInstance, MAKEINTRESOURCE(id), isCursor ? RT_GROUP_CURSOR : RT_GROUP_ICON);
    if (!hSrc)
        return false;
    HGLOBAL hGlobal = LoadResource(hInstance, hSrc);
    if (!hGlobal)
        return false;
    void* pData = LockResource(hGlobal);
    if (!pData)
        return false;

    m_iconType = isCursor ? Cursor : Icon;

    ICONDIR_RES* pIconDir = (ICONDIR_RES*)pData;
    std::vector<ICONDIRENTRY_RES> iconEntries;

    for (WORD i = 0; i < pIconDir->idCount; ++i)
        iconEntries.push_back(pIconDir->idEntries[i]);

    DWORD offset = sizeof(ICONDIR) - sizeof(ICONDIRENTRY) + sizeof(ICONDIRENTRY) * iconEntries.size();
    for (ICONDIRENTRY_RES& entry : iconEntries)
    {
        ICONDIRENTRY* pEntry = new ICONDIRENTRY();
        memcpy_s(pEntry, sizeof(ICONDIRENTRY) - sizeof(DWORD), &entry, sizeof(entry) - sizeof(WORD));
        pEntry->dwImageOffset = offset;
        m_entryList.push_back(pEntry);
        offset += pEntry->dwBytesInRes;

        HRSRC hIconSrc = FindResource(hInstance, MAKEINTRESOURCE(entry.wId), isCursor ? RT_CURSOR : RT_ICON);
        if (!hIconSrc)
            return false;

        DWORD dwSize = SizeofResource(hInstance, hIconSrc);
        if (dwSize != pEntry->dwBytesInRes)
            return false;

        HGLOBAL hGlobal = LoadResource(hInstance, hIconSrc);
        void* pIconData = LockResource(hGlobal);
        void* buf = malloc(pEntry->dwBytesInRes);
        memcpy_s(buf, pEntry->dwBytesInRes, pIconData, pEntry->dwBytesInRes);
        m_imageList.push_back(buf);
        FreeResource(hGlobal);
    }

    FreeResource(hGlobal);
    return true;
}

bool ZIcon::loadFromMemory(const void* data)
{
    const unsigned char* pData = (const unsigned char*)data;
    ICONDIR* pIconDir = (ICONDIR*)pData;
    if (pIconDir->idType == 0)
        m_iconType = Cursor;
    else
        m_iconType = Icon;

    for (WORD i = 0; i < pIconDir->idCount; ++i)
        m_entryList.push_back(new ICONDIRENTRY(pIconDir->idEntries[i]));
    for (ICONDIRENTRY* pEntry : m_entryList)
    {
        void* buf = malloc(pEntry->dwBytesInRes);
        memcpy_s(buf, pEntry->dwBytesInRes, pData + pEntry->dwImageOffset, pEntry->dwBytesInRes);
        m_imageList.push_back(buf);
    }
    return true;
}

bool ZIcon::loadFromFile(const char* path)
{
    wchar_t buf[1024];
    int len = MultiByteToWideChar(CP_ACP, 0, path, -1, buf, 1024);
    if (len <= 0)
        return false;
    return loadFromFile(buf);
}

bool ZIcon::loadFromFile(const wchar_t* path)
{
    if (!m_entryList.empty())
        return false;
    unsigned char* pData = LoadFile(path);
    bool result = loadFromMemory(pData);
    ReleaseFileData(pData);
    return result;
}

bool ZIcon::saveToFile(const char* path) const
{
    wchar_t buf[1024];
    int len = MultiByteToWideChar(CP_ACP, 0, path, -1, buf, 1024);
    if (len <= 0)
        return false;
    return saveToFile(buf);
}

bool ZIcon::saveToFile(const wchar_t* path) const
{
    if (m_entryList.empty())
        return false;
    if (::GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES)
    {
        if (!::DeleteFileW(path))
            return false;
    }
    HANDLE hFile = ::CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                 nullptr);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    std::vector<unsigned char> dataArray = saveToMemory();
    WriteFile(hFile, dataArray.data(), dataArray.size(), nullptr, nullptr);
    ::CloseHandle(hFile);
    return true;
}

std::vector<unsigned char> ZIcon::saveToMemory() const
{
    DWORD sizeFile = sizeof(ICONDIR) + sizeof(ICONDIRENTRY) * (m_entryList.size() - 1);
    for (ICONDIRENTRY* pEntry : m_entryList)
        sizeFile += pEntry->dwBytesInRes;

    std::vector<unsigned char> dataArray;
    dataArray.resize(sizeFile);
    ICONDIR* pIconDir = (ICONDIR*)dataArray.data();
    pIconDir->idCount = (WORD)m_entryList.size();
    pIconDir->idReserved = 0;
    pIconDir->idType = (WORD)m_iconType;

    for (size_t i = 0; i < m_entryList.size(); ++i)
    {
        ICONDIRENTRY* pEntry = m_entryList[i];
        pIconDir->idEntries[i] = *pEntry;
        memcpy_s(dataArray.data() + pEntry->dwImageOffset, pEntry->dwBytesInRes, m_imageList[i], pEntry->dwBytesInRes);
    }
    return std::move(dataArray);
}
