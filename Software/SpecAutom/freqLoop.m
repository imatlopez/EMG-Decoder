clear; global s

dt = 1/1e4;
A  = 1/100;

s = daq.createSession('ni');
s.addAnalogInputChannel('Dev1', [0 1 2 3], 'Voltage');
s.Channels(1).Range = [-5 5];
s.addAnalogOutputChannel('Dev1',0,'Voltage');
s.Rate = 1/dt;

f = logspace(0,3,100)'; f(67) = 101;
t = (0:dt:1-dt)';

raw = cell(length(f),3);

if s.IsLogging; error('DAQ is running'); end

for i = 1:length(f)
    raw{i,1} = f(i);
    s.queueOutputData(A*sin(2*pi*f(i)*t));
    [raw{i,3}, raw{i,2}] = s.startForeground;
    if mod(i,round(length(f)/10))==0; fprintf('%0.2f Hz\n',f(i)); end
end

clear s i t A f

save(['freq' datestr(now,'-mmdd-hhMMss')])