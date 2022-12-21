function pout = plotregion(ds,R)
%
% PLOTREGION plots a region of the Mt. St. Helens topography.
%
%     PLOTREGION(DS) takes a structure DS read in by READREGION and
%     plots it to the current figure window.  If the topography is
%     is already showing in the current window, it is replaced by the
%     new topographic image. Camera lighting is added if it isn't already
%     there.
%
%     PLOTREGION(DS,R) takes an optional refinement factor R.
%     This refinement factor allows the user to specify how resolved
%     to make the topography.  Depending on the graphics capabilities
%     of the machine on which the data is being viewed, the rendering
%     of the full image (R = 1) can be quite slow.  Coarsened versions
%     of the full image (R = 2 or 4, for example) will render much faster.
%     The default value is R = 4.
%
%     P = PLOTREGION(...) returns a handle to a patch object.  This can be
%     useful if you wish to hide the topography, for example.  To do this,
%     use the command
%
%               >> set(p,'visible','off');
%
%     To show the grid resolution of the topograhpy, use the command
%
%               >> set(p,'edgecolor','k');
%
%     NOTE : The colormap used to color the topography is hardwired and
%     will not be affected by any user-applied colormap used to visualize
%     computated solutions.
%
%     See also READREGION, CREATEREGION, MAPC2P_MSH.
%


topo = findobj(gcf,'tag','msh_topo');
if (~isempty(topo))
  delete(topo);
end;

if (nargin == 1)
  R = 4;
end;

m = length(ds.x);
n = length(ds.y);

im = (1:R:m)';
in = (1:R:n)';

x = ds.x(im);
y = ds.y(in);

[xm,ym] = meshgrid(x,y);


p = patch(surf2patch(xm,ym,ds.data(im,in)'));
set(p,'EdgeColor','none');


set(p,'SpecularStrength',0.1);
set(p,'SpecularColorReflectance',0.1);
set(p,'AmbientStrength',0.6);
set(p,'DiffuseStrength',0.8);
set(p,'FaceLighting','gouraud');

cmap_old = colormap;
colormap(gray);
cmap = colormap;
cmap(1:5,1:2) = 0.8*cmap(1:5,1:2);
cmap(6:25,1) = 0.8*cmap(6:25,1);
cmap(6:25,3) = 0.9*cmap(6:25,3);
cmap(6:40,3) = 0.8*cmap(6:40,3);
N = length(cmap);
colormap(cmap);

v = get(p,'Vertices');
cdata = v(:,3);
cmin = ds.msh_zlow;
cmax = ds.msh_zhigh;
idx = (N-1)/(cmax-cmin)*(cdata - cmin) + 1;
% idx = round(idx);
r = interp1(1:N,cmap(:,1),idx,'*linear');
g = interp1(1:N,cmap(:,2),idx,'*linear');
b = interp1(1:N,cmap(:,3),idx,'*linear');
set(p,'FaceVertexCData',[r g b]);
set(p,'FaceColor','interp')
set(p,'tag','msh_topo');

colormap(cmap_old);

% Add a new light;  get rid of any old lighting, otherwise
% things get too washed out.
h = findobj(gca,'type','light');
if (isempty(h))
  %camlight;
end


zmax = cmax;
set(gca,'ZLim',[0 2*zmax]);
set(gca,'xdir','reverse');

% The OPENGL Renderer doesn't seem to work with xdir = reverse;
% or least gives funny results
% set(gcf,'Renderer','opengl');
set(gcf,'Renderer','zbuffer');

% daspect([1 1 1]);
xtick = (-40000:5000:5000);
ytick = (15000:5000:50000);
xticklabel = xtick/1000;
yticklabel = ytick/1000;
xlim = [ds.xmin ds.xmax];
ylim = [ds.ymin ds.ymax];
set(gca,'xlim',xlim,'ylim',ylim,...
    'xtick',xtick,'ytick',ytick,...
    'xticklabel',xticklabel,'yticklabel',yticklabel);
set(gca,'color',[0.8 0.8 0.8]);
set(gca,'tickdir','out');
set(gca,'fontsize',10);
set(gca,'box','off');
xlabel('East/West (km)','fontsize',12);
ylabel('South/North (km)','fontsize',12);
zlabel('Altitude (m)','fontsize',12)
% view(2);



if (nargout > 0)
  pout = p;
end
