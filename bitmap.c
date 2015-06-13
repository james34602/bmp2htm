#include <windows.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct tagBITMAPINFOEX
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[256];
} BITMAPINFOEX, FAR * LPBITMAPINFOEX;

HBITMAP LoadBitmapFromFile(LPCTSTR pszFileName, float *dpi)
{
    HANDLE hFile;
    BITMAPFILEHEADER bf;
    BITMAPINFOEX bi;
    DWORD cb, cbImage;
    DWORD dwError;
    LPVOID pBits, pBits2;
    HDC hDC, hMemDC;
    HBITMAP hbm;

    hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return NULL;

    if (!ReadFile(hFile, &bf, sizeof(BITMAPFILEHEADER), &cb, NULL))
    {
        dwError = GetLastError();
        CloseHandle(NULL);
        SetLastError(dwError);
        return NULL;
    }

    pBits = NULL;
    if (bf.bfType == 0x4D42 && bf.bfReserved1 == 0 && bf.bfReserved2 == 0 &&
        bf.bfSize > bf.bfOffBits && bf.bfOffBits > sizeof(BITMAPFILEHEADER) &&
        bf.bfOffBits <= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOEX))
    {
        if (ReadFile(hFile, &bi, bf.bfOffBits -
                     sizeof(BITMAPFILEHEADER), &cb, NULL))
        {
#ifndef LR_LOADREALSIZE
#define LR_LOADREALSIZE 128
#endif
            *dpi = bi.bmiHeader.biXPelsPerMeter * 2.54 / 100.0;
            hbm = (HBITMAP)LoadImage(NULL, pszFileName, IMAGE_BITMAP, 
                0, 0, LR_LOADFROMFILE | LR_LOADREALSIZE | 
                LR_CREATEDIBSECTION);
            if (hbm != NULL)
            {
                CloseHandle(hFile);
                return hbm;
            }
            cbImage = bf.bfSize - bf.bfOffBits;
            pBits = HeapAlloc(GetProcessHeap(), 0, cbImage);
            if (pBits != NULL)
            {
                if (ReadFile(hFile, pBits, cbImage, &cb, NULL))
                {
                    ;
                }
                else
                {
                    dwError = GetLastError();
                    HeapFree(GetProcessHeap(), 0, pBits);
                    pBits = NULL;
                }
            }
            else
                dwError = GetLastError();
        }
        else
            dwError = GetLastError();
    }
    else
        dwError = ERROR_INVALID_DATA;
    CloseHandle(hFile);

    if (pBits == NULL)
    {
        SetLastError(dwError);
        return NULL;
    }

    hbm = NULL;
    hDC = GetDC(NULL);
    if (hDC != NULL)
    {
        hMemDC = CreateCompatibleDC(hDC);
        if (hMemDC != NULL)
        {
            hbm = CreateDIBSection(hMemDC, (BITMAPINFO*)&bi, DIB_RGB_COLORS,
                                   &pBits2, NULL, 0);
            if (hbm != NULL)
            {
                if (SetDIBits(hMemDC, hbm, 0, abs(bi.bmiHeader.biHeight),
                              pBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS))
                {
                    ;
                }
                else
                {
                    dwError = GetLastError();
                    DeleteObject(hbm);
                    hbm = NULL;
                }
            }
            else
                dwError = GetLastError();

            DeleteDC(hMemDC);
        }
        else
            dwError = GetLastError();

        ReleaseDC(NULL, hDC);
    }
    else
        dwError = GetLastError();

    HeapFree(GetProcessHeap(), 0, pBits);
    SetLastError(dwError);

    return hbm;
}

HBITMAP CopyBitmap(HBITMAP hbm)
{
    return (HBITMAP)CopyImage(hbm, IMAGE_BITMAP, 0, 0, LR_COPYRETURNORG);
}

HBITMAP CreateSolid32BppBitmap(INT cx, INT cy, COLORREF clr)
{
    BITMAPINFO bi;
    HDC hdc;
    HBITMAP hbm;
    HGDIOBJ hbmOld;
    VOID *pvBits;
    RECT rc;
    HBRUSH hbr;
    DWORD cdw;
    BYTE *pb;

    hdc = CreateCompatibleDC(NULL);
    if (hdc == NULL)
        return NULL;

    hbr = CreateSolidBrush(clr);
    if (hbr == NULL)
    {
        DeleteDC(hdc);
        return NULL;
    }

    ZeroMemory(&bi.bmiHeader, sizeof(BITMAPINFOHEADER));
    bi.bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth    = cx;
    bi.bmiHeader.biHeight   = cy;
    bi.bmiHeader.biPlanes   = 1;
    bi.bmiHeader.biBitCount = 32;
    hbm = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &pvBits, NULL, 0);
    if (hbm != NULL)
    {
        rc.left = 0;
        rc.top = 0;
        rc.right = cx;
        rc.bottom = cy;
        hbmOld = SelectObject(hdc, hbm);
        FillRect(hdc, &rc, hbr);
        SelectObject(hdc, hbmOld);
        cdw = cx * cy;
        pb = (BYTE *)pvBits;
        while(cdw--)
        {
            pb++;
            pb++;
            pb++;
            *pb++ = 0xFF;
        }
    }
    DeleteObject(hbr);
    DeleteDC(hdc);
    return hbm;
}

BOOL IsDIBOpaque(HBITMAP hbm)
{
    DWORD cdw;
    BYTE *pb;
    BITMAP bm;
    GetObject(hbm, sizeof(BITMAP), &bm);
    if (bm.bmBitsPixel <= 24)
        return TRUE;
    cdw = bm.bmWidth * bm.bmHeight;
    pb = (BYTE *)bm.bmBits;
    while(cdw--)
    {
        pb++;
        pb++;
        pb++;
        if (*pb++ != 0xFF)
            return FALSE;
    }
    return TRUE;
}

BOOL AlphaBlendBitmap(HBITMAP hbm1, HBITMAP hbm2)
{
    BITMAP bm1, bm2;
    BYTE *pb1, *pb2;
    DWORD cdw;
    INT x, y;
    BYTE a1, a2;
    GetObject(hbm1, sizeof(BITMAP), &bm1);
    GetObject(hbm2, sizeof(BITMAP), &bm2);

    if (bm1.bmBitsPixel == 32 && bm2.bmBitsPixel == 32)
    {
        pb1 = (BYTE *)bm1.bmBits;
        pb2 = (BYTE *)bm2.bmBits;
        cdw = bm1.bmWidth * bm1.bmHeight;
        while(cdw--)
        {
            a2 = pb2[3];
            a1 = (BYTE)(255 - a2);
            *pb1++ = (BYTE)((a1 * *pb1 + a2 * *pb2++) / 255);
            *pb1++ = (BYTE)((a1 * *pb1 + a2 * *pb2++) / 255);
            *pb1++ = (BYTE)((a1 * *pb1 + a2 * *pb2++) / 255);
            *pb1++ = (BYTE)((a1 * *pb1 + a2 * *pb2++) / 255);
        }
        return TRUE;
    }

    if (bm1.bmBitsPixel == 24 && bm2.bmBitsPixel == 32)
    {
        pb1 = (BYTE *)bm1.bmBits;
        pb2 = (BYTE *)bm2.bmBits;
        for(y = 0; y < bm1.bmHeight; y++)
        {
            for(x = 0; x < bm1.bmWidth; x++)
            {
                a2 = pb2[3];
                a1 = (BYTE)(255 - a2);
                *pb1++ = (BYTE)((a1 * *pb1 + a2 * *pb2++) / 255);
                *pb1++ = (BYTE)((a1 * *pb1 + a2 * *pb2++) / 255);
                *pb1++ = (BYTE)((a1 * *pb1 + a2 * *pb2++) / 255);
                pb2++;
            }
            pb1 += bm1.bmWidthBytes - bm1.bmWidth * 3;
        }
        return TRUE;
    }
    return FALSE;
}
