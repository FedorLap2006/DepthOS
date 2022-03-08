#include <depthos/console.h>
#include <depthos/keyboard.h>

char shell_buffer[1024];
int shell_buffer_idx = 0;
char shell_playback_buffer[1024];
int shell_playback_buffer_idx = 0;
bool shell_executing;

const char user[64] = "root";
const char hostname[64] = "depthos";

void command_prompt() {
  console_write_color(user, -1, PINK_COLOR);
  console_putchar_color('@', -1, GREEN_COLOR);
  console_write_color(hostname, -1, BROWN_COLOR);
  console_putchar_color('#', -1, GREEN_COLOR);
  console_putchar(' ');
}

char command_argv[1024][256];
int command_argc = 0;
void execute_command() {
  shell_executing = true;

  command_argc = strsplt(shell_buffer, &command_argv[0][0], 256, 0, ' ');
  if (!command_argc) {
    return;
  }

  if (!strcmp(command_argv[0], "uname")) {
    // printk("===   ===  ==== ====== |  | \n");
    // printk("|  |  |==  |__/   ||   |==| \n");
    // printk("===   ===  =      ||   |\n");
    // printk("______           _   _     _____ _____\n");
    // printk("|  _  \         | | | |   |  _  /  ___|\n");
    // printk("| | | |___ _ __ | |_| |__ | | | \ `--.\n");
    // printk("| | | / _ \ '_ \| __| '_ \| | | |`--. \\\n");
    // printk("| |/ /  __/ |_) | |_| | | \ \_/ /\__/ /\n");
    // printk("|___/ \___| .__/ \__|_| |_|\___/\____/\n");
    // printk("          | |                          \n");
    // printk("          |_|                          \n");

    /*printk("
    \
    ██████╗ ███████╗██████╗ ████████╗██╗  ██╗ ██████╗ ███████╗\n	\
    ██╔══██╗██╔════╝██╔══██╗╚══██╔══╝██║  ██║██╔═══██╗██╔════╝\n	\
    ██║  ██║█████╗  ██████╔╝   ██║   ███████║██║   ██║███████╗\n	\
    ██║  ██║██╔══╝  ██╔═══╝    ██║   ██╔══██║██║   ██║╚════██║\n	\
    ██████╔╝███████╗██║        ██║   ██║  ██║╚██████╔╝███████║\n	\
    ╚═════╝ ╚══════╝╚═╝        ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚══════╝\n	\
                                                                      \
                      ");*/

    const char *text = command_argc > 1 && !strcmp(command_argv[1], "-a")
                           ? "DepthOS depthos 1.0.0 2019"
                           : "DepthOS";
    console_write_color(text, BLACK_COLOR, GREEN_COLOR);
    console_putchar('\n');
  } else if (!strcmp(command_argv[0], "setuser")) {
    if (command_argc < 2)
      console_write_color("No user specified", BLACK_COLOR, RED_COLOR);
    else
      strcpy(user, command_argv[1]);
  } else if (!strcmp(command_argv[0], "shutdown") ||
             !strcmp(command_argv[0], "exit")) {
    // TODO: emulator detection and ACPI
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);
    outw(0x4004, 0x3400);
  } else if (!strcmp(command_argv[0], "clear")) {
    console_clear();
  } else if (!strcmp(command_argv[0], "hostname")) {
    if (command_argc < 2)
      console_write_color("No hostname specified", BLACK_COLOR, RED_COLOR);
    else
      strcpy(hostname, command_argv[1]);

  } else if (!strcmp(command_argv[0], "help")) {
#define COMMAND(c, desc) printk("%-10s - %s\n", #c, desc);
    COMMAND(uname, "Output OS version and username")
    COMMAND(setuser, "Change username")
    COMMAND(clear, "Clear console")
    COMMAND(shutdown, "Shutdown the OS")
    COMMAND(clear, "Clear the screen")
#undef COMMAND
  }

  console_write(shell_playback_buffer);
  // NOTE: shell_executing should be set to false before resetting the index,
  // so if the keyboard interrupt gets called at this line it will write into
  // the stdout instead of playback, which was already displayed.
  shell_executing = false;
  shell_playback_buffer_idx = 0;
}

extern void standard_keycode_handler(int keycode);
void shell_keyhandler(int keycode) {
  if (shell_executing) {
    shell_playback_buffer[shell_playback_buffer_idx++] = keycode;
    standard_keycode_handler(keycode);
    return;
  }

  switch (keycode) {
  case KEY_RETURN:
    shell_buffer[shell_buffer_idx] = 0;
    shell_buffer_idx = 0;
    console_putchar('\n');
    execute_command();
    console_putchar('\n');
    command_prompt();
    break;
  case KEY_BACKSPACE:
    if (shell_buffer_idx > 0) {
      shell_buffer_idx--;
      standard_keycode_handler(keycode);
    }
    break;
  default:
    shell_buffer[shell_buffer_idx++] = keycode;
    standard_keycode_handler(keycode);
  }
}

void shell_eventloop() {
  __kb_driver_set_handler(shell_keyhandler);
  command_prompt();
}