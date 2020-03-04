#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<windows.h>
#include<conio.h>

//Global Variables Declaration
char selection = 0;

//Functions Declaration
void HideCursor();
void StartScreen();
void PressAnyKey();
void GamePlay();
void HelpScreen();


int main() {
	HideCursor();
	system("cls");
	system("mode con cols=80 lines=30");
	while (1) {
		StartScreen();
		PressAnyKey();
		selection = _getch();
		system("cls");
		switch (selection) {
		case '1': GamePlay(); return 0;
		case '2': HelpScreen(); break;
		}
	} //while
	return 0;
}

//Functions
void HideCursor() {
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void StartScreen() {
	printf("####################\n");
	Sleep(100);
	printf("#    * G A M E *   #\n");
	printf("#  *** TETRIS ***  #\n");
	Sleep(100);
	printf("#                  #\n");
	printf("#                  #\n");
	Sleep(100);
	printf("#   * 1 : start *  #\n");
	printf("#                  #\n");
	Sleep(100);
	printf("#                  #\n");
	printf("#                  #\n");
	Sleep(100);
	printf("#   * 2 : help  *  #\n");
	printf("#                  #\n");
	Sleep(100);
	printf("#                  #\n");
	printf("#                  #\n");
	Sleep(100);
	printf("####################");
}

void HelpScreen() {
	printf("help me!");
}

void PressAnyKey() {
	while (1) {
		if (_kbhit()) {
			return;
		}
	}
}