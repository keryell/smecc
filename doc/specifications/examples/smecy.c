//#pragma smecy remap arg(2, in, <some mapping matrix>)
void
SMECY_remap_int1D_to_int2D(int size_in_0, int in[size_in_0],
			   int size_out_0, int size_out_1,
			   int offset_out_0, int offset_out_1,
			   int window_out_0, int window_out_1,
			   int out[size_out_0][size_out_1]) {
#pragma omp parallel for
  for(int i_0 = 0; i_0 < window_out_0; i_0++)
    for(int i_1 = 0; i_1 < window_out_1; i_1++)
      out[offset_out_0 + i_0][offset_out_1 + i_1] = in[window_out_1*i_0 + i_1];
}

void
SMECY_remap_int2D_to_int1D(int size_out_0, int size_out_1,
			   int offset_out_0, int offset_out_1,
			   int window_out_0, int window_out_1,
			   int out[size_out_0][size_out_1],
			   int size_in_0, int in[size_in_0]) {
#pragma omp parallel for
  for(int i_0 = 0; i_0 < window_out_0; i_0++)
    for(int i_1 = 0; i_1 < window_out_1; i_1++)
      in[window_out_1*i_0 + i_1] = out[offset_out_0 + i_0][offset_out_1 + i_1];
}
