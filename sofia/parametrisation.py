#! /usr/bin/env python

from sofia import parametrizer2
import numpy as np


def parametrise(cube,header,mask,objects,cathead,catformt,Parameters):
  
  cathead=np.array(cathead)
  objects=np.array(objects)
  initcatalog=parametrizer2.PySourceCatalog()
  for obj in objects:
    newSource=parametrizer2.PySource()
    newSource.setSourceID(obj[cathead=='ID'])
    newMeasurement=parametrizer2.PyMeasurement()
    newMeasurement.set("X",obj[cathead=='Xg'],0.,"")
    newSource.setParameter(newMeasurement)
    newMeasurement.set("Y",obj[cathead=='Yg'],0.,"")
    newSource.setParameter(newMeasurement)
    newMeasurement.set("Z",obj[cathead=='Zg'],0.,"")
    newSource.setParameter(newMeasurement)
    newMeasurement.set("BBOX_X_MIN",obj[cathead=='Xmin'],0.,"")
    newSource.setParameter(newMeasurement)
    newMeasurement.set("BBOX_X_MAX",obj[cathead=='Xmax'],0.,"")
    newSource.setParameter(newMeasurement)
    newMeasurement.set("BBOX_Y_MIN",obj[cathead=='Ymin'],0.,"")
    newSource.setParameter(newMeasurement)
    newMeasurement.set("BBOX_Y_MAX",obj[cathead=='Ymax'],0.,"")
    newSource.setParameter(newMeasurement)
    newMeasurement.set("BBOX_Z_MIN",obj[cathead=='Zmin'],0.,"")
    newSource.setParameter(newMeasurement)
    newMeasurement.set("BBOX_Z_MAX",obj[cathead=='Zmax'],0.,"")
    newSource.setParameter(newMeasurement)
    initcatalog.insert(newSource)

  moduleParametrizer = parametrizer2.PyModuleParametrisation()
  moduleParametrizer.setFlags(Parameters['parameters']['optimiseMask'], Parameters['parameters']['fitBusyFunction'])
  
  cube = cube.astype('<f4')
  mask = mask.astype('<i2')
  
  moduleParametrizer.run(cube, mask, initcatalog, header)
  results = moduleParametrizer.getCatalog()
  

  # append the results to the objects array or reset
  replParam = ['BBOX_X_MAX','BBOX_X_MIN','BBOX_Y_MAX','BBOX_Y_MIN','BBOX_Z_MAX','BBOX_Z_MIN','F_PEAK','F_TOT','ID','X','Y','Z']
  origParam = ['Xmax','Xmin','Ymax','Ymin','Zmax','Zmin','Fmax','Ftot','ID','Xm','Ym','Zm']
  d = results.getSourcesDict()
  # select data set with maximum number of parameters
  parsListLen = [len(d[d.keys()[i]].getParametersDict()) for i in range(0,len(d))]
  index = parsListLen.index(max(parsListLen))
  # add parameter names from parametrization
  pars = d[d.keys()[index]].getParametersDict()
  cathead = list(cathead)
  catformt = list(catformt)
  for i in sorted(pars):
    if i not in replParam:
      cathead.append(i)
      catformt.append('%12.3f')
  # extend the parameter array
  tmpObjects = np.empty((objects.shape[0],len(cathead)))
  tmpObjects[:,:] = np.NAN
  tmpObjects[:,0:objects.shape[1]] = objects
  objects = tmpObjects
  for i in d:
    source_dict = d[i].getParametersDict()
    # check the source index
    index = int(source_dict['ID'].getValue())
    for j in sorted(source_dict):
      if j in replParam:
        objects[index-1][cathead.index(origParam[replParam.index(j)])] = source_dict[j].getValue()
      else:
        objects[index-1][cathead.index(j)] = source_dict[j].getValue()

  objects = np.array(objects)
  cathead = np.array(cathead)
  catformt = np.array(catformt)
  print 'The parameterisation has completed'
  print
  
  #print objects.shape,cathead.shape
  return cube,header,mask,objects,cathead,catformt
