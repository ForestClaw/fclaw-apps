function region = readregion(fname)
%
% READREGION reads the region information stored in a file.
%
%     REGION = READREGION(FNAME) reads the region information stored
%     in the file FNAME and returns the structure REGION. This
%     structure is read by grid mapping routine MAPC2P_MSH and also
%     used by the plotting routine PLOTREGION.
%
%     If the extension ".region" is not explicitly given in FNAME,
%     it will be assumed.
%
%     See also CREATEREGION, PLOTREGION, MAPC2P_MSH.
%


endl = sprintf('\n');

i = findstr(fname,'.region');
if (isempty(i))
  fname1 = [fname '.region'];
else
  fname1 = fname;
end;
fid = fopen(fname1,'r');

% ---------------------------------
% Read mcells, ncells
[data,info] = fscanf(fid,'%d',1);
region.mcells = data-1;

fid = scan_to_endl(fid);

[data,info] = fscanf(fid,'%d',1);
region.ncells = data-1;

fid = scan_to_endl(fid);

% ---------------------------------
% Read xlow, ylow
[data,info] = fscanf(fid,'%g',1);
region.xmin = data;

fid = scan_to_endl(fid);

[data,info] = fscanf(fid,'%g',1);
region.ymin = data;

fid = scan_to_endl(fid);

% ---------------------------------
% Read xhigh, yhigh
[data,info] = fscanf(fid,'%g',1);
region.xmax = data;

fid = scan_to_endl(fid);

[data,info] = fscanf(fid,'%g',1);
region.ymax = data;

fid = scan_to_endl(fid);

% ---------------------------------
% Read zlow, zhigh
[data,info] = fscanf(fid,'%g',1);
region.msh_zlow = data;

fid = scan_to_endl(fid);

[data,info] = fscanf(fid,'%g',1);
region.msh_zhigh = data;

fid = scan_to_endl(fid);

% ------------------------------------
%  Read dx, dy
[data,info] = fscanf(fid,'%g',1);
region.dx = data;

fid = scan_to_endl(fid);

[data,info] = fscanf(fid,'%g',1);
region.dy = data;

fid = scan_to_endl(fid);

% --------------------------------------
% Read in data
[data,info] = fscanf(fid,'%g',(region.mcells+1)*(region.ncells+1));
region.data = reshape(data,(region.ncells+1),(region.mcells+1))';

fclose(fid);

region.x = linspace(region.xmin,region.xmax,region.mcells+1);
region.y = linspace(region.ymin,region.ymax,region.ncells+1);

% region.xmax = region.x(end);
% region.ymax = region.y(end);

% ----------------------------------------------
% This function reads in any characters/comments after values
% and returns a file handle that is at new line.
% -----------------------------------------------
function fid = scan_to_endl(fid)

endl = sprintf('\n');

[data,info] = fscanf(fid,'%c',1);
while (strcmp(data,endl) == 0)
  [data,info] = fscanf(fid,'%c',1);
end;
