#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>

#include "zfs_abdstats.h"

void
zfs_abdstats_fetch(zfs_abdstats_t *abdstats, regex_t *rgx_row)
{
        int len_mn, len_mv, nmatch = 3;
        regmatch_t pmatch[3];
        char *line, *mname, *mval;
	char *fname = "/proc/spl/kstat/zfs/abdstats";
	FILE *fp;
        size_t len = 0;

        fp = fopen(fname, "r");
	if (fp != NULL) {
		while (getline(&line, &len, fp) != -1) {
                        if (regexec(rgx_row, line, nmatch, pmatch, 0) == 0) {
                                len_mn = pmatch[1].rm_eo - pmatch[1].rm_so + 1;
                                len_mv = pmatch[2].rm_eo - pmatch[2].rm_so + 1;
                                mname = (char *) malloc((size_t) (len_mn + 1) * sizeof(char));
                                mval  = (char *) malloc((size_t) (len_mv + 1) * sizeof(char));
                                strncpy(mname, line + pmatch[1].rm_so, len_mn);
                                strncpy(mval,  line + pmatch[2].rm_so, len_mv);
                                mname[len_mn] = '\0';
                                mval[len_mv] = '\0';
				if (strcmp(mname, "struct_size")) abdstats->struct_size = atoi(mval);
				else if (strcmp(mname, "linear_cnt")) abdstats->linear_cnt = atoi(mval);
				else if (strcmp(mname, "linear_data_size")) abdstats->linear_data_size = atoi(mval);
				else if (strcmp(mname, "scatter_cnt")) abdstats->scatter_cnt = atoi(mval);
				else if (strcmp(mname, "scatter_data_size")) abdstats->scatter_data_size = atoi(mval);
				else if (strcmp(mname, "scatter_chunk_waste")) abdstats->scatter_chunk_waste = atoi(mval);
				else if (strcmp(mname, "scatter_order_0")) abdstats->scatter_order_0 = atoi(mval);
				else if (strcmp(mname, "scatter_order_1")) abdstats->scatter_order_1 = atoi(mval);
				else if (strcmp(mname, "scatter_order_2")) abdstats->scatter_order_2 = atoi(mval);
				else if (strcmp(mname, "scatter_order_3")) abdstats->scatter_order_3 = atoi(mval);
				else if (strcmp(mname, "scatter_order_4")) abdstats->scatter_order_4 = atoi(mval);
				else if (strcmp(mname, "scatter_order_5")) abdstats->scatter_order_5 = atoi(mval);
				else if (strcmp(mname, "scatter_order_6")) abdstats->scatter_order_6 = atoi(mval);
				else if (strcmp(mname, "scatter_order_7")) abdstats->scatter_order_7 = atoi(mval);
				else if (strcmp(mname, "scatter_order_8")) abdstats->scatter_order_8 = atoi(mval);
				else if (strcmp(mname, "scatter_order_9")) abdstats->scatter_order_9 = atoi(mval);
				else if (strcmp(mname, "scatter_order_10")) abdstats->scatter_order_10 = atoi(mval);
				else if (strcmp(mname, "scatter_page_multi_chunk")) abdstats->scatter_page_multi_chunk = atoi(mval);
				else if (strcmp(mname, "scatter_page_multi_zone")) abdstats->scatter_page_multi_zone = atoi(mval);
				else if (strcmp(mname, "scatter_page_alloc_retry")) abdstats->scatter_page_alloc_retry = atoi(mval);
				else if (strcmp(mname, "scatter_sg_table_retry")) abdstats->scatter_sg_table_retry = atoi(mval);
                        }
                        free(mname);
                        free(mval);
                }
        }
        fclose(fp);
}
