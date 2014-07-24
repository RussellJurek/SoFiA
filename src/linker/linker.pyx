# distutils: language = c++
# distutils: sources = RJJ_ObjGen_CreateObjs.cpp RJJ_ObjGen_MemManage.cpp RJJ_ObjGen_DetectDefn.cpp RJJ_ObjGen_ThreshObjs.cpp RJJ_ObjGen_Dmetric.cpp

# cython: boundscheck = False
# cython: wraparound = False
# cython: cdivision = True

cimport numpy as np
import numpy as np
from libcpp.vector cimport vector

cdef extern from "RJJ_ObjGen.h":
	cdef int CreateObjects( float * data_vals, int * flag_vals, 
							int size_x, int size_y, int size_z, 
							int chunk_x_start, int chunk_y_start, int chunk_z_start, 
							int mergeX, int mergeY, int mergeZ, 
							int minSizeX, int minSizeY, int minSizeZ, 
							int min_v_size, 
							float intens_thresh_min, float intens_thresh_max, 
							int flag_value, 
							int start_obj, vector[object_props *] & detections, vector[int] & obj_ids, vector[int] & check_obj_ids, int obj_limit, 
							int max_x_val, int max_y_val, int max_z_val, 
							int ss_mode,
							int * data_metric, int * xyz_order)

	cdef void InitObjGen(vector[object_props *] & detections, int & NOobj, int obj_limit, vector[int] & obj_ids, vector[int] & check_obj_ids, int *& data_metric, int *& xyz_order)
	cdef void FreeObjGen(vector[object_props *] & detections, int *& data_metric, int *& xyz_order)
	cdef void ThresholdObjs(vector[object_props *] & detections, int NOobj, int obj_limit, int minSizeX, int minSizeY, int minSizeZ, int min_v_size, float intens_thresh_min, float intens_thresh_max, int min_LoS_count)
	cdef void CreateMetric( int * data_metric, int * xyz_order, int size_x, int size_y, int size_z)

	cdef cppclass object_props:
		
		void CalcProps()
		
		# Bounding box
		int GetRAmin()
		int GetRAmax()
		int GetDECmin()
		int GetDECmax()
		int GetFREQmin()
		int GetFREQmax()
		
		# Center of mass
		float GetRAi()
		float GetDECi()
		float GetFREQi()
		
		# Geometric center
		float GetRA()
		float GetDEC()
		float GetFREQ()
		
		# Intensity extrema
		float GetMinI()
		float GetMaxI()
		
		# Total intensity
		float GetTI()
		
		# Number of voxels
		int ShowVoxels()
		
		# Sparse representation functions
		int Get_srep_size(int index)
		int Get_srep_grid(int index)
		int Get_srep_strings(int index)

def link_objects(data, mask, mergeX = 0, mergeY = 0, mergeZ = 0, minSizeX = 1, minSizeY = 1, minSizeZ = 1, min_LOS = 1):
	"""
	Given a data cube and a binary mask, create a labeled version of the mask.
	In addition, close groups of objects can be linked together, so they have the same label.
	
	
	Parameters
	----------
	
	data : array
		The data
	
	mask : array
		The binary mask
		
	mergeX, mergeY, mergeZ : int
		The merging length in all three dimensions
		
	minSizeX, minSizeY, minSizeZ : int
		The minimum size objects can have in all three dimensions
		
	min_LOS : int
		The mininum pixel-extent in the spatial (x,y) domain of the data a source must have

	ss_mode : int
		The linking method. A value of 1 uses a cuboid and all other values use an elliptical cylinder.
		
		
	Returns
	-------
	
	objects : list
		Lists of lists. Order of parameters:
			Geometric Center X,Y,Z
			Center-Of-Mass X,Y,X
			Bounding Box Xmin, Xmax, Ymin, Ymax, Zmin, Zmax
			Flux Min, Max, Total
		
		The Bounding box are defined is such a way that they can be
		used as slices, i.e. data[Zmin:Zmax]
			
	
	mask : array
		The labeled and linked integer mask
	"""
	return _link_objects(data.astype(np.single, copy = False), mask.astype(np.intc, copy = False), mergeX, mergeY, mergeZ, minSizeX, minSizeY, minSizeZ, min_LOS)

cdef _link_objects(np.ndarray[dtype = float, ndim = 3] data, np.ndarray[dtype = int, ndim = 3] mask,
				   int mergeX = 3, int mergeY = 3, int mergeZ = 5,
				   int minSizeX = 1, int minSizeY = 1, int minSizeZ = 1,
				   int min_LOS = 1):
		
	cdef int i, x, y, z, g, g_start, g_end
	cdef int obj_id = 0
	cdef int obj_batch
	
	cdef int size_x = data.shape[2]
	cdef int size_y = data.shape[1]
	cdef int size_z = data.shape[0]
			
	# Convert binary mask to conform with the object code
	for z in range(size_z):
		for y in range(size_y):
			for x in range(size_x):
				if mask[z,y,x] > 0:
					mask[z,y,x] = -1
				else:
					mask[z,y,x] = -99
	
	# Define arrays storing datacube geometry metric
	cdef int * data_metric
	cdef int * xyz_order	

	# Chunking is disabled for this interface
	cdef int chunk_x_start = 0
	cdef int chunk_y_start = 0
	cdef int chunk_z_start = 0
	
	# Define min_v_size; does not seem to be used
	cdef int min_v_size = 0
	
	# Define intensity thresholds
	cdef float intens_thresh_min = -1E10
	cdef float intens_thresh_max = 1E10
	
	# Define value that is used to mark sources in the mask
	cdef int flag_val = -1
	
	# Specify size of allocated object groups 
	cdef int obj_limit = 1000
	
	# Object and ID arrays; will be written to by the function
	cdef vector[object_props *] detections
	cdef vector[int] obj_ids
	cdef int NO_obj_ids
	cdef vector[int] check_obj_ids
	cdef int NOobj = 0
	
	# Define linking style: 1 for Rectangle, else ellipse
	cdef int ss_mode = 0
	
	# Inititalize object pointers
	InitObjGen(detections, NOobj, obj_limit, obj_ids, check_obj_ids, data_metric, xyz_order)

	# Define the mapping of RA, Dec and frequency to the datacube's first three axes. RA, Dec and freq. --> 1, 2, 3: use the default values of x=RA=1 y=Dec=2 z=freq.=3
	xyz_order[0] = 1
	xyz_order[1] = 2
	xyz_order[2] = 3

	# create metric for accessing this data chunk in arbitrary x,y,z order
	CreateMetric(data_metric, xyz_order, size_x, size_y, size_z)
		
	# Create and threshold objects
	NOobj = CreateObjects(<float *> data.data, <int *> mask.data, size_x, size_y, size_z, chunk_x_start, chunk_y_start, chunk_z_start, mergeX, mergeY, mergeZ, minSizeX, minSizeY, minSizeZ, min_v_size, intens_thresh_min, intens_thresh_max, flag_val, NOobj, detections, obj_ids, check_obj_ids, obj_limit, size_x, size_y, size_z, ss_mode, data_metric, xyz_order)
	ThresholdObjs(detections, NOobj, obj_limit, minSizeX, minSizeY, minSizeZ, min_v_size, intens_thresh_min, intens_thresh_max, min_LOS)

	# Reset output mask
	for z in range(size_z):
		for y in range(size_y):
			for x in range(size_x):
				mask[z,y,x] = 0
	
	# Create Python list `objects' from C++ vector `detections' and re-label mask with final, sequential IDs
	objects = []
		
	for i in range(NOobj):		
		
		# calculate batch number for this object --- which group of objects does it belong to
		obj_batch = i / obj_limit

		if detections[obj_batch][i - (obj_batch * obj_limit)].ShowVoxels() >= 1:		
	
			obj_id += 1
			
			obj = []
			obj.append(obj_id)
		
			detections[obj_batch][i - (obj_batch * obj_limit)].CalcProps()
			
			# Geometric center
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetRA())
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetDEC())
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetFREQ())
			
			# Center of mass
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetRAi())
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetDECi())
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetFREQi())
			
			# Adding 1 to the maxima to aid slicing/iteration
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetRAmin())
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetRAmax() + 1)
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetDECmin())
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetDECmax() + 1)
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetFREQmin())
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetFREQmax() + 1)
			
			# Number of voxels
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].ShowVoxels())
			
			# Min/Max/Total flux
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetMinI())
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetMaxI())
			obj.append(detections[obj_batch][i - (obj_batch * obj_limit)].GetTI())
		
			objects.append(obj)
			
			for y in range(detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(2), detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(3) + 1):
				for x in range(detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(0), detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(1) + 1):
					
					g_start = detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_grid(((((y - detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(2)) * (detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(1) - detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(0) + 1)) + x - detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(0))))
					g_end = detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_grid(((((y - detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(2)) * (detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(1) - detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(0) + 1)) + x - detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_size(0) + 1)))
					
					for g in range(g_start, g_end):
						mask[detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_strings((2 * g)) : detections[obj_batch][i - (obj_batch * obj_limit)].Get_srep_strings((2 * g) + 1) + 1, y, x] = obj_id
						
	# Free memory for object pointers
	FreeObjGen(detections, data_metric, xyz_order)

	return objects, mask
	
	
	
	
	
	
	
	
	
	