%% Load
clear; close all; filename = {...
    'freq-1206-132412.mat',...  % F - Diff
    'freq-1206-132656.mat',...  % F - Comm
    'amp-1206-132847.mat',...   % A - Diff
    'amp-1206-133025.mat'};     % A = Comm
R = [1.15 1.26]; % Ohms

dat = cell(200,6);
for i = 1:4
    load(filename{i});
    dat(:,2+i) = raw(:,3);
    if i < 3
        dat(:,1) = raw(:,1);
    else
        for j = 1:200
            dat{j,2+i}(:,2) = dat{j,2+i}(:,2) / R(1);
            dat{j,2+i}(:,4) = dat{j,2+i}(:,4) / R(2);
        end
        dat(:,2) = raw(:,1);
    end
end
clear raw f A i j R

%% Proof Plots
pf = [68 130 180];
for i = 1:3
    figure(i); j = pf(i); t = (1:length(dat{j,3}))'/1e4;
    plot(t, dat{j,3}(:,3), 'k', t, dat{j,3}(:,1), 'r')
    xlabel('Time (s)'); ylabel('Amplitude (V)'); fixplot;
    title(sprintf('System Response at a %0.1f Hz 10 mV Input',dat{j,1}));
    legend('AO','DO','location','best'); eval(sprintf('print -dpng Proof%d',i));
end

%% Frequency Response
figure(4); x = cell2mat(dat(1:end-1,1));
y = [cellfun(@(x,f) peakmag(x(:,3), f), dat(1:end-1,3), dat(1:end-1,1)),...
    cellfun(@(x,f) peakmag(x(:,1), f), dat(1:end-1,3), dat(1:end-1,1))];
semilogx(x,y(:,1),'k',x,y(:,2),'r'); xlabel('Frequency (Hz)')
ylabel('Response (dB)'); title('Frequency Response for Sinusoidal Input')
legend('AO','DO','location','best'); fixplot; print -dpng FRQResp

%% Common Mode Rejection
figure(5); x = cell2mat(dat(1:end-1,1));
y = [cellfun(@(x,y) peakcmr(x(:,3), y(:,3)), dat(1:end-1,3), dat(1:end-1,4)),...
    cellfun(@(x,y) peakcmr(x(:,1), y(:,1)), dat(1:end-1,3), dat(1:end-1,4))];
semilogx(x,y(:,1),'k',x,y(:,2),'r'); xlabel('Frequency (Hz)');
ylabel('Rejection Ratio (dB)'); title('CM Rejection Ratio versus Frequency');
legend('AO','DO','location','best'); fixplot; print -dpng CmrrFRQ
figure(6); x = cell2mat(dat(1:end-1,2));
y = [cellfun(@(x,y) peakcmr(x(:,3), y(:,3)), dat(1:end-1,5), dat(1:end-1,6)),...
    cellfun(@(x,y) peakcmr(x(:,1), y(:,1)), dat(1:end-1,5), dat(1:end-1,6))];
semilogx(x,y(:,1),'k',x,y(:,2),'r'); xlabel('Input (V)');
ylabel('Rejection Ratio (dB)'); title('CM Rejection Ratio versus Amplitide');
legend('AO','DO','location','best'); fixplot; print -dpng CmrrAMP

%% Gain
figure(7); x = cell2mat(dat(1:end-1,2));
y = [cellfun(@(x,a) peakmag(x(:,3), 100, a), dat(1:end-1,3), dat(1:end-1,2)),...
    cellfun(@(x,a) peakmag(x(:,1), 100, a), dat(1:end-1,3), dat(1:end-1,2))];
semilogx(x,y(:,1),'k',x,y(:,2),'r'); xlabel('Input (V)'); ylabel('Gain (dB)')
title('Gain for 100 Hz Input'); legend('AO','DO','location','best'); fixplot
print -dpng Gain

%% Power
figure(8); x = cell2mat(dat(1:end-1,1));
y = 1e3*(cellfun(@(x) peakpwr(x(:,2)), dat(1:end-1,3))-...
    cellfun(@(x) peakpwr(x(:,4)), dat(1:end-1,3)));
semilogx(x,y(:,1),'k'); xlabel('Input (V)'); ylabel('Peak Power (mW)')
title('Peak Power versus Frequency'); fixplot; print -dpng PowerFRQ
figure(9); x = cell2mat(dat(1:end-1,2));
y = 1e3*(cellfun(@(x) peakpwr(x(:,2)), dat(1:end-1,5))-...
    cellfun(@(x) peakpwr(x(:,4)), dat(1:end-1,5)));
semilogx(x,y(:,1),'k'); xlabel('Input (V)'); ylabel('Peak Power (mW)')
title('Peak Power versus Amplitude'); fixplot; print -dpng PowerAMP

%% Auxiliary Functions
function out = peakout(h, t)
    hmag = abs(h); db = 20*log10(hmag);
    out = findpeaks(db,t); out = out(~isinf(out) & ~isnan(out));
    out = mean(out);
end

function out = peakmag(y, f, A)
    if nargin < 3 || isempty(A); A = 0.01; end
    t = (1:length(y))'/1e4; x = A*sin(2*pi*f*t); h = y./x;
    out = peakout(h,t);
end

function out = peakcmr(y, x)
    t = (1:length(y))'/1e4; h = y./x;
    out = peakout(h,t);
end

function out = peakpwr(y)
    t = (1:length(y))'/1e4; h = y*5.02;
    out = 10^(peakout(h,t)/20);
end
