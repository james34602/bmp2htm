#include <windows.h>

#include <tchar.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "img2html.h"

HBITMAP g_hbm = NULL;

void Help(void)
{
    fprintf(stderr, ".bmp to .htm with color\n");
}

INT GetImageType(LPCTSTR pszOutputFileName)
{
    LPCTSTR pch;
    pch = _tcsrchr(pszOutputFileName, '.');
    if (pch == NULL)
        return 0;
    if (lstrcmpi(pch, ".bmp") == 0)
        return BMP;
    return 0;
}

BOOL Open(LPCTSTR pszInput)
{
    HBITMAP hbm;
    float dpi;
    INT i;
    
    i = GetImageType(pszInput);
    switch(i)
    {
    case BMP:
        hbm = LoadBitmapFromFile(pszInput, &dpi);
        break;
    }

    if (hbm != NULL)
    {
        g_hbm = hbm;
        return TRUE;
    }
    return FALSE;
}

BOOL Save(LPCTSTR pszOutput)
{
    BITMAP bm;
    HDC hdc;
    HGDIOBJ hbmOld;
    COLORREF clr;
    FILE *fout;

    if (!GetObject(g_hbm, sizeof(BITMAP), &bm))
        return FALSE;

    fout = fopen(pszOutput, "w");
    if (fout == NULL)
        return FALSE;

    hdc = CreateCompatibleDC(NULL);
    hbmOld = SelectObject(hdc, g_hbm);
    {
        int x, y;
        fprintf(fout, "<html><body>\n");
        fprintf(fout, "<!-- img2html by Katayama Hirofumi MZ -->\n");
        fprintf(fout, "<table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" style=\"empty-cells: show;\">\n");
        for (y = 0; y < bm.bmHeight; ++y)
        {
            fprintf(fout, "\t<tr>");
            for (x = 0; x < bm.bmWidth; ++x)
            {
                clr = GetPixel(hdc, x, y);
                fprintf(fout, "<td width=\"1\" height=\"1\" bgcolor=\"#%02X%02X%02X\"></td>",
                    GetRValue(clr),
                    GetGValue(clr),
                    GetBValue(clr)
                );
            }
            fprintf(fout, "</tr>\n");
        }
        fprintf(fout, "</table>\n");
        fprintf(fout, "<!-- img2html by Katayama Hirofumi MZ -->\n");
        fprintf(fout, "</body></html>\n");
    }
    SelectObject(hdc, hbmOld);
    DeleteDC(hdc);

    fclose(fout);
    return TRUE;
}

int main(int argc, char **argv)
{
    OPENFILENAME ofn;
    LPTSTR pchInputFile = NULL, pchOutputFile = NULL;
    TCHAR szInputFile[MAX_PATH], szOutputFile[MAX_PATH];

    if (argc >= 2)
    {
        if (lstrcmpi(argv[1], "--help") == 0 ||
            lstrcmpi(argv[1], "/?") == 0)
        {
            Help();
            return 0;
        }
    }

    if (argc <= 1)
    {
        szInputFile[0] = 0;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFilter = TEXT("画像 (*.bmp)\0*.bmp\0\0*.*\0");
        ofn.lpstrFile = szInputFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = TEXT("Open");
        ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_LONGNAMES |
                    OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (GetOpenFileName(&ofn))
        {
            pchInputFile = szInputFile;
        }
        else
        {
            return 3;
        }
    }
    else
    {
        pchInputFile = argv[1];
    }

    if (!Open(pchInputFile))
    {
        printf("%s: ファイルが開けないか、形式が間違っています。\n", argv[1]);
        return 2;
    }

    if (argc <= 2)
    {
        szOutputFile[0] = 0;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFilter = TEXT("HTMLファイル (*.htm;*.html)\0*.htm;*.html\0");
        ofn.lpstrFile = szOutputFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = TEXT("HTMLファイルの保存");
        ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_LONGNAMES |
                    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
        ofn.lpstrDefExt = TEXT("htm");
        if (GetSaveFileName(&ofn))
        {
            pchOutputFile = szOutputFile;
        }
        else
        {
            return 4;
        }
    }
    else
    {
        pchOutputFile = argv[2];
    }

    if (!Save(pchOutputFile))
    {
        printf("%s: ファイルが出力できませんでした。\n", pchOutputFile);
        DeleteObject(g_hbm);
        return 5;
    }

    DeleteObject(g_hbm);

    return 0;
}
