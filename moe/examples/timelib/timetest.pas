uses timelib;
var i:longint;
begin
for i:=1 to 100000000 do i := i;
writeln(cpspc_time);
end.
