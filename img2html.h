enum
{
    BMP,
    GIF,
    JPEG,
    TIFF
};

/* bitmap.c */
HBITMAP LoadBitmapFromFile(LPCTSTR pszFileName, float *dpi);

/* gif.c */
HBITMAP LoadGifAsBitmap(LPCSTR pszFileName);

/* jpeg.c */
HBITMAP LoadJpegAsBitmap(LPCSTR pszFileName, float *dpi);

/* tiff.c */
HBITMAP LoadTiffAsBitmap(LPCSTR pszFileName, float *dpi);
