/*
 *	UCW Library -- Rate Limiting based on the Token Bucket Filter
 *
 *	(c) 2009 Martin Mares <mj@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef _UCW_TBF_H_
#define _UCW_TBF_H_

/** A data structure describing a single TBF. **/
struct token_bucket_filter {
  double rate;				// Number of tokens received per second
  uns burst;				// Capacity of the bucket
  timestamp_t last_hit;			// Internal state...
  double bucket;
  uns drop_count;
};

/** Initialize the bucket. **/
void tbf_init(struct token_bucket_filter *f);

/**
 * Ask the filter to process a single event. Returns a negative number
 * if the event exceeds the rate (and should be dropped) and a non-negative
 * number if the event passes the filter.
 * The absolute value of the result is the number of dropped events
 * since the last passed event.
 **/
int tbf_limit(struct token_bucket_filter *f, timestamp_t now);

#endif
