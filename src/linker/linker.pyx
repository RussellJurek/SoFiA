# distutils: language = c++
# distutils: sources = RJJ_ObjGen_CreateObjs.cpp RJJ_ObjGen_MemManage.cpp RJJ_ObjGen_DetectDefn.cpp RJJ_ObjGen_ThreshObjs.cpp

# cython: boundscheck = False
# cython: wraparound = False
# cython: cdivision = True

cimport numpy as np
import numpy as np

cdef extern from "RJJ_ObjGen.h":
	cdef int CreateObjects( float * data_vals, int * flag_vals, 
							int size_x, int size_y, int size_z, 
							int chunk_x_start, int chunk_y_start, int chunk_z_start, 
							int mergeX, int mergeY, int mergeZ, 
							int minSizeX, int minSizeY, int minSizeZ, 
							int min_v_size, 
							float intens_thresh_min, float intens_thresh_max, 
							int flag_value, 
							int start_obj, object_props ** detections, int * obj_ids, int& NO_obj_ids, int * check_obj_ids, int& NO_check_obj_ids, int obj_limit, int obj_batch_limit, 
							int max_x_val, int max_y_val, int max_z_val, 
							int ss_mode)
	
	cdef void InitObjGen(object_props **& detections, int& NOobj, int obj_limit, int obj_batch_limit, int *& obj_ids, int& NO_obj_ids, int *& check_obj_ids)
	cdef void FreeObjGen(object_props **& detections, int NOobj, int obj_batch_limit, int *& obj_ids, int *& check_obj_ids)
	cdef void ThresholdObjs(object_props ** detections, int NOobj, int obj_limit, int minSizeX, int minSizeY, int minSizeZ, int min_v_size, float intens_thresh_min, float intens_thresh_max, int min_LoS_count)
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
	
	cdef object_props X
	
	cdef int size_x = data.shape[2]
	cdef int size_y = data.shape[1]
	cdef int size_z = data.shape[0]
		
	# Create output / working mask
	cdef np.ndarray[dtype = int, ndim = 3] final_mask = mask.copy()
	
	# Convert binary mask to conform with the object code
	for x in range(size_x):
		for y in range(size_y):
			for z in range(size_z):
				if final_mask[z,y,x] > 0:
					final_mask[z,y,x] = -1
				else:
					final_mask[z,y,x] = -99
	
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
	
	# Object limits
	cdef int obj_limit = 10000
	cdef int obj_batch_limit = 10000
	
	# Object and ID arrays; will be written to by the function
	cdef object_props **detections
	cdef int *obj_ids
	cdef int NO_obj_ids
	cdef int *check_obj_ids
	cdef int NO_check_obj_ids = 0
	cdef int NOobj = 0
	
	# Define linking style: 1 for Rectangle, else ellipse
	cdef int ss_mode = 0
	
	# Inititalize object pointers
	InitObjGen(detections, NOobj, obj_limit, obj_batch_limit, obj_ids, NO_obj_ids, check_obj_ids)
		
	# Create and threshold objects
	NOobj = CreateObjects(<float *> data.data, <int *> final_mask.data, size_x, size_y, size_z, chunk_x_start, chunk_y_start, chunk_z_start, mergeX, mergeY, mergeZ, minSizeX, minSizeY, minSizeZ, min_v_size, intens_thresh_min, intens_thresh_max, flag_val, NOobj, detections, obj_ids, NO_obj_ids, check_obj_ids, NO_check_obj_ids, obj_limit, obj_batch_limit, size_x, size_y, size_z, ss_mode)	
	ThresholdObjs(detections, NOobj, obj_limit, minSizeX, minSizeY, minSizeZ, min_v_size, intens_thresh_min, intens_thresh_max, min_LOS)
	
	# Reset output mask
	for x in range(size_x):
		for y in range(size_y):
			for z in range(size_z):
				final_mask[z,y,x] = 0
	
	objects = []
		
	for i in range(NOobj):
		
		X = detections[0][i]
		
		if X.ShowVoxels() >= 1:
			
			obj_id += 1
			
			obj = []
			obj.append(obj_id)
		
			X.CalcProps()
			
			# Geometric center
			obj.append(X.GetRA())
			obj.append(X.GetDEC())
			obj.append(X.GetFREQ())
			
			# Center of mass
			obj.append(X.GetRAi())
			obj.append(X.GetDECi())
			obj.append(X.GetFREQi())
			
			# Adding 1 to the maxima to aid slicing/iteration
			obj.append(X.GetRAmin())
			obj.append(X.GetRAmax() + 1)
			obj.append(X.GetDECmin())
			obj.append(X.GetDECmax() + 1)
			obj.append(X.GetFREQmin())
			obj.append(X.GetFREQmax() + 1)
			
			# Number of voxels
			obj.append(X.ShowVoxels())
			
			# Min/Max/Total flux
			obj.append(X.GetMinI())
			obj.append(X.GetMaxI())
			obj.append(X.GetTI())
		
			objects.append(obj)
			
			for x in range(X.Get_srep_size(0), X.Get_srep_size(1) + 1):
				for y in range(X.Get_srep_size(2), X.Get_srep_size(3) + 1):
					
					g_start = X.Get_srep_grid(((((y - X.Get_srep_size(2)) * (X.Get_srep_size(1) - X.Get_srep_size(0) + 1)) + x - X.Get_srep_size(0))))
					g_end = X.Get_srep_grid(((((y - X.Get_srep_size(2)) * (X.Get_srep_size(1) - X.Get_srep_size(0) + 1)) + x - X.Get_srep_size(0) + 1)))
					
					for g in range(g_start, g_end):
						final_mask[X.Get_srep_strings((2 * g)) : X.Get_srep_strings((2 * g) + 1) + 1, y, x] = obj_id
						
	# Free memory for object pointers
	FreeObjGen(detections, NOobj, obj_batch_limit, obj_ids, check_obj_ids)
		
	return objects, final_mask
	
	
	
	
	
	
	
	
	
	