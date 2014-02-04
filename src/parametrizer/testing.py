import PyCatalog
p1=PyCatalog.PyUnit("km/s")
p2=PyCatalog.PyUnit("K")
print p1.asString()
print p2.asString()

p3=p1*p2
print p3.asString()

m1=PyCatalog.PyMeasurement()
m1.set("test",1.,1.,p1)
print m1.getName(),m1.getValue(),m1.getUncertainty(),m1.getUnit().asString()

m2=PyCatalog.PyMeasurement()
m2.set("test2",11.,11.,"K")
print m2.getName(),m2.getValue(),m2.getUncertainty(),m2.getUnit().asString()

m3=PyCatalog.PyMeasurement()
m3.set("test3",111.,111.,PyCatalog.unit_jy)
print m3.getName(),m3.getValue(),m3.getUncertainty(),m3.getUnit().asString()
m3.invert()
print m3.getName(),m3.getValue(),m3.getUncertainty(),m3.getUnit().asString()

m4=PyCatalog.PyMeasurement()
m4.set("test3",111.,111.,"m")
print m4.convert(PyCatalog.unit_kpc)

m4.invert()

m5=m3*m4
print m5.getName(),m5.getValue(),m5.getUncertainty(),m5.getUnit().asString()

s1=PyCatalog.PySource()
s1.setParameter(m1)
s1.setParameter(m2)

pyDict=s1.getParametersDict()

s2=PyCatalog.PySource()
s2.setParametersDict(pyDict)

s2.getParametersDict()

sc1=PyCatalog.PySourceCatalog()
sc1.readDuchampFile("duchamp-results.txt")

print sc1.getSourcesDict()[1].getParametersDict()['DEC'].asString()





