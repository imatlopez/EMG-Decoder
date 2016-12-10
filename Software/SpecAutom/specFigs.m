%% Load
clear; close all;   
R = [1.15 1.26]; % Ohms

load('freq-1207-121555.mat');   % F - Diff
F  = cell2mat(raw(:,1));
FD = raw(:,3);

load('freq-1207-123435.mat');   % F - Comm
FC = raw(:,3);

load('amp-1207-132729.mat');    % A - Diff
A  = cell2mat(raw(:,1));
AD = raw(:,3);

load('amp-1207-123824.mat');    % A = Comm
AC = raw(:,3);

clear i j
dt = 1/1e4;
t = (0:dt:1-dt)';

%% Frequency Response
figure(1);
Y = [cellfun(@(x,f) peakmag(x(:,3), 1e-2, f), FD, num2cell(F)), cellfun(@(x,f) peakmag(x(:,1), 1e-2, f), FD, num2cell(F))];
semilogx(F, Y(:,1), 'k', F, Y(:,2), 'r');
xlabel('Frequency (Hz)')
ylabel('Response (dB)');
title('Frequency Response for Sinusoidal Input')
legend('Analog','To-Digital','location','best');
fixplot;
print -dpng FRQResp

%% Gain
figure(2);
Y = [cellfun(@(x,a) peakmag(x(:,3), a), AD, num2cell(A)), cellfun(@(x,a) peakmag(x(:,1), a), AD, num2cell(A))];
semilogx(A, Y(:,1),'k', A, Y(:,2),'r');
xlabel('Input (V)');
ylabel('Gain (dB)')
title('Gain at 100 Hz');
legend('Analog','To-Digital','location','best');
fixplot
print -dpng Gain

G = Y(55,:);

%% Common Mode Rejection
figure(3);
Y = -[cellfun(@(x,f) peakmag(x(:,3)/G(1), 1e-2, f), FC, num2cell(F)), cellfun(@(x,f) peakmag(x(:,1)/G(2), 1e-2, f), FC, num2cell(F))];
semilogx(F, Y(:,1), 'k', F, Y(:,2), 'r');
xlabel('Frequency (Hz)');
ylabel('Rejection Ratio (dB)');
title('CMRR versus Frequency at 10 mV');
legend('Analog','To-Digital','location','best');
fixplot;
print -dpng CmrrFRQ

figure(4);
Y = -[cellfun(@(x,a) peakmag(x(:,3)/G(1), a), AC, num2cell(A)), cellfun(@(x,a) peakmag(x(:,1)/G(2), a), AC, num2cell(A))];
semilogx(A, Y(:,1), 'k', A, Y(:,2),'r');
xlabel('Input (V)');
ylabel('Rejection Ratio (dB)');
title('CMRR versus Amplitude at 100 Hz');
legend('Analog','To-Digital','location','best');
fixplot;
print -dpng CmrrAMP

fprintf('CMRR1: %0.0f (%0.2f)\n',Y(end,1),10^(Y(end,1)/20))
fprintf('CMRR2: %0.0f (%0.2f)\n',Y(end,2),10^(Y(end,2)/20))

%% SNR
figure(5);
Y = [cellfun(@(x) peakmag(x(:,3), FC{1}(:,3)), FD), cellfun(@(x) peakmag(x(:,1), FC{1}(:,1)), FD)];
semilogx(F, Y(:,1), 'k', F, Y(:,2), 'r');
xlabel('Frequency (Hz)');
ylabel('Signal to Noise Ratio (dB)');
title('SNR versus Frequency at 10 mV');
legend('Analog','To-Digital','location','best');
fixplot;
print -dpng SnrFRQ

fprintf('SNR1: %0.0f (%0.2f)\n',Y(67,1),10^(Y(67,1)/20))
fprintf('SNR2: %0.0f (%0.2f)\n',Y(67,2),10^(Y(67,2)/20))

figure(6);
Y = [cellfun(@(x) peakmag(x(:,3), AC{1}(:,3)), AD), cellfun(@(x) peakmag(x(:,1), AC{1}(:,1)), AD)];
semilogx(A, Y(:,1), 'k', A, Y(:,2),'r');
xlabel('Input (V)');
ylabel('Signal to Noise Ratio (dB)');
title('SNR versus Amplitude at 100 Hz');
legend('Analog','To-Digital','location','best');
fixplot;
print -dpng SnrAMP

fprintf('BNL1: %f\n', mean(sqrt(AC{1}(:,3).^2)));
fprintf('BNL2: %f\n', mean(sqrt(AC{1}(:,1).^2)));

%% Power
figure(7);
Y = 1e3*(cellfun(@(x) peakcur(x(:,2), R(1)), FD) + cellfun(@(x) peakcur(x(:,4), R(2)), FD));
semilogx(F, Y(:,1), 'k');
xlabel('Frequency (Hz)');
ylabel('Max Current (mA)')
title('Max Current versus Frequency at 10 mV');
fixplot;
print -dpng PowerFRQ

figure(8);
Y = 1e3*(cellfun(@(x) peakcur(x(:,2), R(1)), AD) + cellfun(@(x) peakcur(x(:,4), R(2)), AD));
semilogx(A, Y(:,1), 'k');
xlabel('Input (V)');
ylabel('Max Current (mA)')
title('Max Current versus Amplitude at 100 Hz');
fixplot
print -dpng PowerAMP

%% Auxiliary Functions
function out = peakmag(y, A, f)
    t = (0:1e-4:1-1e-4)';
    if nargin < 3 || isempty(f); f = 100; end
    if nargin < 2 || isempty(A); A = 0.01; end
    if length(A) == 1; A = A*sin(2*pi*f*t); end 
    out = 20*log10(sqrt(bandpower(y,1e4,[0 5e3])/bandpower(A,1e4,[0 5e3])));
end

function out = peakcur(y, R)
    out = findpeaks(abs(y)/R, (1:length(y))'/1e4);
    out = out(~isinf(out) & ~isnan(out));
    out = mean(out);
end
