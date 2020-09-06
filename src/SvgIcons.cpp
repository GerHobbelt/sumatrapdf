/* Copyright 2020 the SumatraPDF project authors (see AUTHORS file).
   License: Simplified BSD (see COPYING.BSD) */

extern "C" {
#include <mupdf/fitz.h>
#include <mupdf/pdf.h>
}

#include "utils/BaseUtil.h"
#include "utils/WinUtil.h"
#include "utils/ScopedWin.h"

#include "wingui/TreeModel.h"

#include "Annotation.h"
#include "EngineBase.h"
#include "EngineFzUtil.h"

// material icons https://material.io/resources/icons folder-open
static const char* gIconFileOpen =
    R"(<svg xmlns="http://www.w3.org/2000/svg" height="24" color="currentColor" viewBox="0 0 24 24" width="24"><path d="M0 0h24v24H0V0z" fill="none"/>
<path d="M22 6H12l-2-2H2v16h20V6zm-2 12H4V8h16v10z"/></svg>)";
    /*R"(<svg xmlns="http://www.w3.org/2000/svg" class="icon icon-tabler icon-tabler-folder" width="24" height="24" viewBox="0 0 24 24" stroke-width="2" stroke="currentColor" fill="none" stroke-linecap="round" stroke-linejoin="round">
  <rect x="0" y="0" width="24" height="24" stroke="none"></rect>
  <path d="M5 4h4l3 3h7a2 2 0 0 1 2 2v8a2 2 0 0 1 -2 2h-14a2 2 0 0 1 -2 -2v-11a2 2 0 0 1 2 -2" />
</svg>)";*/


// material icons https://material.io/resources/icons print
static const char* gIconPrint =
    R"(<svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 0 24 24" width="24"><path d="M0 0h24v24H0V0z" fill="none"/>
<path d="M22 8H2v9h4v4h12v-4h4V8zm-6 11H8v-5h8v5zm3-7c-.55 0-1-.45-1-1s.45-1 1-1 1 .45 1 1-.45 1-1 1zm-1-9H6v4h12V3z"/></svg>)";


// material icons https://material.io/resources/icons arrow-back
static const char* gIconPagePrev =
    R"(<svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 0 24 24" width="24"><path d="M0 0h24v24H0z" fill="none"/>
<path d="M20 11H7.83l5.59-5.59L12 4l-8 8 8 8 1.41-1.41L7.83 13H20v-2z"/></svg>)";

// material icons https://material.io/resources/icons arrow-forward
static const char* gIconPageNext =
    R"(<svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 0 24 24" width="24">
<path d="M0 0h24v24H0z" fill="none"/><path d="M12 4l-1.41 1.41L16.17 11H4v2h12.17l-5.58 5.59L12 20l8-8z"/></svg>)";

// https://materialdesignicons.com/  arrow-split-vertical
static const char* gIconLayoutContinous =
    R"(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" width="24" height="24" viewBox="0 0 24 24">
<path d="M18,16V13H15V22H13V2H15V11H18V8L22,12L18,16M2,12L6,16V13H9V22H11V2H9V11H6V8L2,12Z" /></svg>)";

// https://materialdesignicons.com/  arrow-split-horizontal
static const char* gIconLayoutSinglePage =
    R"(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" width="24" height="24" viewBox="0 0 24 24">
<path d="M8,18H11V15H2V13H22V15H13V18H16L12,22L8,18M12,2L8,6H11V9H2V11H22V9H13V6H16L12,2Z" /></svg>)";

//   material icons https://material.io/resources/icons chevron-left
static const char* gIconSearchPrev =
    R"(<svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 0 24 24" width="24"><path d="M0 0h24v24H0z" fill="none"/>
<path d="M15.41 7.41L14 6l-6 6 6 6 1.41-1.41L10.83 12z"/></svg>)";

// material icons https://material.io/resources/icons chevron-right
static const char* gIconSearchNext =
    R"(<svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 0 24 24" width="24"><path d="M0 0h24v24H0z" fill="none"/><path d="M10 6L8.59 7.41 13.17 12l-4.58 4.59L10 18l6-6z"/></svg>)";

//  https://materialdesignicons.com/  format-letter-case
static const char* gIconMatchCase =
    R"(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" width="24" height="24" viewBox="0 0 24 24">
<path d="M20.06,18C20,17.83 19.91,17.54 19.86,17.11C19.19,17.81 18.38,18.16 17.45,18.16C16.62,18.16 15.93,17.92 15.4,17.45C14.87,17 14.6,16.39 14.6,15.66C14.6,14.78 14.93,14.1 15.6,13.61C16.27,13.12 17.21,12.88 18.43,12.88H19.83V12.24C19.83,11.75 19.68,11.36 19.38,11.07C19.08,10.78 18.63,10.64 18.05,10.64C17.53,10.64 17.1,10.76 16.75,11C16.4,11.25 16.23,11.54 16.23,11.89H14.77C14.77,11.46 14.92,11.05 15.22,10.65C15.5,10.25 15.93,9.94 16.44,9.71C16.95,9.5 17.5,9.36 18.13,9.36C19.11,9.36 19.87,9.6 20.42,10.09C20.97,10.58 21.26,11.25 21.28,12.11V16C21.28,16.8 21.38,17.42 21.58,17.88V18H20.06M17.66,16.88C18.11,16.88 18.54,16.77 18.95,16.56C19.35,16.35 19.65,16.07 19.83,15.73V14.16H18.7C16.93,14.16 16.04,14.63 16.04,15.57C16.04,16 16.19,16.3 16.5,16.53C16.8,16.76 17.18,16.88 17.66,16.88M5.46,13.71H9.53L7.5,8.29L5.46,13.71M6.64,6H8.36L13.07,18H11.14L10.17,15.43H4.82L3.86,18H1.93L6.64,6Z" /></svg>)";

// material icons https://material.io/resources/icons zoom-in
static const char* gIconZoomIn =
    R"(<svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 0 24 24" width="24">
<path d="M0 0h24v24H0V0z" fill="none"/>
<path d="M15.5 14h-.79l-.28-.27C15.41 12.59 16 11.11 16 9.5 16 5.91 13.09 3 9.5 3S3 5.91 3 9.5 5.91 16 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14z"/><path d="M12 10h-2v2H9v-2H7V9h2V7h1v2h2v1z"/></svg>)";

// material icons https://material.io/resources/icons zoom-out
static const char* gIconZoomOut =
    R"(<svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 0 24 24" width="24"><path d="M0 0h24v24H0V0z" fill="none"/><path d="M15.5 14h-.79l-.28-.27C15.41 12.59 16 11.11 16 9.5 16 5.91 13.09 3 9.5 3S3 5.91 3 9.5 5.91 16 9.5 16c1.61 0 3.09-.59 4.23-1.57l.27.28v.79l5 4.99L20.49 19l-4.99-5zm-6 0C7.01 14 5 11.99 5 9.5S7.01 5 9.5 5 14 7.01 14 9.5 11.99 14 9.5 14zM7 9h5v1H7z"/></svg>)";

// material icons https://material.io/resources/icons save
static const char* gIconSave =
    R"(<svg xmlns="http://www.w3.org/2000/svg" height="24" viewBox="0 0 24 24" width="24"><path d="M0 0h24v24H0V0z" fill="none"/><path d="M17 3H3v18h18V7l-4-4zm-5 16c-1.66 0-3-1.34-3-3s1.34-3 3-3 3 1.34 3 3-1.34 3-3 3zm3-10H5V5h10v4z"/></svg>)";

// clang-format off
static const char* gAllIcons[] = {
    gIconFileOpen,
    gIconPrint,
    gIconPagePrev,
    gIconPageNext,
    gIconLayoutContinous,
    gIconLayoutSinglePage,
    gIconZoomOut,
    gIconZoomIn,
    gIconSearchPrev,
    gIconSearchNext,
    gIconMatchCase,
    gIconMatchCase,  // TODO: remove this, is for compatiblity with bitmap icons
    gIconSave,
};
// clang-format on

struct MupdfContext {
    fz_locks_context fz_locks_ctx{};
    CRITICAL_SECTION mutexes[FZ_LOCK_MAX];
    fz_context* ctx = nullptr;
    MupdfContext();
    ~MupdfContext();
};

static void fz_lock_context_cs(void* user, int lock) {
    MupdfContext* ctx = (MupdfContext*)user;
    EnterCriticalSection(&ctx->mutexes[lock]);
}

static void fz_unlock_context_cs(void* user, int lock) {
    MupdfContext* ctx = (MupdfContext*)user;
    LeaveCriticalSection(&ctx->mutexes[lock]);
}

MupdfContext::MupdfContext() {
    for (int i = 0; i < FZ_LOCK_MAX; i++) {
        InitializeCriticalSection(&mutexes[i]);
    }
    fz_locks_ctx.user = this;
    fz_locks_ctx.lock = fz_lock_context_cs;
    fz_locks_ctx.unlock = fz_unlock_context_cs;
    ctx = fz_new_context(nullptr, &fz_locks_ctx, FZ_STORE_DEFAULT);
}

MupdfContext::~MupdfContext() {
    fz_drop_context(ctx);
    for (int i = 0; i < FZ_LOCK_MAX; i++) {
        DeleteCriticalSection(&mutexes[i]);
    }
}

void BlitPixmap(fz_pixmap* dst, fz_pixmap* src, int dstX, int dstY) {
    int dx = src->w;
    int dy = src->h;
    int srcN = src->n;
    int dstN = dst->n;
    auto srcStride = src->stride;
    auto dstStride = dst->stride;
    for (size_t y = 0; y < (size_t)dy; y++) {
        u8* s = src->samples + (srcStride * (size_t)y);
        size_t atY = y + (size_t)dstY;
        u8* d = dst->samples + (dstStride * atY) + ((size_t)dstX * dstN);
        for (int x = 0; x < dx; x++) {
            d[0] = s[0];
            d[1] = s[1];
            d[2] = s[2];
            d += dstN;
            s += srcN;
        }
    }
}

void ClearPixmap(fz_pixmap* pixmap) {
    auto stride = pixmap->stride;
    size_t dx = (size_t)pixmap->w;
    size_t dy = (size_t)pixmap->h;
    u8* samples = pixmap->samples;
    CrashIf(pixmap->n != 3);
    for (size_t y = 0; y < dy; y++) {
        u8* d = samples + (stride * y);
        for (size_t x = 0; x < dx; x++) {
            d[0] = 255;
            d[1] = 0;
            d[2] = 0;
            d += pixmap->n;
            if (false) {
                if (x % 2 == 0) {
                    *d++ = 255;
                    *d++ = 0;
                    *d++ = 0;
                } else {
                    *d++ = 0;
                    *d++ = 0;
                    *d++ = 255;
                }
            }
        }
    }
}

fz_pixmap* BuildIconsPixmap(MupdfContext* muctx, int dx, int dy) {
    fz_context* ctx = muctx->ctx;
    int nIcons = (int)dimof(gAllIcons);
    int bmpDx = dx * nIcons;
    int bmpDy = dy;
    fz_pixmap* dstPixmap = fz_new_pixmap(ctx, fz_device_rgb(ctx), bmpDx, bmpDy, nullptr, 0);
    for (int i = 0; i < nIcons; i++) {
        const char* svgData = gAllIcons[i];
        fz_buffer* buf = fz_new_buffer_from_copied_data(ctx, (u8*)svgData, str::Len(svgData));
        fz_image* image = fz_new_image_from_svg(ctx, buf, nullptr, nullptr);
        image->w = dx;
        image->h = dy;
        fz_pixmap* pixmap = fz_get_pixmap_from_image(ctx, image, nullptr, nullptr, nullptr, nullptr);

        BlitPixmap(dstPixmap, pixmap, dx * i, 0);

        fz_drop_pixmap(ctx, pixmap);
        fz_drop_image(ctx, image);
        fz_drop_buffer(ctx, buf);
    }
    return dstPixmap;
}

HBITMAP CreateBitmapFromPixmap(fz_pixmap* pixmap) {
    int w = pixmap->w;
    int h = pixmap->h;
    int n = pixmap->n;
    int imgSize = pixmap->stride * h;
    int bitsCount = n * 8;

    ScopedMem<BITMAPINFO> bmi((BITMAPINFO*)calloc(1, sizeof(BITMAPINFO) + 255 * sizeof(RGBQUAD)));
    BITMAPINFOHEADER* bmih = &bmi.Get()->bmiHeader;
    bmih->biSize = sizeof(*bmih);
    bmih->biWidth = w;
    bmih->biHeight = -h;
    bmih->biPlanes = 1;
    bmih->biCompression = BI_RGB;
    bmih->biBitCount = bitsCount;
    bmih->biSizeImage = imgSize;
    bmih->biClrUsed = 0;
    void* data = nullptr;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD fl = PAGE_READWRITE;
    HANDLE hMap = CreateFileMappingW(hFile, nullptr, fl, 0, imgSize, nullptr);
    uint usage = DIB_RGB_COLORS;
    HBITMAP hbmp = CreateDIBSection(nullptr, bmi, usage, &data, hMap, 0);
    if (data) {
        u8* samples = pixmap->samples;
        memcpy(data, samples, imgSize);
    }
    return hbmp;
}

HBITMAP BuildIconsBitmap(int dx, int dy) {
    MupdfContext* muctx = new MupdfContext();
    fz_pixmap* pixmap = BuildIconsPixmap(muctx, dx, dy);
    // ClearPixmap(pixmap);
#if 1
#if 0
    RenderedBitmap* rbmp = new_rendered_fz_pixmap(muctx->ctx, pixmap);
    HBITMAP bmp = rbmp->hbmp;
#else
    HBITMAP bmp = CreateBitmapFromPixmap(pixmap);
#endif
    fz_drop_pixmap(muctx->ctx, pixmap);
    delete muctx;
    return bmp;
#else
    int nIcons = dimof(gAllIcons);
    int bmpDx = dx * nIcons;
    int bmpDy = dy;
    u8* bits = pixmap->samples;
    HBITMAP res = CreateBitmap(bmpDx, bmpDy, 1, 24, bits);
    fz_drop_pixmap(muctx->ctx, pixmap);
    delete muctx;

    return res;
#endif
}
