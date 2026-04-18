#ifndef _SPANBUF_H
#define _SPANBUF_H

#define MAX_NUM_SPANS 0x40

typedef struct
{
    short	sx_start, sx_end;
	int		sx0;
    float	dz, ddz;
} TSpan;

typedef struct
{
    TSpan	spans[MAX_NUM_SPANS];
} TSpanLine;

extern int	num_draw_spans;
extern int	spans_start[MAX_NUM_SPANS], spans_end[MAX_NUM_SPANS];
extern int	end0_spans[2048], end1_spans[2048];
extern TSpanLine	*span_buf;

void	SetSpanBufSize(int height);
void	ClearSpanBuf();
void	SaveSpanBuf();
void	RestoreSpanBuf();
void	MoveEnd0End1();
void	AddSpan(short sy, TSpan *span);
int		ClipSpan(int cur_sy, TSpan *span);
int		ClipPixel(int sx, int sy, float z1);

#endif