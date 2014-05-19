#! /usr/bin/env python


from xml.etree import ElementTree
from xml.etree.ElementTree import Element, SubElement, tostring, XML
from xml.dom import minidom
from numpy import *



# example page:
# http://pymotw.com/2/xml/etree/ElementTree/create.html


def prettify(elem):
    # Return a pretty-printed XML string for the Element.
    # Indent is set to "" here to save disk space; default would be "\t".
    rough_string = ElementTree.tostring(elem, 'utf-8')
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent="")





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



def make_xml_from_array(objects,cathead,catunits,catfmt,store_pars,outname):
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
	if store_pars == ['*']:
		for i in cathead:
			field = SubElement(table, 'FIELD', name=i, datatype='float', unit=catunits[cathead.index(i)])
	else:
		for par in store_pars:
			index = list(cathead).index(par)
			field = SubElement(table, 'FIELD', name=cathead[index], datatype='float', unit=catunits[index])
	
	# write the data	
	data = SubElement(table, 'DATA')
	tabledata = SubElement(data, 'TABLEDATA')

	if store_pars == ['*']:
		for obj in objects:
			tr = SubElement(tabledata,'TR')
			for i in range(0,len(obj)):
				td = SubElement(tr,'TD')
				td.text = (catfmt[i]%obj[i]).strip()
			  
			
	else:	
		for obj in objects:
			tr = SubElement(tabledata,'TR')
			for par in store_pars:
				td = SubElement(tr,'TD')
				index = list(cathead).index(par)
				td.text = (catfmt[index]%obj[index]).strip()
				


	f1 = open(outname, 'w+')
	f1.write(prettify(top))
	f1.close

	return
