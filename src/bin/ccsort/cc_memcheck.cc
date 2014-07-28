/*
 *@BEGIN LICENSE
 *
 * PSI4: an ab initio quantum chemistry software package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *@END LICENSE
 */

/*! \file
    \ingroup CCSORT
    \brief Enter brief description of file here
*/
#include <cstdio>
#include <libdpd/dpd.h>
#include "Params.h"
#include "MOInfo.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace ccsort {

void cc_memcheck(void)
{
  unsigned long int irrep_size, size, h;
  dpdbuf4 Z;

  fprintf(outfile, "\n");

  if(params.ref == 0) {
    global_dpd_->buf4_init(&Z, 99, 0, 5, 5, 5, 5, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of <ab|cd> integrals: %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

    global_dpd_->buf4_init(&Z, 99, 0, 10, 5, 10, 5, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of <ia|bc> integrals: %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

    global_dpd_->buf4_init(&Z, 99, 0, 0, 5, 0, 5, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of tijab amplitudes:  %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

    fflush(outfile);
  }
  else if(params.ref == 1) {
    global_dpd_->buf4_init(&Z, 99, 0, 5, 5, 5, 5, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of <ab|cd> integrals: %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

    global_dpd_->buf4_init(&Z, 99, 0, 10, 5, 10, 5, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of <ia|bc> integrals: %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

    global_dpd_->buf4_init(&Z, 99, 0, 0, 5, 0, 5, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of tIjAb amplitudes:  %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

  }
  else if(params.ref == 2) {
    global_dpd_->buf4_init(&Z, 99, 0, 7, 7, 7, 7, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of <AB|CD> integrals: %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));
    global_dpd_->buf4_init(&Z, 99, 0, 17, 17, 17, 17, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of <ab|cd> integrals: %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));
    global_dpd_->buf4_init(&Z, 99, 0, 28, 28, 28, 28, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of <Ab|Cd> integrals: %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

    global_dpd_->buf4_init(&Z, 99, 0, 20, 5, 20, 5, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of <IA|BC> integrals: %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

    global_dpd_->buf4_init(&Z, 99, 0, 30, 15, 30, 15, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of <ia|bc> integrals: %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

    global_dpd_->buf4_init(&Z, 99, 0, 24, 28, 24, 28, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of <Ia|Bc> integrals: %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

    global_dpd_->buf4_init(&Z, 99, 0, 27, 29, 27, 29, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of <iA|bC> integrals: %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

    global_dpd_->buf4_init(&Z, 99, 0, 22, 28, 22, 28, 0, "Just a template");
    size = 0;
    for(h=0; h < moinfo.nirreps; h++) {
      irrep_size = (unsigned long int) Z.params->rowtot[h] * Z.params->coltot[h];
      size += irrep_size;
      fprintf(outfile, "\tSize of irrep %1lu of tIjAb amplitudes:  %10.3lf (MW) / %10.3lf (MB)\n",
              h, irrep_size/1e6, (irrep_size/1e6)*sizeof(double));
    }
    global_dpd_->buf4_close(&Z);
    fprintf(outfile, "\tTotal:                                %10.3lf (MW) / %10.3lf (MB)\n\n",
            size/1e6, (size/1e6)*sizeof(double));

  }

  fflush(outfile);
}

}} // namespace psi::ccsort
