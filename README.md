```console
$ cat example.sql 
select foo, bar
from /* TODO: change table name */ Something
where bar like 'strings and %';
-- line comment
/* Invalid unterminated comment

$ ./lex-sqlite <example.sql 
other: select
other: foo,
other: bar
other: from
comment:  TODO: change table name 
other: Something
other: where
other: bar
other: like
string: strings and %
other: ;
comment:  line comment
comment:  Invalid unterminated comment

$ 
```
