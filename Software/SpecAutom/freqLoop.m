clear; global s

dt = 1/5e3;
A  = 1/100;

s = daq.createSession('ni');
s.addAnalogInputChannel('Dev2', 0, 'Voltage');
s.Channels(1).Range = [-5 5];
s.addAnalogOutputChannel('Dev2',0,'Voltage');
s.Rate = 1/dt;

f = (1:1e3)';

raw = cell(length(f),3);

if s.IsLogging; error('DAQ is running'); end

for i = 1:length(f)
    raw{i,1} = f(i);
    t = (0:dt:(2/f(i)-dt))';
    s.queueOutputData(A*sin(2*pi*f(i)*t));
    [raw{i,3}, raw{i,2}] = s.startForeground;
end

r = cellfun(@(x) mean(sqrt(x.^2)), raw(:,3));

clear s i t A

save(['freq' datestr(now,'-mmdd-hhMMss')])