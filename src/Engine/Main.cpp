/*
	Main.cpp		Copyright (C) 1998-1999 Damir Sagidullin
*/
#include	<Windows.h>
#include	<Windowsx.h>
#include	<Stdio.h>
#include	<Math.h>
#include	<Time.h>

#include	"Config.h"
#include	"Console.h"
#include	"Light.h"
#include	"Main.h"
#include	"MapFile.h"
#include	"Menu.h"
#include	"Move.h"
#include	"Net.h"
#include	"Physics.h"
#include	"Render.h"
#include	"Video.h"
#include	"Walltrack.h"

char	program_name[]="Miracle";
char	player_name[32]="_2S_8pi_2D_8e_2R";
char	program_path[1024]="";

GameExport	ge;

int		num_video_modes=0;
vm		video_modes[32];

int		keycode, kb_start=0, kb_end=0, faces_p_s;
char	key_buf[32];
char	key_state[256];
LPMF	key_func[256];

double	chop_temp;

int		frames=0, b_time, time1, time2, time3, c_time=0;
int		mouse_speed=200;
float	fps, delta_z=0;
char	waitaddlight=0, ddrawinit=0, invert_mouse=0, enable_menu=0;
char	pause=0, cur_key_state;

extern bitmap	sun;
extern model3d_t	mdl;

CMenu	menu;
CPakFile	unpak;

BOOL				bActive;
HWND				hwnd;

void	UpdateFrame()
{
	if(!ddrawinit)	return;
	if(!mgl.BeginActions())	return;
	time3 = time1;
	time1 = timeGetTime();
	if(!pause)	c_time += time1 - time3;
	if(map.map_already_loaded)
	{//	Render scene
		VectorCopy(entity.origin, cam_position);
		VectorCopy(entity.angles, cam_angles);
		delta_z *= 0.7f;
		cam_position[2] += 40 - delta_z + float(sin(c_time*pi*0.00075));
		//SetCameraInfo(&cam_position, &cam_angles);
		vec3_t	pos, angle;
		angle[0] = float(c_time<<2);
		angle[1] = float(c_time<<3);
		angle[2] = float(c_time<<4);
		float	rot_angle = c_time*3.1415f/6400;
		pos[0] = 0;//256*float(sin(rot_angle*0.5f));
		pos[1] = -1344;//-1000 + 256*float(cos(rot_angle*0.5f));
		pos[2] = 50*float(cos(rot_angle*2));
		mgl.DrawModel(pos, angle, &mdl, 0.5f, int(sin(rot_angle*2)*64) + 191, 0);//int(sin(rot_angle*4)*63)+192);
		pos[0] = 256*float(sin(rot_angle*4));
		pos[1] = -1152 + 192*float(cos(rot_angle*4));
		pos[2] = 100;
		mgl.AddDynamicLight(pos, float(rand()%128+384), 0, 0);
		pos[0] = -256*float(sin(rot_angle*4));
		pos[1] = -1152 + 192*float(cos(rot_angle*4));
		mgl.AddDynamicLight(pos, 0, float(rand()%128+384), 0);
		pos[0] = 0;
		pos[1] = -500;
		pos[2] = 300;
		mgl.DrawModel(pos, angle, &mdl, float(sin(rot_angle*2)+3)*0.1875f, 255, 1);
		//	модель, которая перед глазами
		/*angle[0] = angle[1] = angle[2] = 0;
		float	matrix[3];
		matrix[0] = matrix[2] = 0;
		matrix[1] = 128;
		RotateVector1(matrix, cam_angles);
		pos[0] = cam_position[0] + float(matrix[0]);
		pos[1] = cam_position[1] + float(matrix[1]);
		pos[2] = cam_position[2] + float(matrix[2]);
		mgl.DrawModel(pos, angle, &mdl, 1, 255);*/
		//	фича над пирамидой
		pos[0] = 0;
		pos[1] = -3328;
		pos[2] = 256;
		mgl.DrawSprite(pos, &sun, float(sin(rot_angle*4)*0.25)+0.75f, 255, 2);
		//	рендерение всей сцены
		faces_p_s = mgl.RenderWorld(cam_position, cam_angles, c_time);
		//AddWallTrack(&sun);
		Move(time1-time3);
		FindEntitiesState(c_time);
		CheckEntitiesMove();
		DrawStatus();
	} else	mgl.DrawBackground(time1);
	DrawConsole();
	menu.DrawMenu();
	time2 = timeGetTime();
	mgl.EndActions();
	if(!(++frames&15))
	{
		fps = 16*float(1000.0/(time2-b_time));
		b_time = time2;
	}
}
void	ClearKeyBuf()
{
	kb_end = kb_start = 0;
}
char	ReadKey()
{
	if(kb_start!=kb_end)
	{
		char	c = key_buf[kb_start++];
		kb_start &= 31;
		return	c;
	}
	return	0;
}

void finiObjects( void )
{
	//unrar.Done();
	//CloseNetLibrary();
	//CloseGraphLibrary();
	FreeMap();
	CloseLogFile();
	//DestroyWindow(hwnd);
}

long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch( message )
    {
	case WM_MOUSEMOVE:
		POINT	mouse_pos;
		int		dx, dy;
		GetCursorPos(&mouse_pos);
		dx = mouse_pos.x - (sx_size>>1);
		dy = mouse_pos.y - (sy_size>>1);
		if((dx!=0) || (dy!=0))
		{
			SetCursorPos(sx_size>>1, sy_size>>1);
			float	k = float(mouse_speed*mgl.Getf(FOV)/9000.0);
			entity.angles[2] += int((dx<<4)*k);
			if(invert_mouse)	dy = -dy;
			entity.angles[0] += int((dy<<4)*k);
			if(dy>0)
			{
				if(entity.angles[0]> 0x4000)	entity.angles[0] = 0x4000;
			} else
			{
				if(entity.angles[0]<-0x4000)	entity.angles[0] = -0x4000;
			}
		}
		break;
    case WM_ACTIVATEAPP:
        bActive = wParam;
        break;

    case WM_SETCURSOR:
        SetCursor(NULL);
        return TRUE;
	case WM_CHAR:
		if(((kb_end+1)&31) != kb_start)
		{
			key_buf[kb_end++] = wParam;
			kb_end &= 31;
		}
		break;

	case WM_KEYUP:
		if(wParam>255)	break;
		key_state[wParam] = (key_state[wParam]==3);
		break;
    case WM_KEYDOWN:
		if(wParam>255)	break;
		if(!key_state[wParam])
		{
			if(key_state[wParam]<2)	key_state[wParam] = 3;
			if((wParam<47) && (wParam>32) && (((kb_end+1)&31) != kb_start))
			{
				key_buf[kb_end++] = wParam-16;
				kb_end &= 31;
			}
		}
		if((wParam>=VK_NUMPAD1) && (wParam<=VK_NUMPAD8))
			waitaddlight = wParam - VK_NUMPAD1 + 1;
        break;
	case	WM_LBUTTONDOWN:
		if(!key_state[0] && (key_state[0]<2))
			key_state[0] = 3;
		break;
	case	WM_RBUTTONDOWN:
		if(!key_state[1] && (key_state[1]<2))
			key_state[1] = 3;
		break;
	case	WM_MBUTTONDOWN:
		if(!key_state[2] && (key_state[2]<2))
			key_state[2] = 3;
		break;
	case	WM_LBUTTONUP:
		key_state[0] = (key_state[0]==3);
		break;
	case	WM_RBUTTONUP:
		key_state[1] = (key_state[1]==3);
		break;
	case	WM_MBUTTONUP:
		key_state[2] = (key_state[2]==3);
		break;
    case	WM_DESTROY:
        finiObjects();
        PostQuitMessage( 0 );
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL initFail( HWND hwnd )
{
    finiObjects();
    CPrintf("DirectDraw Init FAILED");
    DestroyWindow( hwnd );
    return FALSE;
}

void	ChangeMouseSpeed()
{
	if(num_cargs>1)
		sscanf(cargs[1], "%d", &mouse_speed);
	CPrintf("Mouse speed is \"%d\"", mouse_speed);
}
void	InvertMouse()
{
	if(invert_mouse)	CPrintf("Invert mouse - off");
	else	CPrintf("Invert mouse - on");
	invert_mouse = !invert_mouse;
}
void	Quit()
{
	PostMessage(hwnd, WM_CLOSE, 0, 0);
}
void	Pause()
{
	pause = !pause;
	cur_key_state = 2;
}
void	InitMenu()
{
	menu.OpenMenu("menu.pak");
}
void	CloseMenu()
{
	menu.CloseMenu();
}
void	PushMenu()
{
	menu.PushMenu("menu.pak");
}
void	ChangeMap()
{
	char	tmp_name[128];
	sprintf(tmp_name, "%s.pak", cargs[1]);
	LoadMap(tmp_name);
}
void	StartBinds()
{
	for(int	i=0; i<256; i++)
		key_func[i] = NULL;
	key_func[VK_UP] = key_func['W'] = MoveForward;
	key_func[VK_DOWN] = key_func['S'] = MoveBack;
	key_func['D'] = MoveRight;
	key_func['A'] = MoveLeft;
	key_func[VK_HOME] = MoveUp;
	key_func[VK_END] = MoveDown;
	key_func[' '] = key_func[1] = Jump;
	key_func[13] = StopMove;
	key_func['Q'] = TurnY0;
	key_func['E'] = TurnY1;
	key_func[VK_RIGHT] = TurnZ0;
	key_func[VK_LEFT] = TurnZ1;
	key_func[VK_PRIOR] = TurnX0;	//	PgUp
	key_func[VK_NEXT] = TurnX1;		//	PgDown
	key_func[192] = PushConsole;	//	~
	key_func[27] = PushMenu;		//	ESC
	key_func[19] = Pause;			//	Pause
}
static void	InitExports ()
{
	ge.CPrintf = CPrintf;
	ge.CPrintfMessage = CPrintfMessage;
}
static BOOL	doInit (HINSTANCE hInstance, int nCmdShow)
{
	InitExports();
	InitLogFile("miracle.log");
	//	Создаем оконное MD приложение
    WNDCLASS	wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon( hInstance, IDI_APPLICATION );
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = GetStockBrush(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = program_name;
    RegisterClass( &wc );
    hwnd = CreateWindowEx(	0,
							program_name,
							program_name,
							WS_POPUP,
							0,
							0,
							GetSystemMetrics(SM_CXSCREEN),
							GetSystemMetrics(SM_CYSCREEN),
							NULL,
							NULL,
							hInstance,
							NULL );
    if( !hwnd )	return	FALSE;
	ShowWindow( hwnd, nCmdShow );
    UpdateWindow( hwnd );
    SetFocus( hwnd );
	if(!LoadGraphicLibrary("SoftRend.dll"))	return	0;
	if(!LoadNetLibrary("NetGame.dll"))	return	0;
	PushConsole();
	if(!mgl.Init(hwnd, (void *)&ge))	return	0;
	sx_size = mgl.Geti(SX_SIZE);
	sy_size = mgl.Geti(SY_SIZE);
	bits = mgl.Geti(BPP);
	ddrawinit = 1;
	num_video_modes = mgl.GetPosibleVideoModes(video_modes);
	//	Загружаем необходимые графиеские данные
	InitConsole();
	mgl.SetBackground(&background);
	mgl.SetFont(&font);
	UpdateFrame();
	//	Создаем консоль (для данного режима)
	MakeConsole();
	//	Инициализируем команды консоли
	InitConsoleCommands();
	//	Bind'им начальное состояние клавиш
	StartBinds();
	//	Создаем физическую модель игрока
	memset(&entity, 0, sizeof(entity_t));
	entity.mins[0] = -16;
	entity.mins[1] = -16;
	entity.mins[2] = -24;
	entity.maxs[0] = 16;
	entity.maxs[1] = 16;
	entity.maxs[2] = 64;
	//	Выполняем начальный конфигурационный файл
	StartLocalServer();
	LoadConfig("miracle.cfg");
	b_time = timeGetTime();
    return	1;
}

void	FindCurDirectory()
{
	char	*path = GetCommandLine();
	if(path[0]=='"')	path++;
	int		i=0;
	while((path[i]!='"') && (path[i]!=' ') && (i<1000))	i++;
	if(i>=1000)	return;
	while((path[i]!='\\') && (i>0))	i--;
	if(i<2)	return;
	memcpy(program_path, path, i+1);
	//CPrintf(path);
	//CPrintf(program_path);
}

int	PASCAL WinMain(	HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpCmdLine, int nCmdShow)
{
    MSG         msg;
	FindCurDirectory();
	if(!LoadGraphics())	return	FALSE;
    if( !doInit(hInstance, nCmdShow) )	return FALSE;
    while(1)
    {
        if(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
        {
            if(!GetMessage(&msg, NULL, 0, 0))
				return	msg.wParam;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
			if( bActive )
			{
				for(int	i=0; i<256; i++)
				{
					if(key_func[i]==NULL)	continue;
					if(key_state[i]==3)
					{
						cur_key_state = 3;
						key_func[i]();
						key_state[i] = cur_key_state;
					} else
					if(key_state[i]==1)
					{
						key_func[i]();
						key_state[i] = 0;
					}
				}
				UpdateFrame();
				FindZoom(time1 - time3);

				menu.UpdateMenu();
				UpdateConsoleString();

				if(waitaddlight && map.map_already_loaded)
				{
					omni_light	light;
					switch(waitaddlight)
					{
					case	1:
						light.r_rate = 512*64*64;
						light.g_rate = 0;
						light.b_rate = 0;
						break;
					case	2:
						light.r_rate = 0;
						light.g_rate = 512*64*64;
						light.b_rate = 0;
						break;
					case	3:
						light.r_rate = 0;
						light.g_rate = 0;
						light.b_rate = 512*64*64;
						break;
					case	4:
						light.r_rate = 512*64*64;
						light.g_rate = 512*64*64;
						light.b_rate = 0;
						break;
					case	5:
						light.r_rate = 0;
						light.g_rate = 512*64*64;
						light.b_rate = 512*64*64;
						break;
					case	6:
						light.r_rate = 512*64*64;
						light.g_rate = 0;
						light.b_rate = 512*64*64;
						break;
					case	7:
						light.r_rate = 512*64*64;
						light.g_rate = 256*64*64;
						light.b_rate = 0;
						break;
					case	8:
						light.r_rate = 256*64*64;
						light.g_rate = 256*64*64;
						light.b_rate = 256*64*64;
						break;
					}
					waitaddlight = 0;
					light.max_radius = 576;
					VectorCopy(cam_position, light.pos);
					AddOmniLight(&light);
					frames=0;
					b_time = timeGetTime();
				}
			}
        else
			WaitMessage();
    }
}
