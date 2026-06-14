/*
 * gfx_glcapture.cpp — generic glReadPixels -> PNG capture for any backend whose draws
 * land in the current GL framebuffer (used by the SDL2 hardware-GL window-manager, which
 * has no capture of its own). Reads the front buffer (post-present) of the hidden window.
 */
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "glad/glad.h"

static uint32_t crc32_buf(const uint8_t *p, size_t n, uint32_t crc) {
    static uint32_t tab[256]; static int init = 0;
    if (!init) { for (uint32_t i=0;i<256;i++){ uint32_t c=i; for(int k=0;k<8;k++) c=(c&1)?0xEDB88320u^(c>>1):c>>1; tab[i]=c; } init=1; }
    crc ^= 0xFFFFFFFFu;
    for (size_t i=0;i<n;i++) crc = tab[(crc^p[i])&0xFF]^(crc>>8);
    return crc ^ 0xFFFFFFFFu;
}
static void put_be32(FILE *f, uint32_t v){ uint8_t b[4]={(uint8_t)(v>>24),(uint8_t)(v>>16),(uint8_t)(v>>8),(uint8_t)v}; fwrite(b,1,4,f); }
static void png_chunk(FILE *f, const char *type, const uint8_t *data, size_t len){
    put_be32(f,(uint32_t)len); fwrite(type,1,4,f); if(len) fwrite(data,1,len,f);
    uint8_t *tmp=(uint8_t*)malloc(4+len); memcpy(tmp,type,4); if(len) memcpy(tmp+4,data,len);
    uint32_t crc=crc32_buf(tmp,4+len,0); free(tmp); put_be32(f,crc);
}

extern "C" int gfx_glreadpixels_png(const char *path, int w, int h) {
    if (w <= 0 || h <= 0) return -1;
    uint8_t *buf = (uint8_t *)malloc((size_t)w * h * 4);
    if (!buf) return -1;
    glReadBuffer(GL_FRONT);                  /* hidden window, post-SwapWindow */
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buf);   /* rows bottom-up */

    FILE *f = fopen(path, "wb"); if (!f) { free(buf); return -1; }
    static const uint8_t sig[8]={0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A}; fwrite(sig,1,8,f);
    uint8_t ihdr[13];
    ihdr[0]=(uint8_t)(w>>24);ihdr[1]=(uint8_t)(w>>16);ihdr[2]=(uint8_t)(w>>8);ihdr[3]=(uint8_t)w;
    ihdr[4]=(uint8_t)(h>>24);ihdr[5]=(uint8_t)(h>>16);ihdr[6]=(uint8_t)(h>>8);ihdr[7]=(uint8_t)h;
    ihdr[8]=8; ihdr[9]=6; ihdr[10]=ihdr[11]=ihdr[12]=0; png_chunk(f,"IHDR",ihdr,13);

    size_t raw_len=(size_t)h*(1+(size_t)w*4);
    uint8_t *raw=(uint8_t*)malloc(raw_len); size_t o=0;
    for (int y=0;y<h;y++){ const uint8_t *src=buf+(size_t)(h-1-y)*w*4; raw[o++]=0; memcpy(raw+o,src,(size_t)w*4); o+=(size_t)w*4; }

    size_t max_z=2+raw_len+(raw_len/65535+1)*5+4; uint8_t *z=(uint8_t*)malloc(max_z); size_t zo=0;
    z[zo++]=0x78; z[zo++]=0x01; size_t pos=0;
    while (pos<raw_len){ size_t blk=raw_len-pos; if(blk>65535)blk=65535; int fin=(pos+blk>=raw_len)?1:0;
        z[zo++]=(uint8_t)fin; z[zo++]=(uint8_t)(blk&0xFF); z[zo++]=(uint8_t)(blk>>8);
        uint16_t nlen=(uint16_t)~blk; z[zo++]=(uint8_t)(nlen&0xFF); z[zo++]=(uint8_t)(nlen>>8);
        memcpy(z+zo,raw+pos,blk); zo+=blk; pos+=blk; }
    uint32_t a=1,b=0; for(size_t i=0;i<raw_len;i++){ a=(a+raw[i])%65521; b=(b+a)%65521; }
    uint32_t adler=(b<<16)|a; z[zo++]=(uint8_t)(adler>>24);z[zo++]=(uint8_t)(adler>>16);z[zo++]=(uint8_t)(adler>>8);z[zo++]=(uint8_t)adler;
    png_chunk(f,"IDAT",z,zo); png_chunk(f,"IEND",nullptr,0);
    free(z); free(raw); free(buf); fclose(f);
    fprintf(stderr, "gfx_glcapture: wrote %s (%dx%d)\n", path, w, h);
    return 0;
}
