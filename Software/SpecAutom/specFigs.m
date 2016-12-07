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

%% Proof Plots
pf = [30 67 80];
for i = 1:3
    figure(i); j = pf(i);
    plot(t, FD{j}(:,3), 'k', t, FD{j}(:,1), 'r')
    ylim([0 1.2])
    xlabel('Time (s)');
    ylabel('Amplitude (V)');
    title(sprintf('System Response at 10 mV, %0.1f Hz',F(j)));
    legend('Analog','Digital','location','best');
    fixplot;
    eval(sprintf('print -dpng Proof%d',i));
end

%% Frequency Response
figure(4);
Y = [cellfun(@(x) peakmag(x(:,3)), FD), cellfun(@(x) peakmag(x(:,1)), FD)];
semilogx(F, Y(:,1), 'k', F, Y(:,2), 'r');
xlabel('Frequency (Hz)')
ylabel('Response (dB)');
title('Frequency Response for Sinusoidal Input')
legend('Analog','Digital','location','best');
fixplot;
print -dpng FRQResp

%% Common Mode Rejection
figure(5);
Y = [cellfun(@(x,y) peakcmr(x(:,3), y(:,3)), FD, FC), cellfun(@(x,y) peakcmr(x(:,1), y(:,1)), FD, FC)];
semilogx(F, Y(:,1), 'k', F, Y(:,2), 'r');
xlabel('Frequency (Hz)');
ylabel('Rejection Ratio (dB)');
title('CMRR versus Frequency at 10 mV');
legend('AO','DO','location','best');
fixplot;
print -dpng CmrrFRQ

figure(6);
Y = [cellfun(@(x,y) peakcmr(x(:,3), y(:,3)), AD, AC), cellfun(@(x,y) peakcmr(x(:,1), y(:,1)), AD, AC)];
semilogx(A, Y(:,1), 'k', A, Y(:,2),'r');
xlabel('Input (V)');
ylabel('Rejection Ratio (dB)');
title('CMRR versus Amplitude at 100 Hz');
legend('Analog','Digital','location','best');
fixplot;
print -dpng CmrrAMP

%% Gain
figure(7);
E = (1+50/5.6)*(1+10/10)*(1+100/10)*(1+5.6/10);
Y = [cellfun(@(x,a) peakmag(x(:,3), a), AD, num2cell(A)), cellfun(@(x,a) peakmag(x(:,1), a), AD, num2cell(A))];
semilogx(A, Y(:,1),'k', A, Y(:,2),'r',A,20*log10(E)+A*0,'k:');
xlabel('Input (V)');
ylabel('Gain (dB)')
title('Gain at 100 Hz');
legend('Analog','Digital','location','best');
fixplot
print -dpng Gain

%% Power
figure(8);
Y = 1e3*(cellfun(@(x) peakcur(x(:,2), R(1)), FD) + cellfun(@(x) peakcur(x(:,4), R(2)), FD));
semilogx(F, Y(:,1), 'k');
xlabel('Frequency (Hz)');
ylabel('Max Current (mA)')
title('Max Current versus Frequency at 10 mV');
fixplot;
print -dpng PowerFRQ

figure(9);
Y = 1e3*(cellfun(@(x) peakcur(x(:,2), R(1)), AD) + cellfun(@(x) peakcur(x(:,4), R(2)), AD));
semilogx(A, Y(:,1), 'k');
xlabel('Input (V)');
ylabel('Max Current (mA)')
title('Max Current versus Amplitude at 100 Hz');
fixplot
print -dpng PowerAMP

%% Auxiliary Functions
function out = peakmag(y, A)
    if nargin < 3 || isempty(A); A = 0.01; end
    out = 20*log10(sqrt(bandpower(y/A,1e4,[0 5e3])));
end

function out = peakcmr(y, x)
    out = 20*log10(sqrt(bandpower(y,1e4,[0 5e3])/bandpower(x,1e4,[0 5e3])));
end

function out = peakcur(y, R)
    out = findpeaks(abs(y)/R, (1:length(y))'/1e4);
    out = out(~isinf(out) & ~isnan(out));
    out = mean(out);
end
