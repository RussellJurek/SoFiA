#! /usr/bin/env python


from xml.etree import ElementTree
from xml.etree.ElementTree import Element, SubElement, tostring, XML
from xml.dom import minidom
from numpy import *



# example page:
# http://pymotw.com/2/xml/etree/ElementTree/create.html


def prettify(elem):
    """Return a pretty-printed XML string for the Element.
    """
    rough_string = ElementTree.tostring(elem, 'utf-8')
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent="  ")





def make_xml(objects,outname):
	print 'Store the results to xml file: ', outname
	top = Element('VOTABLE')
	resource = SubElement(top, 'RESOURCE', name='SoFiA catalogue')
	description = SubElement(resource, 'DESCRIPTION')
	description.text='Output catalogue from SoFiA'
	coocys = SubElement(resource, 'COOSYS', ID='J2000')
	table = SubElement(resource, 'TABLE', ID='sofia_cat', name='sofia_cat')
	description = SubElement(table, 'DESCRIPTION')
	description.text='Output catalogue from SoFiA'
	
	# write the parameters in fields:
	d = objects.getSourcesDict()
	pars = d[d.keys()[0]].getParametersDict()
	for i in pars:
		# this has to be updated with proper types and units
		field = SubElement(table, 'FIELD', name=i, datatype='float', unit='-')

	# write the data	
	data = SubElement(table, 'DATA')
	tabledata = SubElement(data, 'TABLEDATA')

	for i in d:
		tr = SubElement(tabledata,'TR')
		source_dict = d[i].getParametersDict()
		for j in source_dict:
			td = SubElement(tr,'TD')
			td.text = str(source_dict[j].getValue())



	#print prettify(top)
	#print

	f1 = open(outname, 'w+')
	f1.write(prettify(top))
	f1.close

	return




