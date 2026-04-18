#ifndef _DRAW_2D_H
#define _DRAW_2D_H

#include	"Struct.h"

void	SetFullClipRectangle();
void	SetClipRectangle(rect_t rect);
void	GetClipRectangle(rect_t *rect);
void	DrawSpline(int n, scrpoint_t *p, int resolution, color_t color, float k);
void	DrawCatmullRomSpline(int n, scrpoint_t *p, int resolution, color_t color);
void	DrawLine(int x1, int y1, int x2, int y2, color_t c);
void	DrawBitmap(int x,int y, bitmap *bm, byte alpha, byte type);
void	DrawString(int x, int y, color_t c, char *format, ...);
void	DrawBackground(int time);
void	SetFont(bitmap *bm);
void	SetBackground(bitmap *bm);
void	DrawBar(int x1, int y1, int x2, int y2, color_t c);
void	DrawRectangle(int x1, int y1, int x2, int y2, color_t c);

void	SetPixel(int x, int y, color_t color);
color_t	GetPixel(int x, int y);

#endif