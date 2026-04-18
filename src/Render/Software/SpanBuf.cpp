/*
	SpanBuf.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Stdio.h>
#include	<Memory.h>
#include	<Stdlib.h>

#include	"Const.h"
#include	"SpanBuf.h"
#include	"Video.h"

int		num_draw_spans;
int		spans_start[MAX_NUM_SPANS], spans_end[MAX_NUM_SPANS];
int		temp_end_spans[MAX_SY_SIZE], end0_spans[MAX_SY_SIZE], end1_spans[MAX_SY_SIZE];
TSpanLine	*span_buf=NULL;

void	SetSpanBufSize(int height)
{
	span_buf = (TSpanLine *)realloc(span_buf, height*sizeof(TSpanLine));
}
void	ClearSpanBuf()
{//	очищаем span-буфер
	memset(end1_spans, 0, sizeof(int)*sy_size);
	memset(end0_spans, 0, sizeof(int)*sy_size);
}
void	SaveSpanBuf()
{
	memcpy(temp_end_spans, end1_spans, sizeof(int)*sy_size);
}
void	RestoreSpanBuf()
{
	memcpy(end1_spans, temp_end_spans, sizeof(int)*sy_size);
}
void	MoveEnd0End1()
{
	memcpy(end0_spans, end1_spans, sizeof(int)*sy_size);
}
void	AddSpan(short sy, TSpan *span)
{
	if(span->sx_start>=span->sx_end)	return;
	int		num = end1_spans[sy];
	if(num>=MAX_NUM_SPANS)	return;
	span_buf[sy].spans[num] = *span;
	end1_spans[sy]++;
}

#define	SpansCross	(((testspan->dz-cur_span->dz)+(cur_span->sx0*cur_span->ddz)-(testspan->sx0*testspan->ddz))/((cur_span->ddz-testspan->ddz)*65536.0))

void	NewSpan(int span_start, int span_end)
{
	if(num_draw_spans>=MAX_NUM_SPANS)	return;
	spans_start[num_draw_spans] = span_start;
	spans_end[num_draw_spans] = span_end;
	num_draw_spans++;
}

int		ClipSpan(int cur_sy, TSpan *cur_span)
{// проверка на перекрытие и пересечение спанов
	int	sx_start = cur_span->sx_start;
	int	sx_end = cur_span->sx_end;
	int	numspans;
	if(!num_draw_spans)
	{
		spans_start[0] = sx_start;
		spans_end[0] = sx_end;
		num_draw_spans = 1;
		numspans = end1_spans[cur_sy];
	} else
		numspans = end0_spans[cur_sy];
	if(numspans>MAX_NUM_SPANS)	return	0;
	TSpan	*testspan = &span_buf[cur_sy].spans[0];
	for(int spannum=0; spannum<numspans; spannum++, testspan++)
    {
		int		test_span_start = testspan->sx_start;
		int		test_span_end = testspan->sx_end;
		if((sx_start>=test_span_end) || (sx_end<=test_span_start))	continue;

		int		num_spans_in_span = num_draw_spans;
		for(int	i=0,j=0; i<num_spans_in_span; i++)
		{
			int		span_start = spans_start[i];
			int		span_end = spans_end[i];
			if(span_start>=span_end)	continue;
			j++;
			if((span_start>=test_span_end) || (span_end<=test_span_start))	continue;

			float	z1b = cur_span->dz + cur_span->ddz*((span_start<<16) - cur_span->sx0);
			float	z1e = cur_span->dz + cur_span->ddz*((span_end<<16) - cur_span->sx0);
			float	z2b = testspan->dz + testspan->ddz*((span_start<<16) - testspan->sx0);
			float	z2e = testspan->dz + testspan->ddz*((span_end<<16) - testspan->sx0);
			
			if(z1b>z2b)
			{
				if(z1e<z2e)
				{
					int		sx_cross = float_to_int(SpansCross);
					if((sx_cross>span_end) || (sx_cross<span_start))
						sx_cross = (span_start+span_end)>>1;
					//DrawString(8, 8, "Cross Bug!!!");

					if(test_span_end>sx_cross)
					{
						if(test_span_start>sx_cross)
							spans_end[i] = test_span_start;
						else
							spans_end[i] = sx_cross;
						if(test_span_end<span_end)
							NewSpan(test_span_end, span_end);
					}
				}
			} else
			{
				if(z1e>z2e)
				{
					int	sx_cross = float_to_int(SpansCross);
					if((sx_cross>span_end) || (sx_cross<span_start))
						sx_cross = (span_start+span_end)>>1;

					if(sx_cross>test_span_start)
					{
						if(test_span_end<sx_cross)
							spans_start[i] = test_span_end;
						else
							spans_start[i] = sx_cross;
						if(span_start<test_span_start)
							NewSpan(span_start, test_span_start);
					}
				} else
				{
					if(test_span_start<=span_start)
					{
						spans_start[i] = test_span_end;
					} else
					{
						spans_end[i] = test_span_start;
						if(test_span_end<span_end)
							NewSpan(test_span_end, span_end);
					}
				}
			}
		}
		if(!j)	return	0;
	}
	return	1;
}
int		ClipPixel(int sx, int sy, float z1)
{
	int		numspans = end1_spans[sy];
	int		sx_16 = sx;
	sx >>= 16;
	if(numspans>MAX_NUM_SPANS)	return	0;
	TSpan	*testspan = &span_buf[sy].spans[0];
	for(int spannum=0; spannum<numspans; spannum++, testspan++)
    {
		if((sx>testspan->sx_end) || (sx<testspan->sx_start))	continue;
		float	z2 = testspan->dz + testspan->ddz*(sx_16 - testspan->sx0);
		if(z2>z1)	return	0;
	}
	return	1;
}