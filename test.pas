procedure E;
begin
    T;
    E’
end;
procedure T;
begin
    F;
    T’
end;
procedure E’;
    if lookahead=‘+’ then begin match(‘+’);
        T;
        E’
    end;
procedure T’;
    if lookahead=‘*’ then begin match(‘*’);
        F;
        T’
    end;
