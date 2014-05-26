#! /usr/bin/env python

import numpy as np


def flag(cube,regions):
	
	if regions:
		print 'Start flagging cube'
		dim = len(cube.shape)
		try:
			for region in regions:
				for i in range(0,len(region)/2):
					if region[2*i+1] == '':
						region[2*i+1] = cube.shape[dim-i-1]-1
				if len(region) == 2:
					cube[0,region[2]:region[3],region[0]:region[1]] = np.nan
				else:
					cube[region[4]:region[5],region[2]:region[3],region[0]:region[1]] = np.nan
			print 'Cube has been flagged'

		except:
			print 'WARNING: Flagging did not succeed. Please chech the dimensions of your cube and filters!'
		
			
	return cube
	
