#include "libvim.h"
#include "minunit.h"

void test_setup(void)
{
  vimKey("<Esc>");
  vimKey("<Esc>");
  vimExecute("e!");

  vimInput("g");
  vimInput("g");
  vimInput("0");
}

void test_teardown(void) {}

/* TODO: Get this test green */
/* MU_TEST(insert_count) { */
/*   vimInput("5"); */
/*   vimInput("i"); */
/*   vimInput("a"); */
/*   vimInput("\033"); */

/*   char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine()); */
/*   printf("LINE: %s\n", line); */
/*   mu_check(strcmp(line, "aaaaaThis is the first line of a test file") == 0);
 */
/* } */

MU_TEST(insert_beginning)
{
  vimInput("I");
  vimInput("a");
  vimInput("b");
  vimInput("c");

  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());
  printf("LINE: %s\n", line);
  mu_check(strcmp(line, "abcThis is the first line of a test file") == 0);
}

MU_TEST(insert_cr)
{
  vimInput("I");
  vimInput("a");
  vimInput("b");
  vimInput("c");
  vimKey("<CR>");

  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());
  mu_check(strcmp(line, "This is the first line of a test file") == 0);

  char_u *prevLine = vimBufferGetLine(curbuf, vimCursorGetLine() - 1);
  mu_check(strcmp(prevLine, "abc") == 0);
}

MU_TEST(insert_prev_line)
{
  vimInput("O");
  vimInput("a");
  vimInput("b");
  vimInput("c");
  mu_check(vimCursorGetLine() == 1);

  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());

  printf("LINE: %s\n", line);

  mu_check(strcmp(line, "abc") == 0);
}

MU_TEST(insert_next_line)
{
  vimInput("o");
  vimInput("a");
  vimInput("b");
  vimInput("c");

  mu_check(vimCursorGetLine() == 2);

  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());

  printf("LINE: %s\n", line);

  mu_check(strcmp(line, "abc") == 0);
}
MU_TEST(insert_end)
{
  vimInput("A");
  vimInput("a");
  vimInput("b");
  vimInput("c");
  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());

  printf("LINE: %s\n", line);

  mu_check(strcmp(line, "This is the first line of a test fileabc") == 0);
}

MU_TEST(insert_changed_ticks)
{
  buf_T *buf = vimBufferOpen("collateral/curswant.txt", 1, 0);

  int initialVersion = vimBufferGetLastChangedTick(buf);
  int newVersion;
  vimInput("i");
  newVersion = vimBufferGetLastChangedTick(buf);
  mu_check(newVersion == initialVersion);

  vimInput("a");
  newVersion = vimBufferGetLastChangedTick(buf);
  mu_check(newVersion == initialVersion + 1);

  vimInput("b");
  newVersion = vimBufferGetLastChangedTick(buf);
  mu_check(newVersion == initialVersion + 2);

  vimInput("c");
  newVersion = vimBufferGetLastChangedTick(buf);
  mu_check(newVersion == initialVersion + 3);
}

/* Ctrl_v inserts a character literal */
MU_TEST(insert_mode_ctrlv)
{
  vimInput("O");

  // Character literal mode
  vimKey("<c-v>");

  vimInput("1");
  vimInput("2");
  vimInput("6");

  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());

  printf("LINE: %s\n", line);
  mu_check(strcmp(line, "~") == 0);
}

MU_TEST(insert_mode_ctrlv_no_digit)
{
  vimInput("O");

  // Character literal mode
  vimKey("<c-v>");

  // Jump out of character literal mode by entering a non-digit character
  vimInput("a");

  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());

  printf("LINE: %s\n", line);
  mu_check(strcmp(line, "a") == 0);
}

MU_TEST(insert_mode_ctrlv_newline)
{
  vimInput("O");

  // Character literal mode
  vimKey("<c-v>");

  // Jump out of character literal mode by entering a non-digit character
  vimKey("<cr>");

  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());
  mu_check(line[0] == 13);
}

MU_TEST(insert_mode_utf8)
{
  vimInput("O");

  // Character literal mode
  vimInput("κόσμε");

  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());
  printf("LINE: %s\n", line);
  mu_check(strcmp(line, "κόσμε") == 0);
}

// Regression test for onivim/oni2#1720
MU_TEST(insert_mode_utf8_special_byte)
{
  vimInput("O");

  u_char input[4];
  input[0] = 232;
  input[1] = 128;
  input[2] = 133;
  input[3] = 0;

  vimInput(input);

  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());
  printf("LINE: %s\n", line);
  mu_check(strcmp(line, input) == 0);
}

MU_TEST(insert_mode_arrow_breaks_undo)
{
  int initialLineCount = vimBufferGetLineCount(curbuf);

  // Add a line above...
  vimInput("O");

  // Type a, left arrow, b, but join undo
  vimInput("a");
  vimKey("<left>");
  vimInput("b");

  mu_check(vimBufferGetLineCount(curbuf) == initialLineCount + 1);
  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());
  mu_check(strcmp(line, "ba") == 0);

  // Undoing should undo edit past arrow key being pressed,
  // default vim behavior.
  vimKey("<esc>");
  vimInput("u");
  char_u *lineAfterUndo = vimBufferGetLine(curbuf, vimCursorGetLine());
  mu_check(strcmp(lineAfterUndo, "a") == 0);
  mu_check(vimBufferGetLineCount(curbuf) == initialLineCount + 1);
}

MU_TEST(insert_mode_arrow_key_join_undo)
{
  int initialLineCount = vimBufferGetLineCount(curbuf);

  // Add a line above...
  vimInput("O");

  // Type a, left arrow, b, but join undo
  vimInput("a");

  // <C-g>U joins the undo for left/right arrow
  vimKey("<C-g>");
  vimInput("U");

  // ...and then use arrow
  vimKey("<left>");
  vimInput("b");

  mu_check(vimBufferGetLineCount(curbuf) == initialLineCount + 1);
  char_u *line = vimBufferGetLine(curbuf, vimCursorGetLine());
  mu_check(strcmp(line, "ba") == 0);

  // Undoing should undo entire edit
  vimKey("<esc>");
  vimInput("u");
  mu_check(vimBufferGetLineCount(curbuf) == initialLineCount);
}

MU_TEST_SUITE(test_suite)
{
  MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

  /* MU_RUN_TEST(insert_count); */
  MU_RUN_TEST(insert_prev_line);
  MU_RUN_TEST(insert_next_line);
  MU_RUN_TEST(insert_beginning);
  MU_RUN_TEST(insert_cr);
  MU_RUN_TEST(insert_end);
  MU_RUN_TEST(insert_changed_ticks);
  MU_RUN_TEST(insert_mode_ctrlv);
  MU_RUN_TEST(insert_mode_ctrlv_no_digit);
  MU_RUN_TEST(insert_mode_ctrlv_newline);
  MU_RUN_TEST(insert_mode_utf8);
  MU_RUN_TEST(insert_mode_utf8_special_byte);
  MU_RUN_TEST(insert_mode_arrow_breaks_undo);
  MU_RUN_TEST(insert_mode_arrow_key_join_undo);
}

int main(int argc, char **argv)
{
  vimInit(argc, argv);

  win_setwidth(5);
  win_setheight(100);

  vimBufferOpen("collateral/testfile.txt", 1, 0);

  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  MU_RETURN();
}
