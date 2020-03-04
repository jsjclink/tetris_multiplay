#define _CRT_SECURE_NO_WARNINGS

#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib, "ws2_32.lib")

//Header file
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<windows.h>
#include<conio.h>

//Global Variables Declaration
int map[20][14];
int copymap[20][14];

int blockset[7][4][4][4];
int block[4][4];
int block_x[4];
int block_y[4];
int blocktype = 0;
int blockphase = 0;

int originalpoint_x = 0;
int originalpoint_y = 0;
int tempblock_x[4];
int tempblock_y[4];

int templines[19][12];
int clearlineposition[5];

int score = 0;

int imserver = 0;
char sbuffer[1024];
char rbuffer[1024];

SOCKET s,cs;
WSADATA wsaData;
SOCKADDR_IN sin, cli_addr;
//Functions Declararion
void GetKeyInput();

void Gotoxy(int x, int y);
void PrintMap();
void PrintMap2();
void PrintBlock();
void EraseBlock();//name change erase block
void PrintStatus();



void CreateBlock();
void ChooseRandomBlock();
void AddBlocktoMap();
void BlockMoveDown();
void BlockMoveLeft();
void BlockMoveRight();
void RotateBlock();

int CheckBlockCollisionDown();
int CheckBlockCollisionLeft();
int CheckBlockCollisionRight();
int CheckBlockCollisionRotate();

void CheckLineClear();
void LineClear();
void RemoveLine(int lineidx);
void MoveLinesDownward(int lineidx);

void MultiSend();
void MultiRecv();
void CopyMaptoBuffer();
void CopyBuffertoMap2();
int map2[20][14];

/***************************************************************************/

void GamePlay() {
	//Init
	int framecount = 0;

	clock_t CurTime, OldTime, OldTimeBlockMove;
	OldTime = clock();
	OldTimeBlockMove = clock();

	//Multi

	if (WSAStartup(WINSOCK_VERSION, &wsaData)) {
		printf("WSAStartup failed, ErrorCode : %d\n", WSAGetLastError());
		return ;
	}

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s == INVALID_SOCKET) {
		printf("Failed to create socket, ErrorCode : %d\n", WSAGetLastError());
		WSACleanup(); return ;
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons(1234);
	sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	if(connect(s, (SOCKADDR*)&sin, sizeof(sin))!=0){
		printf("There is no server, Create server ...\n");
		closesocket(s); WSACleanup();

		//Create Server

		if (WSAStartup(WINSOCK_VERSION, &wsaData)) {
			printf("WSAStartup failed, ErrorCode : %d\n", WSAGetLastError());
			return ;
		}

		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (s == INVALID_SOCKET) {
			printf("Failed to create socket, ErrorCode : %d\n", WSAGetLastError());
			WSACleanup(); return ;
		}

		sin.sin_family = AF_INET;
		sin.sin_port = htons(1234);
		sin.sin_addr.S_un.S_addr = htonl(ADDR_ANY);

		if (bind(s, (SOCKADDR*)&sin, sizeof(sin)) == SOCKET_ERROR) {
			printf("Failed to bind, ErrorCode : %d\n", WSAGetLastError());
			closesocket(s); WSACleanup(); return ;
		}

		if (listen(s, 10) != 0) {
			printf("Failed to set listen mode, ErrorCode : %d\n", WSAGetLastError());
			closesocket(s); WSACleanup(); return ;
		}

		printf("Start Server\n");
		printf("Waiting for client connect ...\n");

		int cli_size = sizeof(cli_addr);
		cs = accept(s, (SOCKADDR*)&cli_addr, &cli_size);
		if (cs == INVALID_SOCKET) {
			printf("Failed to accept, ErrorCode : %d\n", WSAGetLastError());
			closesocket(s); WSACleanup(); return ;
		}
		//Success
		printf("Connected to client\n");
		printf("Waiting for start\n");
		imserver = 1;
		recv(cs, rbuffer, 1024, 0);
	}
	else {
		//Success
		printf("Connected to server.\n");
		printf("Press Any Key to start game.\n");
		_getch();
		send(s, sbuffer, 1024, 0);
	}
	

	//start
	system("cls");
	CreateBlock();
	
	while (1) {
		//Data Update
		if (imserver == 1) {
			Sleep(100);
			MultiSend();
			MultiRecv();

		}
		else {
			MultiRecv();
			Sleep(100);
			MultiSend();
		}


		//Keyboard input
		GetKeyInput();

		//Implementation before print(BlockMoveDown, LineClear)
		CurTime = clock();
		if (CurTime - OldTimeBlockMove > 500) {
			OldTimeBlockMove = CurTime;
			//Implementation
			if (CheckBlockCollisionDown() == 0) {
				BlockMoveDown();
			}
			else {
				score++;
				EraseBlock();
				AddBlocktoMap();
				CheckLineClear();
				LineClear();
				system("cls");
				CreateBlock();
				//Add Score

			}
		}

		//Print
		PrintMap();
		PrintMap2();
		PrintBlock();
		PrintStatus();

		//Wait
		while (1) {
			CurTime = clock();
			if (CurTime - OldTime > 100) {
				OldTime = CurTime;
				break;
			}
		}
		framecount++;
	}
}

/***************************************************************************/

void GetKeyInput() {
	int inp;
	if (_kbhit()) {
		inp = _getch();
		if (inp == 'c') {
			if (CheckBlockCollisionRotate()==0) {
				RotateBlock();
			}
		}
		switch (inp) {
		case 'j': {
			if (CheckBlockCollisionLeft()) { break; }
			BlockMoveLeft();
			break;
		}
		case 'l': {
			if (CheckBlockCollisionRight()) { break; }
			BlockMoveRight();
			break;
		}
		}
	}
}

/***************************************************************************/
void Gotoxy(int x, int y) {
	COORD CursorPosition = { x, y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), CursorPosition);
}

void PrintMap() {
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 14; j++) {
			if (map[i][j] == 1) {
				Gotoxy(j, i);
				printf("#");
			}
		}
	}
}

void PrintMap2() {
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 14; j++) {
			if (rbuffer[14 * i + j] == 1) {
				Gotoxy(16 + j, i);
				printf("#");
			}
		}
	}
}

void PrintBlock() {
	for (int i = 0; i < 4; i++) {
		Gotoxy(block_x[i], block_y[i]);
		printf("@");
	}
}

void EraseBlock() {
	for (int i = 0; i < 4; i++) {
		Gotoxy(block_x[i], block_y[i]);
		printf(" ");
	}
}

void PrintStatus() {
	/*
	Gotoxy(15, 0);
	printf("score : %4d", score*100);
	for (int i = 0; i < 4; i++) {
		Gotoxy(15, 2 * i + 1);
		printf("block_x[%d] : %3d", i, block_x[i]);
		Gotoxy(15, 2 * i + 2);
		printf("block_y[%d] : %3d", i, block_y[i]);
	}
	*/
	Gotoxy(31, 0);
	printf("# Score : %4d", score * 100);
	Gotoxy(31, 1);
	if (imserver == 1) {
		printf("Im server");
	}
	else {
		printf("Im client");
	}
}

/***************************************************************************/

void CreateBlock() {
	//initialization
	ChooseRandomBlock();
	int startplace_x = 5, startplace_y = 0;
	int startpoint_x = 0, startpoint_y = 0;
	int endpoint_x = 3, endpoint_y = 3;
	int isfound = 0;
	int count = 0;
	//find startpoint
	for (int i = 0; i < 4; i++) { //x
		for (int j = 0; j < 4; j++) {
			if (block[j][i] == 1) { startpoint_x = i; isfound = 1; break; }
		}
		if (isfound == 1) { break; }
	}
	isfound = 0;

	for (int i = 0; i < 4; i++) { //y
		for (int j = 0; j < 4; j++) {
			if (block[i][j] == 1) { startpoint_y = i; isfound = 1; break; }
		}
		if (isfound == 1) { break; }
	}
	isfound = 0;

	//find endpoint
	for (int i = 3; i >= 0; i--) { //x
		for (int j = 0; j < 4; j++) {
			if (block[j][i] == 1) { endpoint_x = i; isfound = 1; break; }
		}
		if (isfound == 1) { break; }
	}
	isfound = 0;

	for (int i = 3; i >= 0; i--) { //y
		for (int j = 0; j < 4; j++) {
			if (block[i][j] == 1) { endpoint_y = i; isfound = 1; break; }
		}
		if (isfound == 1) { break; }
	}

	//allocate position to each block
	for (int i = startpoint_y; i <= endpoint_y; i++) {
		for (int j = startpoint_x; j <= endpoint_x; j++) {
			if (block[i][j] == 1) {
				block_x[count] = j - startpoint_x + startplace_x;
				block_y[count] = i - startpoint_y + startplace_y;
				count++;
			}
		}
	}

	//calculate originalpoint
	originalpoint_x = block_x[0] - startpoint_x;
	originalpoint_y = block_y[0] - startpoint_y;
}

void ChooseRandomBlock() {
	srand(time(NULL));
	if (imserver == 1) {
		srand(time(NULL)-199);
	}
	blocktype = rand() % 7;
	blockphase = rand() % 4;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			block[i][j] = blockset[blocktype][blockphase][i][j];
		}
	}
}

void AddBlocktoMap() {
	for (int i = 0; i < 4; i++) {
		map[block_y[i]][block_x[i]] = 1;
		block_x[i] = 0;
		block_y[i] = 0;
	}
}

void BlockMoveDown() {
	for (int i = 0; i < 4; i++) {
		Gotoxy(block_x[i], block_y[i]);
		printf("%c", 0);
		block_y[i]++;
	}
	originalpoint_y++;
}

void BlockMoveLeft() {
	EraseBlock();
	for (int i = 0; i < 4; i++) {
		block_x[i]--;
	}
	originalpoint_x--;
}

void BlockMoveRight() {
	EraseBlock();
	for (int i = 0; i < 4; i++) {
		block_x[i]++;
	}
	originalpoint_x++;
}

void RotateBlock() {
	EraseBlock();
	blockphase++;
	blockphase = blockphase % 4;

	int count = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (blockset[blocktype][blockphase][i][j] == 1) {
				block_x[count] = j + originalpoint_x;
				block_y[count] = i + originalpoint_y;
				count++;
			}
		}
	}
}



/***************************************************************************/
int CheckBlockCollisionDown() {
	for (int i = 0; i < 4; i++) {
		if (map[block_y[i] + 1][block_x[i]] == 1) { //한칸 밑에 블럭 있는지 확인
			return 1;
		}
	}
	return 0;
}

int CheckBlockCollisionLeft() {
	for (int i = 0; i < 4; i++) {
		if (map[block_y[i]][block_x[i] - 1] == 1) { //한칸 왼쪽에 블럭 있는지 확인
			return 1;
		}
	}
	return 0;
}

int CheckBlockCollisionRight() {
	for (int i = 0; i < 4; i++) {
		if (map[block_y[i]][block_x[i] + 1] == 1) { //한칸 오른쪽에 블럭 있는지 확인
			return 1;
		}
	}
	return 0;
}

int CheckBlockCollisionRotate() {
	//init
	int count = 0;
	//한번 돌린 후를 확인하기 위해 blockphase++
	blockphase++;
	blockphase = blockphase % 4;

	for (int i = 0; i < 4; i++) { //한칸 돌린 상태를 tempblock_x, tempblock_y에 저장
		for (int j = 0; j < 4; j++) {
			if (blockset[blocktype][blockphase][i][j] == 1) {
				tempblock_x[count] = j + originalpoint_x;
				tempblock_y[count] = i + originalpoint_y;
				count++;
			}
		}
	}

	for (int i = 0; i < 4; i++) { //temp_x, temp_y중에서 map에 충돌하는 것이 있는지 확인
		if (map[tempblock_y[i]][tempblock_x[i]] == 1) {
			//blockphase-- 를 하기 위해 +3, %4를 해줌(0에서 --하면 -1이 될 수도 있기 때문에 덧셈만 함)
			blockphase += 3;
			blockphase = blockphase % 4;

			return 1;
		}
	}
	//blockphase-- 를 하기 위해 +3, %4를 해줌(0에서 --하면 -1이 될 수도 있기 때문에 덧셈만 함)
	blockphase += 3;
	blockphase = blockphase % 4;
	return 0;
}

/***************************************************************************/
void CheckLineClear() {
	//init
	int blockcount;
	int k;
	memset(clearlineposition, -1, sizeof(clearlineposition));

	//check from the bottom to top
	for (int i = 18; i >= 0; i--) {
		blockcount = 0;
		for (int j = 1; j < 13; j++) {
			if (map[i][j] == 1) { blockcount++; }
		}
		if (blockcount == 12) { //if all line is full of block, add line position(index) to clearlineposition[]
			k = 0;
			while (clearlineposition[k] != -1) {
				k++;
			}
			clearlineposition[k] = i;
		}
	}
}

void LineClear() {
	for (int i = 0; i < 5; i++) {
		if (clearlineposition[i] == -1) { return; }
		RemoveLine(clearlineposition[i]);
		MoveLinesDownward(clearlineposition[i]);
		score += 900;
	}
}

void RemoveLine(int lineidx) {
	for (int i = 1; i < 13; i++) {
		map[lineidx][i] = 0;
	}
}

void MoveLinesDownward(int lineidx) {
	for (int i = lineidx - 1; i >= 0; i--) {
		for (int j = 1; j < 13; j++) {
			if (map[i][j] == 1) {
				map[i][j] = 0;
				map[i + 1][j] = 1;
			}
		}
	}
}

/***************************************************************************/

void MultiSend() {
	CopyMaptoBuffer();
	if (imserver == 1) {
		//server
		send(cs, sbuffer, 1024, 0);
	}
	else {
		//client
		send(s, sbuffer, 1024, 0);
	}
}

void MultiRecv() {
	if (imserver == 1) {
		//server
		recv(cs, rbuffer, 1024, 0);
	}
	else {
		//client
		recv(s, rbuffer, 1024, 0);
	}
	void CopyBuffertoMap2();
}

void CopyMaptoBuffer() {
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 14; j++) {
			sbuffer[14 * i + j] = map[i][j];
		}
	}
}

void CopyBuffertoMap2() {
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 14; j++) {
			map2[i][j] = rbuffer[14 * i + j];
		}
	}
}

/***************************************************************************/

//Arrays

int map[20][14] = {
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

int copymap[20][14] = {
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2
};

int map2[20][14] = {
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,0,0,0,0,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1
};


int testblock[4][4] = {
	0,0,1,0,
	1,1,1,0,
	0,0,0,0,
	0,0,0,0
};

int blockset[7][4][4][4] = {
		0,0,0,0,
		1,1,1,1,
		0,0,0,0,
		0,0,0,0,

		0,0,1,0,
		0,0,1,0,
		0,0,1,0,
		0,0,1,0,

		0,0,0,0,
		0,0,0,0,
		1,1,1,1,
		0,0,0,0,

		0,1,0,0,
		0,1,0,0,
		0,1,0,0,
		0,1,0,0,

		1,0,0,0,
		1,1,1,0,
		0,0,0,0,
		0,0,0,0,

		0,1,1,0,
		0,1,0,0,
		0,1,0,0,
		0,0,0,0,

		0,0,0,0,
		1,1,1,0,
		0,0,1,0,
		0,0,0,0,

		0,1,0,0,
		0,1,0,0,
		1,1,0,0,
		0,0,0,0,

		0,0,1,0,
		1,1,1,0,
		0,0,0,0,
		0,0,0,0,

		0,1,0,0,
		0,1,0,0,
		0,1,1,0,
		0,0,0,0,

		0,0,0,0,
		1,1,1,0,
		1,0,0,0,
		0,0,0,0,

		1,1,0,0,
		0,1,0,0,
		0,1,0,0,
		0,0,0,0,


		0,1,1,0,
		0,1,1,0,
		0,0,0,0,
		0,0,0,0,

		0,1,1,0,
		0,1,1,0,
		0,0,0,0,
		0,0,0,0,

		0,1,1,0,
		0,1,1,0,
		0,0,0,0,
		0,0,0,0,

		0,1,1,0,
		0,1,1,0,
		0,0,0,0,
		0,0,0,0,

		0,1,1,0,
		1,1,0,0,
		0,0,0,0,
		0,0,0,0,

		0,1,0,0,
		0,1,1,0,
		0,0,1,0,
		0,0,0,0,

		0,0,0,0,
		0,1,1,0,
		1,1,0,0,
		0,0,0,0,

		1,0,0,0,
		1,1,0,0,
		0,1,0,0,
		0,0,0,0,

		0,1,0,0,
		1,1,1,0,
		0,0,0,0,
		0,0,0,0,

		0,1,0,0,
		0,1,1,0,
		0,1,0,0,
		0,0,0,0,

		0,0,0,0,
		1,1,1,0,
		0,1,0,0,
		0,0,0,0,

		0,1,0,0,
		1,1,0,0,
		0,1,0,0,
		0,0,0,0,

		1,1,0,0,
		0,1,1,0,
		0,0,0,0,
		0,0,0,0,

		0,0,1,0,
		0,1,1,0,
		0,1,0,0,
		0,0,0,0,

		0,0,0,0,
		1,1,0,0,
		0,1,1,0,
		0,0,0,0,

		0,1,0,0,
		1,1,0,0,
		1,0,0,0,
		0,0,0,0
};