begin
  integer k;
  integer m;
  integer function F(n);
    begin
      integer t;
      if n<=0 then F:=1
      else F:=n*F(n-1)
    end;
  integer t;
  read(m);
  k:=F(m);
  write(k)
end