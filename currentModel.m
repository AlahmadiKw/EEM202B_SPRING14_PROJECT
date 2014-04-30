% 
%    This is the MATLAB function that computes current loads
%    given battery lives, as reported in [1].
%
% ----------------------------------------
% Usage (in MATLAB environment):
%  >> I = currentModel ([alpha, beta], L)
%
% ---------------------------------------- 
% I      - vector of current loads (mA)
% L      - vector of battery lives (min)
% alpha  - battery parameter (mA-min)
% beta   - battery parameter (1/min^0.5)
%
% ----------------------------------------
% Created by Daler N. Rakhmatov on November 12, 2003.
% Copyright (c) 2003 University of Victoria, all rights reserved.
% Contact: daler@ece.uvic.ca.
%
% ----------------------------------------
% [1] D.Rakhmatov, S.Vrudhula, and D.Wallach; 
% "A Model for Battery Lifetime Analysis for Organizing Applications
%  on a Pocket Computer", IEEE Trans. VLSI, vol.11, no.6, 2003.
%


function y = currentModel(beta, x)

a = beta(1);
b = beta(2);

y = x;
for m = 1:10,  % number of series terms
y = y + 2*(1 - exp(-b*b*m*m*x))/(b*b*m*m);
end;

y = a./y;
