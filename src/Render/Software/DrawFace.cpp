/*
	DrawFace.cpp
*/

#include	<Math.h>

#include	"Const.h"
//#include	"MapFile.h"
#include	"Math_3D.h"
#include	"DrawFace.h"
#include	"DrawSpan.h"
#include	"Portal.h"
#include	"Render.h"
#include	"TexGrad.h"
#include	"Surface.h"
#include	"SpanBuf.h"
#include	"Video.h"

extern TMap	map;

static point_3d	pts[64];
static point_3d	*default_vlist[64];

static point_3d	new_points[256];
static point_3d	*clip_list1[64];
static point_3d	*clip_list2[64];

int		scr_face_edge[MAX_SY_SIZE][2];
void	CrossPoint(point_3d *out, point_3d *a, point_3d *b)
{
	out->p[0] = float(a->p[0] + (b->p[0] - a->p[0]) * where);
	out->p[1] = float(a->p[1] + (b->p[1] - a->p[1]) * where);
	out->p[2] = float(a->p[2] + (b->p[2] - a->p[2]) * where);
	if(clip_mode)
	{
		out->u = float(a->u + (b->u - a->u) * where);
		out->v = float(a->v + (b->v - a->v) * where);
		out->r = float(a->r + (b->r - a->r) * where);
		out->g = float(a->g + (b->g - a->g) * where);
		out->b = float(a->b + (b->b - a->b) * where);
	}
	ProjectPointToScreen(out);
	CodePoint(out);
}

int		ClipPoly(int n, point_3d **vl, int codes_or, point_3d ***out_vl)
{
	if(!codes_or)	goto	NotClip;

	int		i, j, k, p;
	p=0;
	point_3d	**cur, *a, *b;

	if(codes_or & CC_OFF_LEFT)
	{
		cur = clip_list1;
		for (k=i=0, j=n-1; i<n; j=i++)
		{
			a = vl[i];
			b = vl[j];
			if(!(b->ccodes & CC_OFF_LEFT)) cur[k++] = b;
			if ((b->ccodes ^ a->ccodes) & CC_OFF_LEFT)
			{
				where = (a->p[2]*left_clip - a->p[0])/(b->p[0] - a->p[0] + (a->p[2] - b->p[2])*left_clip);
				CrossPoint(&new_points[p], a, b);
				cur[k++] = &new_points[p++];
			}
		}
		n = k;
		vl = cur;
	}
	if (codes_or & CC_OFF_RIGHT) 
	{
		cur = (vl == clip_list1) ? clip_list2 : clip_list1;
		for (k=i=0, j=n-1; i<n; j=i++)
		{
			a = vl[i];
			b = vl[j];
			if (!(b->ccodes & CC_OFF_RIGHT)) cur[k++] = b;
			if ((b->ccodes ^ a->ccodes) & CC_OFF_RIGHT)
			{
				where = (a->p[2]*right_clip - a->p[0])/(b->p[0] - a->p[0] + (a->p[2] - b->p[2])*right_clip);
				CrossPoint(&new_points[p], a, b);
				cur[k++] = &new_points[p++];
			}
		}
		n = k;
		vl = cur;
	}
	if (codes_or & CC_OFF_TOP) 
	{
		cur = (vl == clip_list1) ? clip_list2 : clip_list1;
		for (k=i=0, j=n-1; i<n; j=i++)
		{
			a = vl[i];
			b = vl[j];
			if(!(b->ccodes & CC_OFF_TOP)) cur[k++] = b;
			if((b->ccodes ^ a->ccodes) & CC_OFF_TOP)
			{
				where = (a->p[2]*top_clip - a->p[1])/(b->p[1] - a->p[1] + (a->p[2] - b->p[2])*top_clip);
				CrossPoint(&new_points[p], a, b);
				cur[k++] = &new_points[p++];
			}
		}
		n = k;
		vl = cur;
	}
	if(codes_or & CC_OFF_BOT) 
	{
		cur = (vl == clip_list1) ? clip_list2 : clip_list1;
		for (k=i=0, j=n-1; i<n; j=i++)
		{
			a = vl[i];
			b = vl[j];
			if(!(b->ccodes & CC_OFF_BOT)) cur[k++] = b;
			if((b->ccodes ^ a->ccodes) & CC_OFF_BOT)
			{
				where = (a->p[2]*bottom_clip - a->p[1])/(b->p[1] - a->p[1] + (a->p[2] - b->p[2])*bottom_clip);
				CrossPoint(&new_points[p], a, b);
				cur[k++] = &new_points[p++];
			}
		}
		n = k;
		vl = cur;
	}
	for(i=0; i<n; i++) if(vl[i]->ccodes & CC_BEHIND) return 0;
NotClip:
	*out_vl = vl;
	if(mirror || portal)
		n = ClipPortalPoly(n, vl, out_vl);
	return	n;
}
static int	invert_poly=0;
void	ComputeEdge(point_3d *a, point_3d *b)
{
	point_3d	*temp;
	int	right;

	if (a->sy == b->sy) return;

	if (a->sy < b->sy)
		right = invert_poly;
	else
	{
		temp = a;
		a = b;
		b = temp;
		right = !invert_poly;
	}
	// compute dxdy
    int sy		= fix_to_int(a->sy);
	int sy_end	= fix_to_int(b->sy);
	int dsx = float_to_int(65536.0 * (b->sx - a->sx) / (b->sy - a->sy));
	int sx = a->sx;
	// fixup x location to 'y' (subpixel correction in y)
	sx += float_to_int(((double) dsx * ((sy << 16) - a->sy)) / 65536.0);

	if(mirror)
	{
		right ^= 1;
		sx = (sx_size<<16)-65535-sx;
		dsx = -dsx;
	}
	for(; sy<sy_end; sy++, sx += dsx)
		scr_face_edge[sy][right] = sx;
}
extern float	z_gradient[3];
void	DrawPolygon(int n, point_3d **vl)
{
	int		i, j, ymin, ymax;
	ymin = ymax = vl[0]->sy;
	for (i=1; i<n; i++)
	{
		if (vl[i]->sy < ymin) ymin = vl[i]->sy;
		else if (vl[i]->sy > ymax) ymax = vl[i]->sy;
	}

	j = n-1;
	for (i=0; i<n; i++)
	{
		ComputeEdge(vl[i], vl[j]);
		j = i;
	}

	int sy_end = fix_to_int(ymax);
    for(int sy=fix_to_int(ymin); sy<sy_end; sy++)
	{
		int		sx_start = fix_to_int(scr_face_edge[sy][0]);
		int		sx_end = fix_to_int(scr_face_edge[sy][1]);
		if(sx_start>=sx_end)	continue;
		if(!face_type)
		{
			TSpan	cur_span;
			cur_span.sx_start = sx_start;
			cur_span.sx_end = sx_end;
			cur_span.sx0 = sx_start<<16;
			cur_span.dz = z_gradient[0]+z_gradient[1]*sx_start+z_gradient[2]*sy;
			cur_span.ddz = z_gradient[1]/65536.0f;
			AddSpan(sy, &cur_span);
			continue;
		}
		spans_start[0] = sx_start;
		spans_end[0] = sx_end;
		num_draw_spans = 1;

		int		numspans = end1_spans[sy];
		if(numspans>=MAX_NUM_SPANS)	continue;
		for(int spannum=end0_spans[sy]; spannum<numspans; spannum++)
		{
			int	test_start = span_buf[sy].spans[spannum].sx_start;
			int	test_end = span_buf[sy].spans[spannum].sx_end;

			if((sx_start>=test_end) || (sx_end<=test_start))	continue;

			int		num_spans_in_span = num_draw_spans;

			for(i=0, j=0; i<num_spans_in_span; i++)
			{
				int		span_start = spans_start[i];
				int		span_end = spans_end[i];
				if(span_start>=span_end)	continue;
				j++;
				if((span_start>=test_end) || (span_end<=test_start))	continue;
				if(span_start<test_start)
				{
					spans_end[i] = test_start;
					if(span_end>test_end)
					{
						if(num_draw_spans<64)
						{
							spans_start[num_draw_spans] = test_end;
							spans_end[num_draw_spans] = span_end;
							num_draw_spans++;
						}
					}
				} else
					spans_start[i] = test_end;
			}
			//if(j==0)	break;
		}
		//if(j==0)	continue;
		TSpan	cur_span;
		cur_span.sx_start = sx_start;
		cur_span.sx_end = sx_end;
		cur_span.sx0 = sx_start<<16;
		cur_span.dz = z_gradient[0]+z_gradient[1]*sx_start+z_gradient[2]*sy;
		cur_span.ddz = z_gradient[1]/65536.0f;
		if(!ClipSpan(sy, &cur_span))	continue;

		for(i=0, j=0; i<num_draw_spans; i++)
		{
			int		start = spans_start[i];
			int		end = spans_end[i];
			if(start>=end)	continue;
			DrawSpan(sy, start, end);
			//*((short *)virtual_screen + screen_row_table[sy] + start) = 31;
			j++;
		}
		if(j && !(mode&2))	AddSpan(sy, &cur_span);
	}
}
void	DrawTransparentPolygon(int n, point_3d **vl)
{
	int		ymin, ymax;
	ymin = ymax = vl[0]->sy;
	for(int i=1; i<n; i++)
	{
		if (vl[i]->sy < ymin) ymin = vl[i]->sy;
		else if (vl[i]->sy > ymax) ymax = vl[i]->sy;
	}

	int j = n-1;
	for(i=0; i<n; i++)
	{
		ComputeEdge(vl[i], vl[j]);
		j = i;
	}

	int sy_end = fix_to_int(ymax);
    for(int sy=fix_to_int(ymin); sy<sy_end; sy++)
	{
		TSpan	cur_span;
		cur_span.sx_start = fix_to_int(scr_face_edge[sy][0]);
		cur_span.sx_end = fix_to_int(scr_face_edge[sy][1]);
		if(cur_span.sx_start>=cur_span.sx_end)	continue;
		cur_span.sx0 = cur_span.sx_start<<16;
		cur_span.dz = z_gradient[0]+z_gradient[1]*cur_span.sx_start+z_gradient[2]*sy;
		cur_span.ddz = z_gradient[1]/65536.0f;
		if(!face_type)
		{
			AddSpan(sy, &cur_span);
			continue;
		}

		num_draw_spans = 0;
		if(!ClipSpan(sy, &cur_span))	continue;

		for(int	i=0, j=0; i<num_draw_spans; i++)
		{
			int		start = spans_start[i];
			int		end = spans_end[i];
			if(start>=end)	continue;
			DrawSpan(sy, start, end);
			//*((short *)virtual_screen + screen_row_table[sy] + start) = -1;
			j++;
		}
		if(j && !(mode&2))	AddSpan(sy, &cur_span);
	}
}
extern int	faces_p_s;
static vec3_t	lpoints[128], upoints[128], bpoints[128];
static int		nr, nl, nu, nb;
static float	cx, cy;
void	DrawWaterFace(int face)
{
	float		delta = float(time*pi*0.001);
	int			codes_or=0, codes_and=0xff;
	point_3d	**vlist;
	for (int i=0; i<nu; i++)
	{
		upoints[i][2] += float(4*sin(pi*upoints[i][0]/128 + delta)) + float(4*sin(pi*upoints[i][1]/128 + delta));
		TransformPoint(&pts[i], upoints[i]);
		codes_or  |= pts[i].ccodes;
		codes_and &= pts[i].ccodes;
	}
	if(codes_and) return;
	clip_mode = 0;
	t_plane	plane;
	vec3_t	a, b;
	if(nu>3)
	{
		VectorSubtract(upoints[3], upoints[0], a);
		VectorSubtract(upoints[1], upoints[0], b);
	} else
	{
		VectorSubtract(upoints[2], upoints[0], a);
		VectorSubtract(upoints[1], upoints[0], b);
	}
	CrossProduct(a, b, plane.normal);
	plane.dist = DotProduct(plane.normal, upoints[0]);
	plane.type = 3;
	nu = ClipPoly(nu, default_vlist, codes_or, &vlist);
	if(!nu)	return;

	int		t_info = map.faces[face].texinfo;
	int		texnum = map.texinfo[t_info].texnum;
	float	u,v;
	texmap	tex;

	ComputeFaceMipLevel(nu, vlist);
	GetTextureMap(&tex, face, texnum, &u, &v);
	ComputeDynamicTextureGradients(face, t_info, u, v, &plane);
	ComputeZGradients(&plane);
	SetTexture((short *)tex.bitmap, tex.width, tex.height);
	SetSurfaceType(face_type);
	if(DistToCamera(&plane)<0)
	{
		invert_poly = 1;
		DrawTransparentPolygon(nu, vlist);
		invert_poly = 0;
	} else
		DrawTransparentPolygon(nu, vlist);
	faces_p_s++;
}
void	DivYDrawFace(int face)
{
	float	miny, maxy, cy0, cy1;
	miny = maxy = lpoints[0][1];
	for(int	i=1; i<nl; i++)
	{
		if(miny>lpoints[i][1])	miny = lpoints[i][1];
		if(maxy<lpoints[i][1])	maxy = lpoints[i][1];
	}
	cy0 = ((int(miny)>>5)+1)*32.0f;
	cy1 = (int(maxy)>>5)*32.0f;
	if(cy1!=maxy)	cy1 += 32;
	int		ky = int(miny)>>5;
	for(cy=cy0; cy<=cy1; cy+=32, ky++, nl=nb)
	{
		int		j=nl-1;
		for(nu=nb=i=0; i<nl; i++)
		{
			if(((lpoints[j][1]>cy) && (lpoints[i][1]<cy)) || ((lpoints[j][1]<cy) && (lpoints[i][1]>cy)))
			{
				vec3_t	cross;
				float	where = (cy-lpoints[j][1])/(lpoints[i][1]-lpoints[j][1]);
				cross[0] = lpoints[j][0] + (lpoints[i][0]-lpoints[j][0])*where;
				cross[1] = lpoints[j][1] + (lpoints[i][1]-lpoints[j][1])*where;
				cross[2] = lpoints[j][2] + (lpoints[i][2]-lpoints[j][2])*where;
				VectorCopy(cross, upoints[nu++]);
				VectorCopy(cross, bpoints[nb++]);
			}
			if(lpoints[i][1]<=cy)
				VectorCopy(lpoints[i], upoints[nu++]);
			if(lpoints[i][1]>=cy)
				VectorCopy(lpoints[i], bpoints[nb++]);
			j=i;
		}
		for(i=0; i<nb; i++)	VectorCopy(bpoints[i], lpoints[i]);
		DrawWaterFace(face);
	}
}
static vec3_t	points[512], rpoints[128];
void	DivXDrawFace(int face)
{
	int		firstedge = map.faces[face].firstedge;
	int		n = map.faces[face].numedges;
	float	minx, maxx, cx0, cx1;
	int		i, j;
	for (i=0; i<n; i++)
	{
		int edge = map.surf_edges[firstedge+i];
		if (edge < 0)	VectorCopy(map.vertices[map.edges[-edge].v[1]].point, points[i]);
		else			VectorCopy(map.vertices[map.edges[ edge].v[0]].point, points[i]);
	}
	minx = maxx = points[0][0];
	for(i=1; i<n; i++)
	{
		if(minx>points[i][0])	minx = points[i][0];
		if(maxx<points[i][0])	maxx = points[i][0];
	}
	cx0 = ((int(minx)>>5)+1)*32.0f;
	cx1 = (int(maxx)>>5)*32.0f;
	if(cx1!=maxx)	cx1 += 32;
	for(cx=cx0; cx<=cx1; cx+=32, n=nr)
	{
		j=n-1;
		for(nr=nl=i=0; i<n; i++)
		{
			if(((points[j][0]>cx) && (points[i][0]<cx)) || ((points[j][0]<cx) && (points[i][0]>cx)))
			{
				vec3_t	cross;
				float	where = (cx-points[j][0])/(points[i][0]-points[j][0]);
				cross[0] = points[j][0] + (points[i][0]-points[j][0])*where;
				cross[1] = points[j][1] + (points[i][1]-points[j][1])*where;
				cross[2] = points[j][2] + (points[i][2]-points[j][2])*where;
				VectorCopy(cross, lpoints[nl++]);
				VectorCopy(cross, rpoints[nr++]);
			}
			if(points[i][0]<=cx)
				VectorCopy(points[i], lpoints[nl++]);
			if(points[i][0]>=cx)
				VectorCopy(points[i], rpoints[nr++]);
			j=i;
		}
		for(i=0; i<nr; i++)	VectorCopy(rpoints[i], points[i]);
		DivYDrawFace(face);
	}
}
void	DrawFace(int face)
{
	//if(faces_p_s>=1024)	return;
	if((face_type==2) && !(mode&2))
	{//	water
		trans_faces[num_trans_faces++]=face;
		return;
	}
	int		firstedge = map.faces[face].firstedge;
	int		n = map.faces[face].numedges;
	int		codes_or=0, codes_and=0xff;
	point_3d	**vlist;
	if(face_type!=2)
	{
		for (int i=0; i<n; i++)
		{
			int edge = map.surf_edges[firstedge+i];
			if (edge < 0)	TransformPoint(&pts[i], map.vertices[map.edges[-edge].v[1]].point);
			else			TransformPoint(&pts[i], map.vertices[map.edges[ edge].v[0]].point);
			codes_or  |= pts[i].ccodes;
			codes_and &= pts[i].ccodes;
		}
	} else
	{
		DivXDrawFace(face);
		return;
	}
	if(codes_and) return;	//	if polygon is off screen, then exit
	clip_mode = 0;

	n = ClipPoly(n, default_vlist, codes_or, &vlist);
	if(!n)	return;

	int		t_info = map.faces[face].texinfo;
	int		texnum = map.texinfo[t_info].texnum;
	float	u,v;
	texmap	tex;

	if(face_type)
	{
		ComputeFaceMipLevel(n, vlist);
		GetTextureMap(&tex, face, texnum, &u, &v);
		ComputeTextureGradients(face, t_info, u, v);
		SetTexture((short *)tex.bitmap, tex.width, tex.height);
		SetSurfaceType(face_type);
		faces_p_s++;
	}
	ComputeZGradients(&map.planes[map.faces[face].planenum]);
	if(mode)	DrawTransparentPolygon(n, vlist);
	else		DrawPolygon(n, vlist);
}

void	InitDefaultPointList()
{
	for(int i=0; i<64; i++) default_vlist[i] = &pts[i];
}

void	DrawMarkOnWall(wmark_t *mark)
{
	point_3d	**vlist;
	int		n=4, codes_or=0, codes_and=0xff;
	for (int i=0; i<n; i++)
	{
		TransformPoint(&pts[i], mark->p[i]);
		codes_or  |= pts[i].ccodes;
		codes_and &= pts[i].ccodes;
	}
	if(codes_and) return;

	clip_mode = 0;
	n = ClipPoly(n, default_vlist, codes_or, &vlist);
	if(!n)	return;

	ComputeMarkTextureGradients(mark);
	SetTexture((short *)mark->bm->bm, mark->bm->width, mark->bm->height);
	SetSurfaceType(5);
	ComputeZGradients(&map.planes[map.nodes[mark->node].planenum]);
	int	m = mode;
	mode = 2;
	DrawPolygon(n, vlist);
	mode = m;
}