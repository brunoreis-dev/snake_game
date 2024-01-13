#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/select.h>

#define cols 50
#define rows 40
#define foods 100

char board[cols * rows];

int isGameOver = 0;

void enableRawMode()
{
  struct termios raw;
  tcgetattr(STDIN_FILENO, &raw);
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode()
{
  struct termios raw;
  tcgetattr(STDIN_FILENO, &raw);
  raw.c_lflag |= (ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int kbhit()
{
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &fds);
}

void fill_board()
{
  int x, y;

  for (y = 0; y < rows; y++)
  {
    for (x = 0; x < cols; x++)
    {
      if (x == 0 || y == 0 || x == cols - 1 || y == rows - 1)
      {
        board[y * cols + x] = '#';
      }
      else
      {
        board[y * cols + x] = ' ';
      }
    }
  }
}

void clear_screen()
{
  // Hack clear screen on Windows. Do not use!
  // system("cls");
  // on Linux & Mac
  system("clear");
}

void print_board()
{
  int x, y;

  for (y = 0; y < rows; y++)
  {
    for (x = 0; x < cols; x++)
    {
      putchar(board[y * cols + x]);
    }
    putchar('\n');
  }
}

#define SNAKE_MAX_LEN 256
struct SnakePart
{
  int x, y;
};
struct Snake
{
  int length;
  struct SnakePart part[SNAKE_MAX_LEN];
};
struct Snake snake;

struct Food
{
  int x, y;
  int consumed;
};
struct Food food[foods];

void draw_snake()
{
  int i;

  for (i = snake.length - 1; i > 0; i--)
  {
    board[snake.part[i].y * cols + snake.part[i].x] = '*';
  }
  board[snake.part[0].y * cols + snake.part[0].x] = '@';
}

void move_snake(int deltaX, int deltaY)
{
  int i;

  for (i = snake.length - 1; i > 0; i--)
  {
    snake.part[i] = snake.part[i - 1];
  }

  snake.part[0].x += deltaX;
  snake.part[0].y += deltaY;
}

void ready_keyboard()
{
  int ch = getchar();

  switch (ch)
  {
  case 'w':
    move_snake(0, -1);
    break;
  case 's':
    move_snake(0, 1);
    break;
  case 'a':
    move_snake(-1, 0);
    break;
  case 'd':
    move_snake(1, 0);
    break;
  }
}

void draw_food()
{
  int i;

  for (i = 0; i < foods; i++)
  {
    if (!food[i].consumed)
    {
      board[food[i].y * cols + food[i].x] = '+';
    }
  }
}

void setup_food()
{
  int i;

  for (i = 0; i < foods; i++)
  {
    food[i].x = 1 + rand() % (cols - 2);
    food[i].y = 1 + rand() % (rows - 2);
    food[i].consumed = 0;
  }
}

void setup_snake()
{
  snake.length = 1;
  snake.part[0].x = 1 + rand() % (cols - 2);
  snake.part[0].y = 1 + rand() % (rows - 2);
}

void game_rules()
{
  int i;

  for (i = 0; i < foods; i++)
  {
    if (!food[i].consumed)
    {
      if (food[i].x == snake.part[0].x &&
          food[i].y == snake.part[0].y)
      {
        food[i].consumed = 1;
        snake.length++;
      }
    }
  }

  if (snake.part[0].x == 0 || snake.part[0].x == cols - 1 ||
      snake.part[0].y == 0 || snake.part[0].y == rows - 1)
  {
    isGameOver = 1;
  }

  for (i = 1; i < snake.length; i++)
  {
    if (snake.part[0].x == snake.part[i].x &&
        snake.part[0].y == snake.part[i].y)
    {
      isGameOver = 1;
    }
  }
}

void printGameBoard()
{
  printf("Snake Game, Score: %d\n", snake.length * 100);
  print_board();
}

void printGameOverInfo()
{
  printf("GAME OVER, Final Score: %d\n", snake.length * 100);
  printf("Press enter to restart");
}

int main(int argc, char **argv)
{
  enableRawMode();

  srand(time(0));

  int isStopLoopWhenGameOver = 0;

  while (1)
  {
    setup_snake();
    setup_food();

    while (!isGameOver)
    {
      fill_board();
      draw_food();
      draw_snake();
      game_rules();
      clear_screen();
      printGameBoard();
      if (!isGameOver)
        ready_keyboard();
    }

    if (isStopLoopWhenGameOver == 0)
    {
      printGameOverInfo();
    }

    isStopLoopWhenGameOver = 1;

    while (kbhit())
    {
      usleep(100000);
    };

    if (getchar() == 10)
    {
      isGameOver = 0;
      clear_screen();
      isStopLoopWhenGameOver = 0;
    }
  }

  disableRawMode();
  return 0;
}
