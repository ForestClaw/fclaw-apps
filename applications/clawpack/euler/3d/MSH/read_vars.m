function parms = read_vars()

data = load('setprob.data');

parms.example = data(1);
parms.mapping = data(2);
parms.manifold = data(3);
end