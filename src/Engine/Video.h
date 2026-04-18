#ifndef _VIDEO_H
#define _VIDEO_H

#define	DRAW_LIGHT		1
#define	DRAW_TEXTURED	2

extern int		sx_size, sy_size, bits, gamma;
extern int		frames, b_time, c_time, e_time;
extern bitmap	background, font;

void	DrawStatus();
int		LoadGraphics();

#endif