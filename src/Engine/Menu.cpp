/*
	Menu.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<stdio.h>
#include	<malloc.h>

#include	"console.h"
#include	"menu.h"
#include	"move.h"
#include	"pcxfile.h"
#include	"main.h"
#include	"render.h"
#include	"unpak.h"
#include	"video.h"

extern int	time1;
extern char	enable_menu, cur_key_state;

int		CMenu::OpenMenu(char *name)
{
	ClearKeyBuf();
	CloseConsole();
	can_move = 0;

	push_item = 0;
	num_push_items = 0;
	menu_init = 0;
	bitmaps = NULL;
	menu_data = NULL;

	menupf = new CPakFile;
	if(!menupf->OpenPak(name))	return 0;
	int		size = menupf->ExtractByName("menu.dat", &menu_data);
	if (size<=0)	return	0;
	header = (menu_header *)menu_data;
	if(strcmp(header->id,"Типа меню v1.0b"))
	{
		CPrintf("\"%s\" is not menu file", name);
		CloseMenu();
		return	0;
	}
	items  = (menu_item *)(menu_data + header->items_ofs);
	num_items = header->items_len / sizeof(menu_item);
	pics  = (menu_pic *)(menu_data + header->pics_ofs);
	num_pics = header->pics_len / sizeof(menu_pic);

	bitmaps = (bitmap *) malloc(num_pics*sizeof(bitmap));
	for(int i=0; i<num_pics; i++)	bitmaps[i].bm = NULL;

	cur_item = items[push_item].first_item;

	if(!LoadCurrentItemPics())
	{
		CloseMenu();
		return	0;
	}
	menu_init = 1;
	enable_menu = 1;
	cur_key_state = 2;
	time = time1;
	return	1;
}
void	CMenu::CloseMenu()
{
	menu_init = 0;
	enable_menu = 0;

	FreePics();
	free(menu_data);	menu_data = NULL;
	free(bitmaps);		bitmaps = NULL;
	menupf->ClosePak();
	delete	menupf;

	cur_key_state = 2;
	can_move = 1;
}
void	CMenu::PushMenu(char *name)
{
	if(menu_init)
		CloseMenu();
	else
		OpenMenu(name);
}
int		CMenu::MenuStatus()
{
	return	menu_init;
}
int		CMenu::UpdateMenu()
{
	if(!menu_init)	return	0;
	int		first_item = items[push_item].first_item;
	int		end_item = first_item + items[push_item].num_items - 1;
	char	c;
	while(c = ReadKey())
	{
		if(c==22)
		{
			cur_item--;
			if(cur_item<first_item)	cur_item = end_item;
			time = time1;
		}
		if(c==24)
		{
			cur_item++;
			if(cur_item>end_item)	cur_item = first_item;
			time = time1;
		}
		if(c==13)
		{
			if(items[cur_item].num_items>0)
			{
				push_items[num_push_items++] = push_item;
				push_item = cur_item;
				if(!LoadCurrentItemPics())
				{
					CloseMenu();
					return 0;
				}
			} else
				return	items[cur_item].first_item;
		}
		/*if(c==27)
		{
			if(num_push_items)
			{
				cur_item = push_item;
				push_item = push_items[--num_push_items];
				if(!LoadCurrentItemPics())
				{
					CloseMenu();
					return 0;
				}
			} else
			{
				CloseMenu();
				return 0;
			}
		}*/
	}
	return	0;
}
void	CMenu::DrawMenu()
{
	if(!menu_init)	return;
	int		x1, y1, i, end_i, num;
	i = items[push_item].first_pic;
	end_i = i + items[push_item].num_pics;
	for(; i<end_i; i++)
	{
		x1 = int(sx_size*0.01*pics[i].x1);
		y1 = int(sy_size*0.01*pics[i].y1);
		mgl.DrawBitmap(x1, y1, &bitmaps[i], 255, 1);
	}
	i = items[push_item].first_item;
	end_i = i + items[push_item].num_items;
	for(; i<end_i; i++)
	{
		num = items[i].item_pic;
		x1 = int(sx_size*0.01*pics[num].x1);
		y1 = int(sy_size*0.01*pics[num].y1);
		if(i==cur_item)
		{
			int	alpha = 127 + 24*(time1-time)/125;
			if(alpha>255)	alpha = 255;
			mgl.DrawBitmap(x1, y1, &bitmaps[num], alpha, 1);
		}
		else
			mgl.DrawBitmap(x1, y1, &bitmaps[num], 127, 1);
	}
}

void	CMenu::FreePics()
{
	if(bitmaps==NULL)	return;
	for(int i=0; i<num_pics; i++)
	{
		if(bitmaps[i].bm!=NULL)
		{
			free(bitmaps[i].bm);
			bitmaps[i].bm = NULL;
		}
	}
}
int		CMenu::LoadPic(int num)
{
	if(bitmaps[num].bm!=NULL)	return	1;
	bitmap8	bm;
	if(LoadPCX8(&bm, pics[num].name, menupf)<0)	return	0;
	int	x1 = int(sx_size*0.01*pics[num].x1);
	int	x2 = int(sx_size*0.01*pics[num].x2);
	int	y1 = int(sy_size*0.01*pics[num].y1);
	int	y2 = int(sy_size*0.01*pics[num].y2);
	//bitmaps[num] = bm;
	ResizeToBitmap16(&bitmaps[num], &bm, x2-x1, y2-y1);
	free(bm.bm);
	return	1;
}

int		CMenu::LoadCurrentItemPics()
{
	FreePics();
	int		i = items[push_item].first_pic;
	int		end_i = i + items[push_item].num_pics;
	for(; i<end_i; i++)
		if(!LoadPic(i))	return	0;

	i = items[push_item].first_item;
	end_i = i + items[push_item].num_items;
	for(; i<end_i; i++)
		if(!LoadPic(items[i].item_pic))	return	0;
	return	1;
}