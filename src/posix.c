/*
 * OPENSTEP 4.x lacks the POSIX libraries (even though is has the headers) for
 * POSIX support. I thus sourced the minimum needed functions from this URL:
 *
 * http://www.math.unl.edu/~rdieter1/OpenStep/Developer/PortingTips/posix1.txt
 */

#ifdef NeXT

#include <libc.h> /* for getwd */
#include <string.h> /* for strncpy */
#include <signal.h> /* for sigvec */
#include <sys/param.h> /* for MAXPATHLEN */

int sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
        struct sigvec vec, ovec;
        int st;

        vec.sv_handler = act->sa_handler;
        vec.sv_mask = act->sa_mask;
        vec.sv_flags = act->sa_flags;

        st = sigvec(sig, &vec, &ovec);

        if (oact) {
                oact->sa_handler = ovec.sv_handler;
                oact->sa_mask = ovec.sv_mask;
                oact->sa_flags = ovec.sv_flags;
        }

        return st;
}

char *getcwd(char *buf, size_t size)
{
	char pathname[MAXPATHLEN];

	 if (getwd(pathname) == NULL) {
		return NULL;
	}

	strncpy (buf, pathname, size);
	return buf;
}

#endif
