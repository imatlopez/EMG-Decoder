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

%% Bar Plot

%% Error Bar Plot
figure(1)
errorbar(repmat(R',[1 3]),100*M(:,:,2)',100*D(:,:,2)',100*D(:,:,2)','LineWidth',3)
legend('Bicep','Forearm','Thumb','location','best')
xlabel('Gain Resistance (K\Omega)')
ylabel('False Positive Rate (%)')
title('False Positive for Increasing Gain Resistance')
fixplot(28)
print -dpng SpecPlot

%% Trend Plot
figure(2)
L = P(1,:,2); for i = 2:15; L = [L P(i,:,2)]; end %#ok<AGROW>
L = polyfit(repmat(R,[1 15]),L,1);
plot(repmat(R',[1 3]),100*M(:,:,2)','o',R,100*polyval(L,R),'k:')
legend('Bicep','Forearm','Thumb','Trend','location','best')
xlabel('Gain Resistance (K\Omega)')
ylabel('False Positive Rate (%)')
title('False Positive for Increasing Gain Resistance')
fixplot(28)
print -dpng LpecPlot

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
[~, T1, S1] = friedman(P(:,:,1),5);
set(gcf, 'rend','painters','pos',[10 10 400 150]); print -dpng Anv1
[~, T2, S2] = friedman(P(:,:,2),5);
set(gcf, 'rend','painters','pos',[10 10 400 150]); print -dpng Anv2