#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>

#include "menu.h"
#include "wayland.h"

static void
read_items (struct menu *menu)
{
  char buf[sizeof menu->input];
  while (fgets (buf, sizeof buf, stdin))
    {
      char *p = strchr (buf, '\n');
      if (p)
        {
          *p = '\0';
        }
      char *thumb_path = NULL;
      char *text = buf;
      if (strncmp (buf, "[img:", 5) == 0)
        {
          char *end = strchr (buf + 5, ']');
          if (end)
            {
              *end = '\0';
              thumb_path = strdup (buf + 5);
              text = end + 1;
              while (*text == ' ')
                text++;
            }
        }
      menu_add_item (menu, strdup (text));
      menu->items[menu->item_count - 1].thumb_path = thumb_path;
    }
}

static void
print_item (struct menu *menu, char *text, bool exit)
{
  puts (text);
  fflush (stdout);
  if (exit)
    {
      menu->exit = true;
    }
}

int
main (int argc, char *argv[])
{
  struct menu *menu = menu_create (print_item);
  menu_getopts (menu, argc, argv);
  read_items (menu);
  int status = menu_run (menu);
  menu_destroy (menu);
  return status;
}
