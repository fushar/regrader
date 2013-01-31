/*
 *	The UCW Library -- Threading Helpers
 *
 *	(c) 2006 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "ucw/lib.h"
#include "ucw/threads.h"
#include "ucw/conf.h"

uns ucwlib_thread_stack_size = 65556;

static struct cf_section threads_config = {
  CF_ITEMS {
    CF_UNS("DefaultStackSize", &ucwlib_thread_stack_size),
    CF_END
  }
};

static void CONSTRUCTOR
ucwlib_threads_conf_init(void)
{
  cf_declare_section("Threads", &threads_config, 0);
}
