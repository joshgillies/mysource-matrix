
spool drop.log
ol drop.log
set serveroutput on

prompt drop all tables in the schema

begin
  for rec in (select TABLE_NAME, constraint_name from user_constraints where constraint_type = 'R') LOOP
    execute immediate 'alter table '||rec.table_name||' drop constraint '||rec.constraint_name;
  end loop;

  for rec in (select table_name from user_tables) loop
       execute immediate 'drop table &&1..'||rec.table_name;
  end loop;
  for rec in (select sequence_name from user_sequences) loop
      execute immediate 'drop sequence &&1..'||rec.sequence_name;
  end loop;
  for rec in (select trigger_name from user_triggers) loop
      execute immediate 'drop trigger &&1..'||rec.trigger_name;
  end loop;
end;
/
exit;
/
