#!/bin/sh

LANG=EUC-JP

echo	"=== drop ������;"
echo	"drop table ������;"
echo	"create table ������(̾�� char(30), �� char(30));"
echo	"insert into ������ values('����ŵ��', '�̳�ƻ');"
echo	"insert into ������ values('���ռ���', '����');"
echo	"insert into ������ values('��������', '�̳�ƻ');"
echo	"insert into ������ values('_�܎��Ŏ͎ގɎ؎���', '�̳�ƻ');"
echo	"select * from ������;"
echo	"commit;			==="
xjSql $1 <<_EOF_ > z
drop table ������;
create table ������(̾�� char(30), �� char(30));
insert into ������ values('����ŵ��', '�̳�ƻ');
insert into ������ values('���ռ���', '����');
insert into ������ values('��������', '�̳�ƻ');
insert into ������ values('_�܎��Ŏ͎ގɎ؎���', '�̳�ƻ');
select * from ������;
commit;
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=4" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� like '����' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� like '����';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=0" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� like '%����%' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� like '%����%';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=2" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� like '����ŵ��' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� like '����ŵ��';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=1" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� like '����%ŵ��' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� like '����%ŵ��';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=1" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� like '����%' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� like '����%';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=2" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� like '%ŵ��' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� like '%ŵ��';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=1" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� like '����____' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� like '����____';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=2" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� like '____ŵ��' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� like '____ŵ��';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=1" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� like '__ŵ��' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� like '__ŵ��';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=0" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� like '��%ŵ__' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� like '��%ŵ__';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=1" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� not like '��%ŵ__' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� not like '��%ŵ__';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=3" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� not like '%�܎��Ŏ͎�%' ===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� not like '%�܎��Ŏ͎�%';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=3" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ������ where ̾�� like '\_�܎��Ŏ͎�%' escape '\'===";
xjSql $1 <<_EOF_ > z
select * from ������ where ̾�� like '\_�܎��Ŏ͎�%' escape '\';
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=1" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== drop table ɽ;"
echo	"	create table ɽ(���� char(20), ���� int);"
echo	"	insert into ɽ values('����ŵ��', 100);"
echo	"	insert into ɽ values('���ռ���', 90);"
echo	"	commit;"
echo	"	select * from ɽ;"
xjSql $1 <<_EOF_ > z
drop table ɽ;
create table ɽ(���� char(20), ���� int);
insert into ɽ values('����ŵ��', 100);
insert into ɽ values('���ռ���', 90);
commit;
select * from ɽ;
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=2" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== select * from ɽ where ���� = 100;"
xjSql $1 <<_EOF_ > z
select * from ɽ where ���� = 100;
quit;
_EOF_
if [ $? -ne 0 ] ; then exit 1 ; fi
count=$( fgrep "Total=1" z |wc -l )
if [ $count -ne 1 ]; then exit 1; fi

echo	"=== OK ==="
