function [xp,yp,zp] = mapc2p(xc,yc,zc)
%
%  MAPC2P_MSH is the mapping function for the Mt. St. Helens topography.
%
%      [XP,YP,ZP] = MAPC2P(XC,YC,ZC) takes computational coordinates,
%      assumed to be in [0,1]x[0,1]x[0,1] and maps them to coordinates
%      in the atmospheric zone above the Mt. St. Helens region
%      defined in the global variable REGION_MSH.  The computational
%      coordinate zc = 0 is mapped to the surface of of the topography,
%      and the value zc = 1 is mapped to a region 4 times the height
%      of the heighest point on the MSH topopgraphy (i.e. the crater rim,
%      at approximately 2500 meters).
%
%      This routine assumes that the variable named "region_msh" has
%      been defined as a global variable somewhere else in the graphics
%      code.  The logical place to do this is in SETPROB.
%
%      Note that in order to use this file, you must either copy it or
%      symbolically link it to the file "mapc2p.m".
%
%      There is an equivalent Fortran version of this in "mapc2p_msh.f".
%
%      See also PLOTREGION, CREATEREGION, READREGION.
%

global region_msh;

region_msh = readregion('msh3.region');

% if (isempty(region_msh))
%   str = ['mapc2p_msh : The region ''region_msh'' must be defined ',...
% 	' as a global variable.  See READREGION'];
%   error(str);
% end;

% Scale to brick domain.
s = 0;
[xc1,yc1,~] = mapc2m_brick(xc,yc,s);


dsv = region_msh;

xmin = dsv.xmin;
xmax = dsv.xmax;

ymin = dsv.ymin;
ymax = dsv.ymax;

xp = (xmax - xmin)*xc1 + xmin;
yp = (ymax - ymin)*yc1 + ymin;
alpha = 0.0;
zc = alpha*zc.*zc + (1-alpha)*zc;

[xm,ym] = meshgrid(dsv.x,dsv.y);

zbase = interp2(xm,ym,dsv.data',xp,yp,'*linear');

% Modify this for a different atmospheric ceiling height.
ztop = 4*dsv.msh_zhigh;

% I think we have to define yp again to flip the y indices.
zp = (ztop - zbase).*zc + zbase;
