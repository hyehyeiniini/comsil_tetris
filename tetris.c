#include "tetris.h"

static struct sigaction act, oact;

int main(){
	int exit=0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);	

	srand((unsigned int)time(NULL));
	createRankList();
	while(!exit){
		clear();
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_RANK: rank(); break;
		case MENU_EXIT: exit=1; break;
		default: break;
		}
	}

	endwin();
	system("clear");
	return 0;
}

void InitTetris(){
	int i,j;

	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0; // tetris 화면 초기화

	
	for (int i = 0; i < VISIBLE_BLOCKS; i++) {
		nextBlock[i] = rand() % 7;
	}

	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;	
	gameOver=0;
	timed_out=0;

	recommendY= 0;
	recommendX=0;
	recommendR=0;
	max_score=0;



	DrawOutline(); // 테트리스 기본 틀 그리기
	DrawField(); // Field그리기

	Node* root = (Node *)malloc(sizeof(Node));
	root->level = 0;
	root->child_num = 0;
	root->accumualtedScore = score;
	for(int a = 0; a < HEIGHT; a++){
		for(int b = 0; b < WIDTH; b++){
			root->recField[a][b]= field[a][b]; // 기록용 tetris 화면 초기화
		}
	}
	recommend(root);
	DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
	DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate,' '); // Current block 그리기
	DrawNextBlock(nextBlock); // Next block을 그리기
	PrintScore(score); // 점수 출력하기
}

void DrawOutline(){	
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2,WIDTH+10);
	printw("NEXT BLOCK");
	DrawBox(3,WIDTH+10,4,8);
	DrawBox(9,WIDTH+10,4,8);

	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(16,WIDTH+10);
	printw("SCORE");
	DrawBox(17,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;	
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score){
	move(18,WIDTH+11);
	printw("%8d",score);
}

void DrawNextBlock(int *nextBlock){
	int i, j;
	for( i = 0; i < 4; i++ ){
		move(4+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[1]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}

	for( i = 0; i < 4; i++ ){
		move(10+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[2]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

/////////////////////////첫주차 실습에서 구현해야 할 함수/////////////////////////

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			if (block[currentBlock][blockRotate][i][j] == 1){
				if (f[blockY+i][blockX+j] == 1) {return 0;}
				else if (blockY+i >= HEIGHT) {return 0;}
				else if (blockX+j < 0 || blockX+j >= WIDTH) {return 0;}
			}
		}
	}
	return 1;
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	int blockPX = blockX;
	int blockPY = blockY;
	int blockPRotate = blockRotate;

	//1. 이전 블록 정보를 찾는다. ProcessCommand의 switch문을 참조할 것
	switch (command){
	case KEY_UP:
		blockPRotate = (blockRotate == 0) ? 3 : (blockRotate-1) % 4; // 1-> 0, 2-> 1, 3->2, 0->3
		break;
	case KEY_DOWN:
		blockPY = blockY-1;
		break;
	case KEY_RIGHT:
		blockPX = blockX-1;
		break;
	case KEY_LEFT:
		blockPX = blockX+1;
		break;
	default:
		break;
	}

	//2. 이전 블록 정보를 지운다. DrawBlock함수 참조할 것.
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			if (block[currentBlock][blockPRotate][i][j] == 1){
				move(i+blockPY+1,j+blockPX+1);
				printw(".");
			}
		}
	}

	// 3. 고스트 블록 정보를 지운다.
	while(CheckToMove(field, currentBlock, blockPRotate, blockPY, blockPX)){
		blockPY++;
	}
	blockPY--;

	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			if (block[currentBlock][blockPRotate][i][j] == 1){
				move(i+blockPY+1,j+blockPX+1);
				printw(".");
			}
		}
	}
	move(HEIGHT, WIDTH+10);

	//4. 새로운 블록 정보를 그린다. 
	DrawBlockWithFeatures(blockY, blockX, currentBlock, blockRotate, ' ');
}

void BlockDown(int sig){
	// user code
	int drawFlag;
	timed_out = 0;

	// 1. 블록이 한 칸 내려갈 수 있는지 확인한다.
	// 2. 내려갈 수 있으면 블록을 아래로 한 칸 내리고 함수를 종료, 내려갈 수 없으면 다음 step을 수행한다.
	if (CheckToMove(field, nextBlock[0], blockRotate, blockY+1, blockX)){
		blockY++;
		DrawChange(field, KEY_DOWN, nextBlock[0], blockRotate, blockY, blockX);
	}
	else{
		if (blockY == -1){ // 3. 블록의 y좌표가 -1이라면 개임 종료 flag = true
			gameOver = TRUE;
			return;
		}
		// 4. 블록을 필드에 쌓는다.
		score += AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);

		// 5. 완전한 line이 있으면 지우고, 점수를 갱신한다.
		score += DeleteLine(field);
		DrawField();

		// 6. Next block -> curr block. Next block은 랜덤하게 결정
		for (int i = 0; i < VISIBLE_BLOCKS - 1; i++) {
			nextBlock[i] = nextBlock[i + 1];
		}
		nextBlock[VISIBLE_BLOCKS - 1] = rand() % 7;
		
		// 7. 현재 블록의 위치를 초기화한다.
		blockRotate=0;
		blockY=-1;
		blockX=WIDTH/2-2;

		// 8. Next 블록과 갱신된 score를 화면에 출력한다.
		DrawNextBlock(nextBlock);
		PrintScore(score);

		Node* root = (Node *)malloc(sizeof(Node));
		root->level = 0;
		root->child_num = 0;
		root->accumualtedScore = score;
		for(int a = 0; a < HEIGHT; a++){
			for(int b = 0; b < WIDTH; b++){
				root->recField[a][b]= field[a][b]; // 기록용 tetris 화면 초기화
			}
		}
		max_score = 0;
		recommendX = 0;
		recommendY = 0;
		recommendR = 0;

		recommend(root);

		// 9. Field와 current block을 화면에 갱신하여 출력한다.
		DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate, ' ');
	}
}


int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	// user code
	// Block이 추가된 영역의 필드값을 바꾼다.
	int touch = 0;
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			if (block[currentBlock][blockRotate][i][j] == 1){
				if (f[blockY+i+1][blockX+j] == 1 || blockY+i+1 == 22 ){ // if문 추가됨
					touch++;
				}
				f[blockY+i][blockX+j] = 1;
			}
		}
	}
	return touch * 10;
}

int DeleteLine(char f[HEIGHT][WIDTH]){
	// user code
	int i, j, k;
	int count = 0; // 지워진 줄의 갯수 
	for(j = HEIGHT-1; j>= 0; j--){
		for(i = 0; i<WIDTH; i++){
			if(f[j][i] == 0){ // 빈줄이면 break
				break;
			}
		}
		if (i == WIDTH){ // 꽉 찬 줄
			// 한 줄 내린다
			for (k = j; k >= 1; k--){
				for (i = 0; i<WIDTH; i++){
					f[k][i] = f[k-1][i];
				}
			}
			// 지워진 줄의 갯수 업데이트
			count++;
			j++; // 다시 내린 줄부터 확인
		}
	}
	// 점수 갱신
	return count * count * 100;
}

///////////////////////////////////////////////////////////////////////////

void DrawShadow(int y, int x, int blockID,int blockRotate){
	// user code
	while(CheckToMove(field, blockID, blockRotate, y, x)){
		y++;
	}
	DrawBlock(y-1, x, blockID, blockRotate, '/');
}

void DrawBlockWithFeatures(int y, int x, int blockID,int blockRotate, char tile){
	DrawShadow(y, x, blockID, blockRotate);
	DrawRecommend(recommendY, recommendX, blockID, recommendR);
	DrawBlock(y, x, blockID, blockRotate, tile);
	
}

void createRankList(){
	// 목적: Input파일인 "rank.txt"에서 랭킹 정보를 읽어들임, 읽어들인 정보로 랭킹 목록 생성
	// 1. "rank.txt"열기
	// 2. 파일에서 랭킹정보 읽어오기
	// 3. LinkedList로 저장
	// 4. 파일 닫기
	FILE *fp;
	int i, j;

	//1. 파일 열기
	fp = fopen("rank.txt", "r");

	// 2. 정보읽어오기
	/* int fscanf(FILE* stream, const char* format, ...);
	stream:데이터를 읽어올 스트림의 FILE 객체를 가리키는 파일포인터
	format: 형식지정자 등등
	변수의 주소: 포인터
	return: 성공할 경우, fscanf 함수는 읽어들인 데이터의 수를 리턴, 실패하면 EOF리턴 */
	// EOF(End Of File): 실제로 이 값은 -1을 나타냄, EOF가 나타날때까지 입력받아오는 if문
	if (fp == NULL) return;
	if (fscanf(fp, "%d", &score_num) != EOF) {
		if (score_num == 0){
			printw("no score_num\n");
			return;
		}
		else printw("score_num :%d\n", score_num);

		dummyRoot = malloc(sizeof(RankNode));
		RankNode * curr = dummyRoot;
		int i = 0;
		while (i < score_num){
			RankNode * newNode = (RankNode *)malloc(sizeof(RankNode));
			fscanf(fp, "%s %d", newNode->name, &(newNode->score));
			newNode->next = NULL;

			curr->next = newNode;
			curr = curr->next;
			i++;
		}
		curr = NULL;
		free(curr);
	}
	// 4. 파일닫기
	fclose(fp);
	return;
}


void rank(){
	//목적: rank 메뉴를 출력하고 점수 순으로 X부터~Y까지 출력함
	createRankList();
	//1. 문자열 초기화
	// int X=1, Y=score_num, ch, i, j;
	clear();
	int ch;

	//2. printw()로 3개의 메뉴출력
	printw("1. list ranks from X to Y\n");
	printw("2. list ranks by a specific name\n");
	printw("3. delete a specific rank\n");

	//3. wgetch()를 사용하여 변수 ch에 입력받은 메뉴번호 저장
	ch = wgetch(stdscr);

	//4. 각 메뉴에 따라 입력받을 값을 변수에 저장
	//4-1. 메뉴1: X, Y를 입력받고 적절한 input인지 확인 후(X<=Y), X와 Y사이의 rank 출력
	if (ch == '1') {
		rank1();
	}

	//4-2. 메뉴2: 문자열을 받아 저장된 이름과 비교하고 이름에 해당하는 리스트를 출력
	else if ( ch == '2') {
		rank2();
	}

	//4-3. 메뉴3: rank번호를 입력받아 리스트에서 삭제
	else if ( ch == '3') {
		rank3();
	}
	getch();
}

void rank1(){
	// 문자열 초기화
	int X=1, Y=score_num, i, j;

	printw("X: ");
	echo();
	scanw("%d", &X);
	noecho();
	if (X == KEY_ENTER){
		X = 1;
	}

	printw("Y: ");
	echo();
	scanw("%d", &Y);
	noecho();
	if (Y == KEY_ENTER){
		Y = score_num;
	}

	if (X > Y || X <= 0 || Y > score_num){
		printw("search failure : no rank in the list\n");
		getch();
		return;
	}
	printw("         name        |    score    \n");
	printw("----------------------------------\n");
	
	RankNode * curr = dummyRoot;
	for (int i = 0; i < X; i++){
		curr = curr->next;
	}
	for (int i = X; i < Y+1; i++){
		printw("%-20s | %-10d\n", curr->name, curr->score);
		curr = curr->next;
	}
}

void rank2(){
	char str[NAMELEN+1];
	int check = 0;

	printw("Input the name: ");
	echo();
	scanw("%s", str);
	noecho();

	printw("         name        |    score    \n");
	printw("----------------------------------\n");

	RankNode* curr = dummyRoot;
	while (curr->next != NULL){
		curr = curr->next;
		if (!strcmp(curr->name, str)){
			check = 1;
			printw("%-20s | %-10d\n", curr->name, curr->score);
		}
	}
	if (check == 0){
		printw("search failure : no rank in the list\n");
	}
}

void rank3(){
	int num;
	printw("Input the rank: ");
	echo();
	scanw("%d", &num);
	noecho();

	if (num > score_num){
		printw("search failure : no rank in the list\n");
		return;
	}

	RankNode* curr = dummyRoot;
	for (int i = 0; i < num-1; i++){
		curr = curr->next;
	}
	RankNode* tmp = curr->next;
	curr->next = curr->next->next;
	free(tmp);
	printw("result: the rank deleted\n");
	writeRankFile();
}

void writeRankFile(){
	// 목적: 추가된 랭킹 정보가 있으면 새로운 정보를 "rank.txt"에 쓰고 없으면 종료
	int sn, i;
	//1. "rank.txt" 연다
	FILE *fp = fopen("rank.txt", "w");

	//2. 랭킹 정보들의 수를 "rank.txt"에 기록
	fprintf(fp, "%d\n", score_num);

	//3. 탐색할 노드가 더 있는지 체크하고 있으면 다음 노드로 이동, 없으면 종료
	if (score_num == 0){
		return;
	}

	RankNode* curr = dummyRoot;
	curr = curr->next;
	while (curr != NULL){
		fprintf(fp, "%s %d\n", curr->name, curr->score);
		curr = curr->next;
	}
	curr = dummyRoot;
	while (curr != NULL){
		RankNode* tmp = curr;
		curr = curr->next;
		free(tmp);
	}
	fclose(fp);
}

void newRank(int score){
	// 목적: GameOver시 호출되어 사용자 이름을 입력받고 score와 함께 리스트의 적절한 위치에 저장
	char str[NAMELEN+1];
	int i, j;
	clear();
	//1. 사용자 이름을 입력받음
	printw("your name: ");
	echo();
	getstr(str);
	noecho();
	//2. 새로운 노드를 생성해 이름과 점수를 저장, score_number가
	RankNode* newNode = (RankNode*)malloc(sizeof(RankNode));
	newNode->score = score;
	strcpy(newNode->name, str);
	newNode->next = NULL;

	if(score_num == 0) {
		dummyRoot = (RankNode*)malloc(sizeof(RankNode));
		dummyRoot->next = newNode;
	}
	else {
		RankNode* curr = dummyRoot->next;
		int cmp_score = curr->score;
		if (score > cmp_score){
			newNode->next = curr;
			dummyRoot->next = newNode;
		}
		else{
			while(score <= cmp_score && curr->next != NULL){
				cmp_score = curr->next->score;
				if (score > cmp_score) break;
				curr = curr->next;
			}
			newNode->next = curr->next;
			curr->next = newNode;
		}
	}
	score_num++;
	writeRankFile();
}


void DrawRecommend(int y, int x, int blockID,int blockRotate){
	// user code
	DrawBlock(y, x, blockID, blockRotate, 'R');
}

int recommend(Node* root){
	int max = 0;
	int x, y;
	int rotate_range;

	switch (root->curBlockID){
	case 0:
		rotate_range = 2;
		break;
	case 1:
		rotate_range = 4;
		break;
	case 2:
		rotate_range = 4;
		break;
	case 3:
		rotate_range = 4;
		break;
	case 4:
		rotate_range = 1;
	case 5:
		rotate_range = 2;
		break;
	case 6:
		rotate_range = 2;
		break;
	default:
		rotate_range = 0;
		break;
}

	for (int i = 0; i < 3; i++){
		for (int j = 0; j <= WIDTH; j++){
			if (!CheckToMove(root->recField, nextBlock[root->level], i, 0, j)){
				continue;
			}
			Node * newNode = (Node *)malloc(sizeof(Node));
			newNode->accumualtedScore = root->accumualtedScore;
			if (root->level + 1 > BLOCK_NUM){
				newNode->curBlockID = rand()%7;
			}
			else newNode->curBlockID = nextBlock[root->level];
			x = j;
			y = blockY;
			for(int a = 0; a < HEIGHT; a++){
				for(int b = 0; b < WIDTH; b++){
					newNode->recField[a][b]= root->recField[a][b]; // 기록용 tetris 화면 초기화
				}
			}
			while(CheckToMove(newNode->recField, newNode->curBlockID , i, y+1, x)){
				y++;
			}

			newNode->accumualtedScore += AddBlockToField(newNode->recField, newNode->curBlockID, i, y, x);
			newNode->accumualtedScore += DeleteLine(newNode->recField);
			newNode->level = root->level+1;
			newNode->recBlockRotate = i;
			newNode->parent = root;
			newNode->child_num = 0;
			newNode->recBlockX = j;
			newNode->recBlockY = y;
	
			root->child[root->child_num] = newNode;
			root->child_num += 1;

			if (newNode->level != VISIBLE_BLOCKS){
				recommend(newNode);
			}
		
			int temp = newNode->accumualtedScore;
			if (temp > max_score && newNode->level == VISIBLE_BLOCKS){
				max_score = temp;
				move(7, WIDTH+25);
				printw("max : %d\n", max_score);
				Node* tmp = newNode;
				while (tmp->level > 1){
					tmp = tmp->parent;
				}
				recommendR = tmp->recBlockRotate;
				recommendX = tmp->recBlockX;
				recommendY = tmp->recBlockY;
				move(8, WIDTH+25);
				printw("(%d, %d, %d)\n", recommendX, recommendY, recommendR);
			}
		}
	}
}

void get_rec_pos(Node * root, int * max, int * res_R, int * res_X, int * res_Y){
	Node* curr = root;
	Node* next_block;

	for (int i = 0; i < curr->child_num; i++){
		if (curr->child[i]->level == VISIBLE_BLOCKS){
			if (curr->child[i]->accumualtedScore > *max){
				*max = curr->child[i]->accumualtedScore;
				next_block = curr->child[i];
				while (next_block->level > 1){
					next_block = next_block->parent;
				}
				*res_R = next_block->recBlockRotate;
				*res_X = next_block->recBlockX;
				*res_Y = next_block->recBlockY;
			}
		}
		else{
			get_rec_pos(curr->child[i], max, res_R, res_X, res_Y);
		}
	}
}

void recommendedPlay(){
	// user code
}
