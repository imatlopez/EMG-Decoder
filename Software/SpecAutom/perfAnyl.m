clear; close all;
%% Import the data
[~, ~, raw] = xlsread('../../Forms/Testing.xlsx','Analysis');
dat = cell2mat(raw(2:end,3:23));
clear raw

%% Organize data
R = dat(1, 1:7);
P = zeros(15,7,2);
M = zeros(3,7,2);
D = zeros(3,7,2);
for i = 1:3
    for j = [1 3]
        P((1:5)+(i-1)*5,:,round(j/2)) = dat(4*(0:4)+j+1,(7*i-6):(7*i));
        M(i,:,round(j/2)) = mean(dat(4*(0:4)+j+1,(7*i-6):(7*i)),1);
        D(i,:,round(j/2)) = std(dat(4*(0:4)+j+1,(7*i-6):(7*i)),[],1)/sqrt(5);
    end
end
clear dat

%% Error Bar Plot
figure(1)
errorbar(repmat(R',[1 3]),100*M(:,:,2)',100*D(:,:,2)',100*D(:,:,2)')
xlabel('Gain Resistance (K\Omega)')
ylabel('False Positive Rate (%)')
title('False Positive for Increasing Gain Resistance')
fixplot
print -dpng SpecPlot

%% Lilliefors
N = zeros(6,7);
V(1) = vartestn(P(:,:,1));
V(2) = vartestn(P(:,:,2));
for i = 1:3
    for j = 1:7
        for k = 1:2
            [~, N(i+3*k-3,j)] = lillietest(P((1:5)+(i-1)*5,j,k));
        end
    end
end

%% 2 Way Anova
close all;
[H1, T1, S1] = anova2(P(:,:,1),5);
set(gcf, 'rend','painters','pos',[10 10 360 120]); print -dpng Anv1
[H2, T2, S2] = anova2(P(:,:,2),5);
set(gcf, 'rend','painters','pos',[10 10 360 120]); print -dpng Anv2