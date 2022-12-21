function plot_msh()

ds = readregion('msh3.region');
plotregion(ds,1);

% [ax,bx,ay,by] = plot_topo('mshblalb.topo','w');
% [ax,bx,ay,by] = plot_topo('msh.region','w');

% Lower left   ( -112.34626736,  43.18013542)
% Upper right  ( -111.26428819,  43.95986458)

% axis([-112.34626736,-111.26428819,43.18013542,43.95986458]);
shg

end

function [ax,bx,ay,by] = plot_topo(fname,c)
% PLOT_TOPO plots the topo from file FNAME


% Top of the cinder cone.  Pass these values into plot_feature
% to get the height of the feature.
% xp = 3.0206e+03;
% yp = 1.1689e+04;

% behind the dam
% xp =  2.6732e+03;
% yp = 1.1942e+04;

% c = 'w';  % Color of the topo
hold on;
[p,ax,bx,ay,by] = plot_feature(fname,c);
hold on;

%fprintf('Height at input location : %12.4f\n',hp);

daspect([1,1,1]);

% axis([ax bx ay by]);

camlight;
setviews;
view(vtop);
shg

end


function [p,ax,bx,ay,by,hpout] = plot_feature(fname,c,xp,yp)

fid = fopen(fname);

ncols = fscanf(fid,'%d',1); fscanf(fid,'%s',1);
nrows = fscanf(fid,'%d',1); fscanf(fid,'%s',1);
xll = fscanf(fid,'%g',1);   fscanf(fid,'%s',1);
yll = fscanf(fid,'%g',1);   fscanf(fid,'%s',1);
dx = fscanf(fid,'%g',1);    fscanf(fid,'%s',1);
nodata = fscanf(fid,'%g',1); fscanf(fid,'%s',1);
T = fscanf(fid,'%g',nrows*ncols);
fclose(fid);

% --------------------------------
ax = xll;
ay = yll;

bx = ax + dx*(ncols-1);
by = ay + dx*(nrows-1);

x = linspace(ax,bx,ncols);
y = linspace(ay,by,nrows);

T = reshape(T,ncols,nrows);
T = fliplr(T);
% T = reshape(T,nrows,ncols)';
% T = fliplr(T);


[xm,ym] = meshgrid(x,y);

nodata = find(T == nodata);
T(nodata) = nan;
c = T;
c(~nodata) = 0;
c(nodata) = 1;

p = surf(xm,ym,T');
set(p,'cdata',c');
set(p,'edgecolor','none');

colormap([1 0 0; 1 1 1]);
set(p,'cdatamapping','direct');


fprintf('Min height  : %12.4f\n',min(T(:)));
fprintf('Max height  : %12.4f\n',max(T(:)));

zlim([min(T(:)), max(T(:))]);


if (nargin > 2)
    % find height of given location (xp,yp)
    hp = interp2(xm,ym,T,xp,yp,'linear');
    if (nargout > 5)
        hpout = hp;
    end
end


end

