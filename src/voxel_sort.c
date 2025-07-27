void sort_voxels(int32_t* instance_to_voxel_map, uint32_t grid_length, uint32_t grid_area, uint32_t grid_volume, Vec3f cam_pos)
{
	// All these variables suffexed "_term" will be 0 or 1, and are used to
	// selectively terms we don't want in the final calculation.
	int32_t x_positive_term = 0;
	int32_t x_negative_term = 0;
	int32_t x_unit_term = 0;
	int32_t x_row_term = 0;
	int32_t x_slice_term = 0;

	int32_t y_positive_term = 0;
	int32_t y_negative_term = 0;
	int32_t y_unit_term = 0;
	int32_t y_row_term = 0;
	int32_t y_slice_term = 0;

	int32_t z_positive_term = 0;
	int32_t z_negative_term = 0;
	int32_t z_unit_term = 0;
	int32_t z_row_term = 0;
	int32_t z_slice_term = 0;

	float x_abs = fabs(cam_pos.x);
	float y_abs = fabs(cam_pos.y);
	float z_abs = fabs(cam_pos.z);

	if(z_abs > y_abs)
	{
		if(z_abs > x_abs)
		{
			z_slice_term = 1;
			if(x_abs > y_abs)
			{
				x_row_term = 1;
				y_unit_term = 1;
			}
			else
			{
				y_row_term = 1;
				x_unit_term = 1;
			}
		}
		else
		{
			x_slice_term = 1;
			z_row_term = 1;
			y_unit_term = 1;
		}
	}
	else // y_abs > z_abs
	{
		printf("z not slicin! z: %f, x: %f\n", z_abs, x_abs);
		if(y_abs > x_abs)
		{
			y_slice_term = 1;
			if(z_abs > x_abs)
			{
				z_row_term = 1;
				x_unit_term = 1;
			}
			else
			{
				x_row_term = 1;
				z_unit_term = 1;
			}
		}
		else
		{
			x_slice_term = 1;
			y_row_term = 1;
			z_unit_term = 1;
		}
	}

	if(cam_pos.x >= 0)
	{
		x_positive_term = 1;
	}
	else
	{
		x_negative_term = 1;
	}

	if(cam_pos.y >= 0)
	{
		y_positive_term = 1;
	}
	else
	{
		y_negative_term = 1;
	}

	if(cam_pos.z >= 0)
	{
		z_positive_term = 1;
	}
	else
	{
		z_negative_term = 1;
	}

	if(1==0) // DBG voxel terms
	{
		printf("\033[2J\033[H");
		printf("x_positive_term: %i\n", x_positive_term);
		printf("x_negative_term: %i\n", x_negative_term);
		printf("x_unit_term:     %i\n", x_unit_term);
		printf("x_row_term:      %i\n", x_row_term);
		printf("x_slice_term:    %i\n", x_slice_term);
		printf("\n");
		printf("y_positive_term: %i\n", y_positive_term);
		printf("y_negative_term: %i\n", y_negative_term);
		printf("y_unit_term:     %i\n", y_unit_term);
		printf("y_row_term:      %i\n", y_row_term);
		printf("y_slice_term:    %i\n", y_slice_term);
		printf("\n");
		printf("z_positive_term: %i\n", z_positive_term);
		printf("z_negative_term: %i\n", z_negative_term);
		printf("z_unit_term:     %i\n", z_unit_term);
		printf("z_row_term:      %i\n", z_row_term);
		printf("z_slice_term:    %i\n", z_slice_term);
	}

	for(int32_t i = 0; i < grid_volume; i++)
	{
		int32_t unit_index = i % grid_length;
		int32_t row_index = (i % grid_area) / grid_length;
		int32_t slice_index = i / grid_area;

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
}
