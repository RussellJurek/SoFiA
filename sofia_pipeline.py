#! /usr/bin/env python

# import default python libraries
import numpy as np
import sys, os
import string

# import source finding modules
sys.path.insert(0, os.environ['SOFIA_MODULE_PATH'])
from sofia import functions
from sofia import readoptions
from sofia import import_data
from sofia import sigma_cube
from sofia import pyfind
from sofia import addrel
from sofia import threshold_filter
from sofia import smooth_cube
from sofia import writemask
from sofia import writemoment
from sofia import linker
from sofia import store_xml
from sofia import store_ascii
from sofia import cubelets
from sofia import parametrisation
from sofia import wcs_coordinates
from sofia import flag_cube



# ---------------------------------
# ---- READ DEFAULT PARAMETERS ----
# ---------------------------------

print "\n--- SoFiA: Reading default parameters ---"
sys.stdout.flush()

# This reads in the default parameters:
default_file = '%s/SoFiA_default_input.txt'%(os.path.dirname(os.path.realpath(__file__)))
#default_file = 'SoFiA_default_input.txt'
Parameters = readoptions.readPipelineOptions(default_file)



# ------------------------------
# ---- READ USER PARAMETERS ----
# ------------------------------

print "\n--- SoFiA: Reading user parameters ---"
sys.stdout.flush()

# This reads in a file with parameters and creates a dictionary:
parameter_file = sys.argv[1]
print 'Parameters extracted from: ', parameter_file
print
User_Parameters = readoptions.readPipelineOptions(parameter_file)

# Overwrite default parameters with user parameters (if exist):
for task in User_Parameters.iterkeys():
        if(task in Parameters):
                for key in User_Parameters[task].iterkeys():
                        if(key in Parameters[task]):
                                Parameters[task][key] = User_Parameters[task][key]

# Define the base and output directory name used for output files (defaults to input file 
# name if writeCat.basename is found to be invalid):
outputBase = Parameters['writeCat']['basename']
outputDir  = Parameters['writeCat']['outputDir']

if((not outputBase) or outputBase.isspace() or ("/" in outputBase) or ("\\" in outputBase) or (outputBase == ".") or (outputBase == "..")):
    outroot = Parameters['import']['inFile'].split('/')[-1]
    if((outroot.lower()).endswith(".fits") and len(outroot) > 5):
        outroot = outroot[0:-5]
else:
    outroot = outputBase

if((not outputDir) or (not os.path.isdir(outputDir)) or (outputDir.isspace())):
    outroot = Parameters['import']['inFile'][0:len(Parameters['import']['inFile'])-len(Parameters['import']['inFile'].split('/')[-1])]+outroot
else:
    if outputDir[-1] != '/': outputDir += '/'
    outroot = outputDir + outroot


# ---------------------
# ---- IMPORT DATA ----
# ---------------------

print "\n--- SoFiA: Reading data cube(s) ---"
sys.stdout.flush()

np_Cube, dict_Header, mask = import_data.read_data(**Parameters['import'])


# -------------------------
# ---- PRECONDITIONING ----
# -------------------------

print "\n--- SoFiA: Running input filters ---"
sys.stdout.flush()

# ---- FLAGGING ----
if Parameters['steps']['doFlag']:
	np_Cube = flag_cube.flag(np_Cube,**Parameters['flag'])

# ---- SMOOTHING ----
if Parameters['steps']['doSmooth']:
	np_Cube = smooth_cube.smooth(np_Cube, **Parameters['smooth'])	

# ---- RFI ----


# ---- SIGMA CUBE ---- 
if Parameters['steps']['doScaleNoise']:
	np_Cube = sigma_cube.sigma_scale(np_Cube, **Parameters['scaleNoise'])



# -----------------
# ---- FILTERS ----
# -----------------

print "\n--- SoFiA: Running source finder ---"
sys.stdout.flush()

# apply the different filters that each create a mask.
# create an empty mask, the size of the cube:


# --- PYFIND ---
if Parameters['steps']['doSCfind']:
	print 'Running S+C filter'
	pyfind_mask = pyfind.SCfinder(np_Cube, dict_Header, **Parameters['SCfind'])
	mask = mask + pyfind_mask

# --- WAVELET ---	
if Parameters['steps']['doWavelet']:
	print 'Running wavelet filter'
	# still to be added

# --- CNHI ---	
if Parameters['steps']['doCNHI']:
	print 'Running CNHI filter'
	# still to be added

# --- THRESHOLD ---	
if Parameters['steps']['doThreshold']:
	print 'Running threshold filter'
	#mask+=threshold_filter.filter(np_Cube, dict_Header, **Parameters['threshold'])
	threshold_filter.filter(mask,np_Cube, dict_Header, **Parameters['threshold'])

print 'Filtering complete.'
print

# Check whether any voxel is detected
NRdet = (mask > 0).sum()
if not NRdet:
	print 'WARNING: No voxels detected!'
	print 

# Reload original data cube for parameterisation if it has been changed
if Parameters['steps']['doSmooth'] or Parameters['steps']['doScaleNoise'] or Parameters['import']['weightsFile']:
	Parameters['import']['weightsFile'] = ''
	Parameters['import']['maskFile'] = ''
	np_Cube, dict_Header, bla = import_data.read_data(**Parameters['import'])



# -----------------
# ---- MERGING ----
# -----------------

if Parameters['steps']['doMerge'] and NRdet:
	print "\n--- SoFiA: Merging detections ---"
	sys.stdout.flush()
	
	objects, mask = linker.link_objects(np_Cube, mask, **Parameters['merge'])
	
	print 'Merging complete'
	print



# -------------------------------------
# ---- OUTPUT FOR DEBUGGING (MASK) ----
# -------------------------------------

if Parameters['steps']['doDebug'] and NRdet:
	print "\n--- SoFiA: Writing all-source mask cube for debugging ---"
	sys.stdout.flush()
	writemask.writeMask(mask, dict_Header, Parameters, '%s_mask.debug_all.fits'%outroot)



# ----------------------------------------------------
# ---- ESTIMATE RELIABILITY FROM NEGATIVE SOURCES ----
# ----------------------------------------------------

if Parameters['steps']['doReliability'] and Parameters['steps']['doMerge'] and NRdet:
	print "\n--- SoFiA: Determining reliability ---"
	sys.stdout.flush()
	objects,reliable = addrel.EstimateRel(np.array(objects), outroot, **Parameters['reliability'])
	print 'The following sources have been detected:', reliable
	print
elif Parameters['steps']['doMerge'] and NRdet:
	reliable = list(np.array(objects)[np.array(objects)[:,16] > 0,0].astype(int)) # select all positive sources
	print 'The following sources have been detected:', reliable
else: reliable=[1,] # if not merging, all detected voxels have ID = 1 and here they are set to be reliable



# ------------------------------------------
# ---- OUTPUT FOR DEBUGGING (CATALOGUE) ----
# ------------------------------------------

if Parameters['steps']['doMerge'] and NRdet:
	if 'bunit' in dict_Header: dunits=dict_Header['bunit']
	else: dunits='-'
	catParNames = ('ID','Xg','Yg','Zg','Xm','Ym','Zm','Xmin','Xmax','Ymin','Ymax','Zmin','Zmax','NRvox','Fmin','Fmax','Ftot')
	catParUnits = ('-','pix','pix','chan','pix','pix','chan','pix','pix','pix','pix','chan','chan','-',dunits,dunits,dunits)
	catParFormt = ('%10i', '%10.3f', '%10.3f', '%10.3f', '%10.3f', '%10.3f', '%10.3f', '%7i', '%7i', '%7i', '%7i', '%7i', '%7i', '%8i', '%12.3e', '%12.3e', '%12.3e')
	if Parameters['steps']['doReliability']:
		catParNames = tuple(list(catParNames) + ['NRpos',  'NRneg',  'Rel'])
		catParUnits = tuple(list(catParUnits) + ['-','-','-'])
		catParFormt = tuple(list(catParFormt) + ['%12.3e', '%12.3e', '%12.6f'])
	if Parameters['steps']['doDebug']:
		print "\n--- SoFiA: Writing all-source catalogue for debugging ---"
		#sys.stdout.flush()
		store_ascii.make_ascii_from_array(objects, catParNames, catParUnits, catParFormt, Parameters['writeCat']['parameters'], outroot+'_cat.debug.ascii')



# --------------------------------------------------
# ---- REMOVE NON RELIABLE AND NEGATIVE SOURCES ----
# --------------------------------------------------

if Parameters['steps']['doMerge'] and NRdet:
	print "\n--- SoFiA: Removing all sources that are not reliable ---"

	# make sure that reliable is sorted
	relList = list(reliable)
	relList.sort()
	reliable = np.array(relList)

	# remove non reliable sources in the objects array
	relObjects = []
	for rr in reliable:
		relObjects.append([len(relObjects) + 1] + list(objects[rr - 1]))
	relObjects = np.array(relObjects)
	objects = relObjects

	tmpCatParNames = list(catParNames)
	tmpCatParNames[0] = 'ID_old'
	catParNames = tuple(['ID'] + tmpCatParNames)

	tmpCatParFormt = list(catParFormt)
	catParFormt= tuple(['%10i'] + tmpCatParFormt)
	tmpCatParUnits = list(catParUnits)
	catParUnits= tuple(['-'] + tmpCatParUnits)

	# in the mask file
	mask *= -1
	index = 1
	for rr in reliable:
        	mask[mask == -rr] = index
        	index += 1
	mask[mask < 0] = 0


	newRel = []
	for i in range(0, len(relObjects)):
        	newRel.append(i + 1)
	reliable = np.array(newRel)



# ----------------------------------------
# ---- OUTPUT FOR DEBUGGING (MOMENTS) ----
# ----------------------------------------

if Parameters['steps']['doDebug'] and NRdet:
	print "\n--- SoFiA: Writing pre-optimisation mask and moment maps for debugging ---"
	sys.stdout.flush()
	debug=1
	writemask.writeMask(mask, dict_Header, Parameters, '%s_mask.debug_rel.fits'%outroot)
	mom0_Image = writemoment.writeMoment0(np_Cube, mask, outroot, debug, dict_Header)
	writemoment.writeMoment1(np_Cube, mask, outroot, debug, dict_Header, mom0_Image)



# ----------------------
# ---- PARAMETERISE ----
# ----------------------

#results=PyCatalog.PySourceCatalog()
#results.readDuchampFile("duchamp-results.txt")

if Parameters['steps']['doParameterise'] and Parameters['steps']['doMerge'] and NRdet:
	print "\n--- SoFiA: Parametrising sources ---"
	sys.stdout.flush()
#	np_Cube, dict_Header, mask, objects, catParNames, catParFormt = parametrisation.parametrise(np_Cube, dict_Header, mask, objects, catParNames, catParFormt, Parameters)
	np_Cube, mask, objects, catParNames, catParFormt, catParUnits = parametrisation.parametrise(np_Cube, mask, objects, catParNames, catParFormt, catParUnits, Parameters, dunits)
	catParNames=tuple(catParNames)
	catParUnits=tuple(catParUnits)
	catParFormt=tuple(catParFormt)

        
	print 'Parameterisation complete'
	print





# --------------------
# ---- WRITE MASK ----
# --------------------

if Parameters['steps']['doWriteMask'] and NRdet:
        print "\n--- SoFiA: Writing mask ---"
	writemask.writeMask(mask, dict_Header, Parameters, '%s_mask.fits'%outroot)



# -------------------
# ---- MAKE MOM0 ----
# -------------------

if Parameters['steps']['doMom0'] or Parameters['steps']['doMom1']:
	print "\n--- SoFiA: Writing moment-0 map ---"
	sys.stdout.flush()
	debug = 0
	mom0_Image = writemoment.writeMoment0(np_Cube, mask, outroot, debug, dict_Header)



# --------------------
# ---- MAKE MOM1  ----
# --------------------

if Parameters['steps']['doMom1'] and NRdet:
	print "\n--- SoFiA: Writing moment-1 map ---"
	sys.stdout.flush()
	debug = 0
	writemoment.writeMoment1(np_Cube, mask, outroot, debug, dict_Header, mom0_Image)



# ------------------------
# ---- STORE CUBELETS ----
# ------------------------

if Parameters['steps']['doCubelets'] and Parameters['steps']['doMerge'] and NRdet:
	print "\n--- SoFiA: Writing cubelets ---"
	sys.stdout.flush()
	objects = np.array(objects)
	cathead = np.array(catParNames)
	cubelets.writeSubcube(np_Cube, dict_Header, mask, objects, cathead, outroot)



# ---------------------------------------------------
# ---- APPEND PARAMETER VALUES IN PHYSICAL UNITS ----
# ---------------------------------------------------

if Parameters['steps']['doMerge'] and NRdet:
	print "\n--- SoFiA: Adding WCS position to catalog ---"
	sys.stdout.flush()
	objects, catParNames, catParFormt, catParUnits = wcs_coordinates.add_wcs_coordinates(objects,catParNames,catParFormt,catParUnits,Parameters)



# --------------------
# ---- STORE DATA ----
# --------------------

if Parameters['steps']['doWriteCat'] and Parameters['steps']['doMerge'] and NRdet:
	print "\n--- SoFiA: Writing output catalogue ---"
	sys.stdout.flush()
	if Parameters['writeCat']['writeXML'] and Parameters['steps']['doMerge'] and NRdet:
		store_xml.make_xml_from_array(objects, catParNames, catParUnits, catParFormt, Parameters['writeCat']['parameters'],outroot + '_cat.xml')
		#store_xml.make_xml(results, outroot + '_cat.xml')
	if Parameters['writeCat']['writeASCII'] and Parameters['steps']['doMerge'] and NRdet:
		store_ascii.make_ascii_from_array(objects, catParNames, catParUnits, catParFormt, Parameters['writeCat']['parameters'], outroot+'_cat.ascii')
		#store_ascii.make_ascii(results, Parameters['writeCat']['parameters'], outroot + '_cat.ascii')



print "\n--- SoFiA: Pipeline finished ---"
sys.stdout.flush()
