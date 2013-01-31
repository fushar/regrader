/*
 *	UCW Library -- Universal Sorter: Two-Way Merge Module
 *
 *	(c) 2007 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

static void P(twoway_merge)(struct sort_context *ctx UNUSED, struct sort_bucket **ins, struct sort_bucket **outs)
{
  struct fastbuf *fin1, *fin2, *fout1, *fout2, *ftmp;
  P(key) kbuf1, kbuf2, kbuf3, kbuf4;
  P(key) *kin1 = &kbuf1, *kprev1 = &kbuf2, *kin2 = &kbuf3, *kprev2 = &kbuf4;
  P(key) *kout = NULL, *ktmp;
  int next1, next2, run1, run2;
  int comp;
  uns run_count = 0;

  fin1 = sbuck_read(ins[0]);
  next1 = P(read_key)(fin1, kin1);
  if (sbuck_have(ins[1]))
    {
      fin2 = sbuck_read(ins[1]);
      next2 = P(read_key)(fin2, kin2);
    }
  else
    {
      fin2 = NULL;
      next2 = 0;
    }
  fout1 = fout2 = NULL;

  run1 = next1, run2 = next2;
  while (next1 || next2)
    {
      if (!run1)
	comp = 1;
      else if (!run2)
	comp = -1;
      else
	comp = P(compare)(kin1, kin2);
      ktmp = (comp <= 0) ? kin1 : kin2;
      if (!kout || !(P(compare)(kout, ktmp) LESS 0))
	{
	  SWAP(fout1, fout2, ftmp);
	  if (unlikely(!fout1))
	    {
	      if (!fout2)
		fout1 = sbuck_write(outs[0]);
	      else if (outs[1])
		fout1 = sbuck_write(outs[1]);
	      else
		fout1 = fout2;
	    }
	  run_count++;
	}
#ifdef SORT_ASSERT_UNIQUE
      ASSERT(comp != 0);
#endif
      if (comp LESS 0)
	{
	  P(copy_data)(kin1, fin1, fout1);
	  SWAP(kin1, kprev1, ktmp);
	  next1 = P(read_key)(fin1, kin1);
	  run1 = next1 && (P(compare)(kprev1, kin1) LESS 0);
	  kout = kprev1;
	}
#ifdef SORT_UNIFY
      else if (comp == 0)
	{
	  P(key) *mkeys[] = { kin1, kin2 };
	  struct fastbuf *mfb[] = { fin1, fin2 };
	  P(copy_merged)(mkeys, mfb, 2, fout1);
	  SWAP(kin1, kprev1, ktmp);
	  next1 = P(read_key)(fin1, kin1);
	  run1 = next1 && (P(compare)(kprev1, kin1) LESS 0);
	  SWAP(kin2, kprev2, ktmp);
	  next2 = P(read_key)(fin2, kin2);
	  run2 = next2 && (P(compare)(kprev2, kin2) LESS 0);
	  kout = kprev2;
	}
#endif
      else
	{
	  P(copy_data)(kin2, fin2, fout1);
	  SWAP(kin2, kprev2, ktmp);
	  next2 = P(read_key)(fin2, kin2);
	  run2 = next2 && (P(compare)(kprev2, kin2) LESS 0);
	  kout = kprev2;
	}
      if (!run1 && !run2)
	{
	  run1 = next1;
	  run2 = next2;
	}
    }

  if (fout2 && fout2 != fout1)
    outs[1]->runs += run_count / 2;
  if (fout1)
    outs[0]->runs += (run_count+1) / 2;
}
