% 
%    This is the MATLAB function that estimates battery parameters
%    alpha and beta, as reported in [1].
%
% ----------------------------------------
% Usage (in MATLAB environment):
%  >> [alpha, beta] = paramEst('ILtable.dat', alpha0, beta0)
%
% ---------------------------------------- 
% alpha0  - initial estimate for alpha (mA-min)
% beta0   - initial estimate for beta (1/min^0.5)
%
% ILtable.dat format:
% <current_load> <current_load> ...  - applied constant current (mA)
% <battery_life> <battery_life> ...  - measured battery life (min)
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


function [a, b] = paramEst(ILTable, a0, b0)

table = load(ILTable);

I = table(1, :);
L = table(2, :);

warning backtrace;

beta0 = [a0 b0];
[beta, r] = nlinfit(L, I, 'currentModel', beta0);

a = beta(1);
b = beta(2);

II = currentModel(beta, L);
figure;
semilogx(L, II, 'o', L, I, 'x');
xlabel('Battery Life, min');
ylabel('Current Load, mA');
grid on;

