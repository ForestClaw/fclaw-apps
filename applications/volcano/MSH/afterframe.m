plot_msh();

showslices()
showpatchborders()

fprintf("qmax = %12.4e\n",qmin)
fprintf("qmax = %12.4e\n",qmax);

% Contour lines
cv = linspace(qmin,qmax,30);
drawcontourlines(cv);

% z-axis and slices
showgridlines(1:3,'z')
setslicecolor('none','z');

% Color map and color bar
colormap(parula)
clim([5e4,3e5])
% clim([qmin,qmax])
colorbar

% Camera lighting
view(vfront)
hfront = camlight;

view(vleft)
hleft = camlight;

view(-30.76,34.42)
hv = camlight;

% Axes and aspect ratio
daspect([1 1 1]);
set(gca,'clipping','off')
set(gca,'zlimmode','auto')

tstr = sprintf("Pressure at t = %.2f",t);
title(tstr,'fontsize',16);


