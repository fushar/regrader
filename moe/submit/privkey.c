/*
 *  This is a trivial private key generator using a less secure
 *  random generator (/dev/urandom). This should be safe enough
 *  for the short-lived contest keys and it helps us to avoid
 *  spending hours by generating super-safe random numbers.
 *
 *  (c) 2007 Martin Mares <mj@ucw.cz>
 */

#include "ucw/lib.h"

#include <stdio.h>
#include <sys/types.h>
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include <gcrypt.h>

int main(void)
{
  gnutls_x509_privkey key;
  int err;

  gnutls_global_init();
  gcry_control(GCRYCTL_ENABLE_QUICK_RANDOM);
  err = gnutls_x509_privkey_init(&key);
  if (err < 0)
    die("privkey_init: %s", gnutls_strerror(err));
  err = gnutls_x509_privkey_generate(key, GNUTLS_PK_RSA, 1024, 0);
  if (err < 0)
    die("privkey_generate: %s", gnutls_strerror(err));

  byte buf[32768];
  size_t size = sizeof(buf);
  err = gnutls_x509_privkey_export(key, GNUTLS_X509_FMT_PEM, buf, &size);
  if (err < 0)
    die("privkey_export: %s", gnutls_strerror(err));
  puts(buf);

  return 0;
}
