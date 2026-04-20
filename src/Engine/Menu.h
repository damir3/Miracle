#ifndef _MENU_H
#define _MENU_H

#include	"struct.h"
#include	"unpak.h"

typedef struct
{
	char	id[16];	// = "Like menu"
    int		items_ofs, items_len;
	int		pics_ofs, pics_len;
} menu_header;
typedef struct
{
    short	first_item, num_items;
    short	first_pic, num_pics;
    short	item_pic;
} menu_item;
typedef struct
{
	char	type[4];
    char	x1, y1, x2, y2;
    char	name[32];
} menu_pic;
class	CMenu
{
	char		menu_init;
	int			time;
	int			num_items, num_pics, push_item, cur_item;
	char		num_push_items, push_items[32];
	char		*menu_data;
	bitmap		*bitmaps;
	CPakFile	*menupf;

	menu_header	*header;
	menu_item	*items;
	menu_pic	*pics;

public:
	void	PushMenu(char *name);
	int		OpenMenu(char *name);
	void	CloseMenu();
	int		LoadPic(int num);
	int		LoadCurrentItemPics();
	void	FreePics();
	void	DrawMenu();
	int		UpdateMenu();
	int		MenuStatus();
};
#endif