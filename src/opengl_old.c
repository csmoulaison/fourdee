for(int32_t i = 0; i < grid_volume; i++)
{
	int32_t unit_index = i % grid_length;
	int32_t row_index = (i % grid_area) / grid_length;
	int32_t slice_index = i / grid_volume;

	int32_t x =
		  (unit_index  * x_positive_term + (grid_length - 1 - unit_index)  * x_negative_term) * x_unit_term
		+ (row_index   * x_positive_term + (grid_length - 1 - row_index)   * x_negative_term) * x_row_term
		+ (slice_index * x_positive_term + (grid_length - 1 - slice_index) * x_negative_term) * x_slice_term;
	int32_t y =
		  (unit_index  * y_positive_term + (grid_length - 1 - unit_index)  * y_negative_term) * y_unit_term
		+ (row_index   * y_positive_term + (grid_length - 1 - row_index)   * y_negative_term) * y_row_term
		+ (slice_index * y_positive_term + (grid_length - 1 - slice_index) * y_negative_term) * y_slice_term;
	int32_t z =
		  (unit_index  * z_positive_term + (grid_length - 1 - unit_index)  * z_negative_term) * z_unit_term
		+ (row_index   * z_positive_term + (grid_length - 1 - row_index)   * z_negative_term) * z_row_term
		+ (slice_index * z_positive_term + (grid_length - 1 - slice_index) * z_negative_term) * z_slice_term;

	instance_to_voxel_map[i] = z * grid_area + y * grid_length + x;
}
