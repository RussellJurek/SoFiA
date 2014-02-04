#! /usr/bin/env python

##################################################################################
#
# addrel
# last modified: 2013-05-15
#
#  run as: addrel <table1>.txt ... <tableN>.txt
# creates: <table1>_rel.txt ... <tableN>_rel.txt [Ndrel_scatter.png]
#          [Ndrel_contour.pdf] 
#
# - reads table(s) of positive and negative detection parameters written by
#   pyfind  
# - calculates the N-dimensional density field of positive and negative
#   sources in a chosen parameter space (putting all tables together)
# - writes new table(s) equal to the input table(s) + two columns giving the 
#   number of positive and negative sources predicted by the N-dimensional
#   density fields at the location of each detected source
# - optionally creates two figures with:
#   1) scatter plot of positive and negative sources on various projections of
#      the chosen N-dimentional parameter space
#   2) contour plot of the 2-dimensional density field of positive and
#      negative sources on the same projections of 1), with (pseudo-)reliable
#      sources overplotted
# - if <tablei>_rel.txt already exist(s), will create the above figures (if
#   requested) from these tables without re-computing the N-dimensional density
#   field. The code will, however, calculate the 2-dimensional density fields
#   for all projections
#
# input parameters to be given below:
# - column of various parameters in the input tables
# - doscatter and docontour to choose whether to create the above figures
# - minimum reliability for a source to be flagged reliable
# - limit on low-number statistics when calculating the reliability
#
# more input can be specified in the code section:
# "SET PARAMETERS TO WORK WITH AND GRIDDING/PLOTTNG/SMOOTHING FOR EACH"
# these are:
# - parameters to actually work with
# - axis label and limit when plotting
# - grid on which to compute the number of positive and negative sources
#   predicted by the 2-dimensional density fields when creating contour figure
# - sigma of gaussians used to smooth the source distribution and estimate the
#   density fields
# - projections to plot
#
##################################################################################


# IMPORT PYTHON MODULES
from pylab import *
from sys import exit,argv
import string
from matplotlib import rc
import scipy.stats as stats
import pyfits
from os import path
rc('text', usetex=True)
rc('font', family='serif')
rc('font', serif='Times Roman')
rc('font', size=18)


# START INPUT

#Nmin=0         # minimum number of positive+negative sources within dV**Npars
# END INPUT

# define class of gaussian_kde with user-defined covariance matrix
class gaussian_kde_set_covariance(stats.gaussian_kde):
	def __init__(self, dataset, covariance):
		self.covariance = covariance
		stats.gaussian_kde.__init__(self, dataset)
	def _compute_covariance(self):
		self.inv_cov = linalg.inv(self.covariance)
		self._norm_factor = sqrt(linalg.det(2*pi*self.covariance)) * self.n

def EstimateRel(data,pngoutname,idCOL=0,nrvoxCOL=13,fminCOL=14,fmaxCOL=15,ftotCOL=16,parSpace=['ftot','fmax','nrvox'],projections=[[2,0],[2,1],[0,1]],kernel=[0.15,0.05,0.1],doscatter=1,docontour=1,check_kernel=0,dostats=0,saverel=1,relThresh=0.99,Nmin=0,dV=0.2,fMin=0,verb=0):
	########################################
	### BUILD ARRAY OF SOURCE PARAMETERS ###
	########################################
	
	# get position of positive and negative sources
	pos=data[:,ftotCOL]>0
	neg=data[:,ftotCOL]<=0
	Npos=pos.sum()
	Nneg=neg.sum()

	if not Npos:
		print 'FATAL ERROR: no positive sources found; cannot proceed'
		exit()
	elif not Nneg:
		print 'FATAL ERROR: no negative sources found; cannot proceed'
		exit()

	# get array of relevant source parameters and set what to plot
	ids=data[:,idCOL]
	nrvox=data[:,nrvoxCOL].reshape(-1,1)
	ftot =abs(data[:,ftotCOL]).reshape(-1,1)
	fmax =data[:,fmaxCOL]*pos-data[:,fminCOL]*neg
	fmax=fmax.reshape(-1,1)

	#################################################################
	### SET PARAMETERS TO WORK WITH AND GRIDDING/PLOTTNG FOR EACH ###
	#################################################################

	pars=concatenate((log10(eval(parSpace[0])),
					  log10(eval(parSpace[1])),
					  log10(eval(parSpace[2]))),
					 axis=1)

	pars=transpose(pars)

	# axis labels when plotting
	labs=['log$_\mathrm{10}$ $F_\mathrm{tot}$',
		  'log$_\mathrm{10}$ $F_\mathrm{max}$',
		  'log$_\mathrm{10}$ $N_\mathrm{vox}$',
		  'log$_\mathrm{10}$ $N_\mathrm{chan}$',
		  ]

	# axes limits when plotting
	pmin,pmax=pars.min(axis=1),pars.max(axis=1)
	pmin,pmax=pmin-0.1*(pmax-pmin),pmax+0.1*(pmax-pmin)
	lims=[[pmin[0],pmax[0]],
		  [pmin[1],pmax[1]],
		  [pmin[2],pmax[2]],
		  ]

	# grid on which to evaluation Np and Nn in order to plot contours
	grid=[[pmin[0],pmax[0],0.02*(pmax[0]-pmin[0])],
		  [pmin[1],pmax[1],0.02*(pmax[1]-pmin[1])],
		  [pmin[2],pmax[2],0.02*(pmax[2]-pmin[2])],
		  ]

	# calculates the number of row and columns in figure
	nr=int(floor(sqrt(len(projections))))
	nc=len(projections)/nr

	# variance (sigma**2) of gaussian for smoothing along each axis
	# covariance matrix is taken to be diagonal (i.e., all sigma_ij terms are 0)
	kernel=array(kernel)

	################################
	### EVALUATE N-d RELIABILITY ###
	################################

	if verb: print '  estimate normalised positive and negative density fields ...'

	# Np and Nn calculated with *authomatic* covariance
	#Np=stats.kde.gaussian_kde(pars[:,pos])
	#Nn=stats.kde.gaussian_kde(pars[:,neg])

	# Np and Nn calculated with *input* covariance
	setcov=array(((kernel[0]**2,0,0),
				  (0,kernel[1]**2,0),
				  (0,0,kernel[2]**2)))

	if verb:
		print '  using diagonal kernel with sigma:',
		for kk in kernel: print '%.4f'%kk,
		print

	Np=gaussian_kde_set_covariance(pars[:,pos],setcov)
	Nn=gaussian_kde_set_covariance(pars[:,neg],setcov)

	#############################
	### PRINT STATS TO SCREEN ###
	#############################

	if docontour or dostats:
		# volume within which to calculate the 
		dV=(2*kernel).prod()

		if verb: print '  calculating reliability at source location from density field ...'

		Nps=Np(pars[:,pos])*Npos
		Nns=Nn(pars[:,pos])*Nneg
		Rs=(Nps-Nns)/Nps

		if Rs.max()>1:
			print '  maximum reliability larger than 1 -- something is wrong!!!'
			exit()
		# take maximum(Rs,0) to include objects with Rs<0 if relThresh==0
		pseudoreliable=maximum(Rs,0)>=relThresh

		# OLD
		#if check_kernel:
		#    # integrate density fields within boxes of side +/- kernel centred on
		#    # positive sources
		#    # I have verified that these integrals give very similar R as that
		#    # below; i.e., R(evaluate)=R(integrate). See Re
		#    print '  integrating density fields within a +/-1 sigma(kernel) box (this takes time) ...'
		#    NpI=array([Np.integrate_box(pars[:,pos][:,jj]-kernel,pars[:,pos][:,jj]+kernel)*Npos for jj in range(Npos)])
		#    NnI=array([Nn.integrate_box(pars[:,pos][:,jj]-kernel,pars[:,pos][:,jj]+kernel)*Nneg for jj in range(Npos)])
		#
		#    print '  source density at the location of positive sources (per +/-1 sigma(kernel) volume element):'
		#    print '             positive: %3.1f - %3.1f'%(NpI.min(),NpI.max())
		#    print '             negative: %3.1f - %3.1f'%(NnI.min(),NnI.max())
		#    print '  positive + negative: %3.1f - %3.1f'%((NpI+NnI).min(),(NpI+NnI).max())
		#    dRs=sqrt(NpI*NnI*(NpI+NnI))/NpI**2
		#    print '  median error on R is %.2f'%median(dRs)
		#    print '  %i/%i positive sources have R<0 within 1-sigma error bar'%(((Rs+1*dRs)<0).sum(),Rs.shape[0])
		#
		#    reliable=(Rs>relThresh)*((NpI+NnI)>Nmin)
		#    
		#    #plot(Nps*dV,NpI,'bo')
		#    #plot(Nns*dV,NnI,'ro')
		#    #plot([0,1],[0,1*0.85],'k-')
		#    #show()
		#    #exit()
		# END OLD

		# I have verified (see above OLD lines) that NpI=0.85*Nps for a variety of
		# reasonable kernels, so I use 0.85*Nps as a proxy for NpI (same for Nns)
		if verb:
			print '  multiplying by 0.85*dV to get a proxy of integral of density fields within a +/-1 sigma(kernel) box ...'
			print '  source density at the location of positive sources (per +/-1 sigma(kernel) volume element):'
			print '             positive: %3.1f - %3.1f'%(0.85*dV*Nps.min(),0.85*dV*Nps.max())
			print '             negative: %3.1f - %3.1f'%(0.85*dV*Nns.min(),0.85*dV*Nns.max())
			print '  positive + negative: %3.1f - %3.1f'%(0.85*dV*(Nps+Nns).min(),0.85*dV*(Nps+Nns).max())

		nNps=Np(pars[:,neg])*Npos
		nNns=Nn(pars[:,neg])*Nneg
		nRs=(nNps-nNns)/nNps
		dnRs=sqrt(nNps*nNns*(nNps+nNns))/nNps**2/sqrt(dV*0.85)
		if verb:
			print '  median error on R at location of negative sources: %.2f'%median(dnRs)
			print '  R<0 at the location of %3i/%3i negative sources'%((nRs<0).sum(),nRs.shape[0])
			print '  R<0 at the location of %3i/%3i negative sources within 1-sigma error bar'%(((nRs+1*dnRs)<0).sum(),nRs.shape[0])

		dRs=sqrt(Nps*Nns*(Nps+Nns))/Nps**2/sqrt(dV*0.85)
		if verb:
			print '  median error on R at location of positive sources: %.2f'%median(dRs)
			print '  R<0 at the location of %3i/%3i positive sources'%((Rs<0).sum(),Rs.shape[0])
			print '  R<0 at the location of %3i/%3i positive sources within 1-sigma error bar'%(((Rs+1*dRs)<0).sum(),Rs.shape[0])
		# Nmin is by default zero so the line below normally selects (Rs>=relThresh)*(ftot[pos].reshape(-1,)>fMin)
		# take maximum(Rs,0) to include objects with Rs<0 if relThresh==0
		reliable=(maximum(Rs,0)>=relThresh)*((Nps+Nns)*0.85*dV>Nmin)*(ftot[pos].reshape(-1,)>fMin)
		
		delt=(nNps-nNns)/sqrt(nNps+nNns)*sqrt(0.85*dV)

		if verb:
			print '  negative sources found:'
			print '    %20s: %4i'%('total',Nneg)
			print '  positive sources found:'
			print '    %20s: %4i'%('total',Npos),
			print
			print '                  R>%.2f: %4i'%(relThresh,pseudoreliable.sum()),
			print
			print '     R>%.2f, N(3sig)>%3i: %4i'%(relThresh,Nmin,reliable.sum()),
			print

		if check_kernel:
			plot(arange(-10,10,0.01),stats.norm().cdf(arange(-10,10,0.01)),'k-')
			plot(arange(-10,10,0.01),stats.norm(scale=0.4).cdf(arange(-10,10,0.01)),'k:')
			hist(delt,bins=arange(delt.min(),delt.max()+0.01,0.01),cumulative=True,histtype='step',normed=True,color='r')
			xlim(-3,3)
			ylim(0,1)
			xlabel('$(P-N)/\sqrt{N+P}$')
			ylabel('cumulative distribution')
			legend(('Gaussian ($\sigma=1$)','Gaussian ($\sigma=0.4$)','negative sources'),loc='upper left',prop={'size':15})
			plot([0,0],[0,1],'k--')
			title('$\sigma$(kde) = %.3f, %.3f, %.3f'%(kernel[0],kernel[1],kernel[2]),fontsize=20)
			show()
			#savefig('test_scatter.pdf')

	############################
	### SCATTER PLOT SOURCES ###
	############################

	if doscatter:
		if verb: print '  plotting sources ...'
		fig1=figure(figsize=(18,4.5*nr))
		subplots_adjust(left=0.06,bottom=0.15/nr,right=0.97,top=1-0.08/nr,wspace=0.35,hspace=0.25)

		np=0
		for jj in projections:
			if verb: print '    projection %i/%i'%(projections.index(jj)+1,len(projections))
			np,p1,p2=np+1,jj[0],jj[1]
			subplot(nr,nc,np)
			scatter(pars[p1,pos],pars[p2,pos],marker='o',c='b',s=10,edgecolor='face',alpha=0.5)
			scatter(pars[p1,neg],pars[p2,neg],marker='o',c='r',s=10,edgecolor='face',alpha=0.5)
			xlim(lims[p1][0],lims[p1][1])
			ylim(lims[p2][0],lims[p2][1])
			xlabel(labs[p1])
			ylabel(labs[p2])
		fig1.savefig('%s_scat.png'%pngoutname)

	#####################
	### PLOT CONTOURS ###
	#####################

	if docontour:
		levs=10**arange(-1.5,2,0.5)

		if verb: print '  plotting contours ...'
		fig2=figure(figsize=(18,4.5*nr))
		subplots_adjust(left=0.06,bottom=0.15/nr,right=0.97,top=1-0.08/nr,wspace=0.35,hspace=0.25)
		np=0
		for jj in projections:
			if verb: print '    projection %i/%i'%(projections.index(jj)+1,len(projections))
			np,p1,p2=np+1,jj[0],jj[1]
			g1,g2=grid[p1],grid[p2]
			x1=arange(g1[0],g1[1],g1[2])
			x2=arange(g2[0],g2[1],g2[2])
			pshape=(x2.shape[0],x1.shape[0])

			# get array of source parameters on current projection
			parsp=concatenate((pars[p1:p1+1],pars[p2:p2+1]),axis=0)

			# derive Np and Nn density fields on the current projection
			# Np and Nn calculated with *authomatic* covariance
			#Np=stats.kde.gaussian_kde(parsp[:,pos])
			#Nn=stats.kde.gaussian_kde(parsp[:,neg])
			# Np and Nn calculated with *input* covariance
			setcov=array(((kernel[p1]**2,0.0),(0.0,kernel[p2]**2)))
			Np=gaussian_kde_set_covariance(parsp[:,pos],setcov)
			Nn=gaussian_kde_set_covariance(parsp[:,neg],setcov)

			# evaluate density  fields on grid on current projection
			g=transpose(transpose(mgrid[slice(g1[0],g1[1],g1[2]),slice(g2[0],g2[1],g2[2])]).reshape(-1,2))
			Np=Np(g)
			Nn=Nn(g)
			Np=Np/Np.sum()*Npos
			Nn=Nn/Nn.sum()*Nneg
			Np.resize(pshape)
			Nn.resize(pshape)
			subplot(nr,nc,np)
			contour(x1,x2,Np,origin='lower',colors='b',levels=levs)
			contour(x1,x2,Nn,origin='lower',colors='r',levels=levs)

			if reliable.sum(): scatter(pars[p1,pos][reliable],pars[p2,pos][reliable],marker='o',s=10,edgecolor='k')
			if (pseudoreliable*(reliable==False)).sum(): scatter(pars[p1,pos][pseudoreliable*(reliable==False)],pars[p2,pos][pseudoreliable*(reliable==False)],marker='x',s=40,edgecolor='0.5')

			xlim(lims[p1][0],lims[p1][1])
			ylim(lims[p2][0],lims[p2][1])
			xlabel(labs[p1])
			ylabel(labs[p2])
		fig2.savefig('%s_cont.png'%pngoutname)

	###############################
	### ADD Np and Nn TO TABLES ###
	###############################

	# this allows me not to calculate R everytime I want to do
	# some plot analysis, but just read it from the file
	if saverel:
		if not (docontour or dostats):
			Nps=Np(pars[:,pos])*Npos
			Nns=Nn(pars[:,pos])*Nneg
		Np=zeros((data.shape[0],))
		Np[pos]=Nps
		Nn=zeros((data.shape[0],))
		Nn[pos]=Nns
		R=-ones((data.shape[0],)) # R will be -1 for negative sources
		# set R to zero for positive sources if R<0 because of Nn>Np
		R[pos]=maximum(0,(Np[pos]-Nn[pos])/Np[pos])
		data=concatenate((data,Np.reshape(-1,1),Nn.reshape(-1,1),R.reshape(-1,1)),axis=1)

	data=[list(jj) for jj in list(data)]
	return data,ids[pos][reliable].astype(int)
